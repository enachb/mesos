/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <set>
#include <string>
#include <vector>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <gmock/gmock.h>

#include <stout/option.hpp>
#include <stout/os.hpp>
#include <stout/path.hpp>
#include <stout/stringify.hpp>
#include <stout/strings.hpp>

#include "linux/cgroups.hpp"

#include "tests/utils.hpp"

using namespace process;

const static std::string HIERARCHY = "/tmp/mesos_cgroups_testing_hierarchy";


class CgroupsTest : public ::testing::Test
{
protected:
  static void SetUpTestCase()
  {
    TearDownTestCase();
  }

  static void TearDownTestCase()
  {
    // Remove the testing hierarchy.
    if (cgroups::checkHierarchy(HIERARCHY).isSome()) {
      // Remove all cgroups.
      Try<std::vector<std::string> > cgroups = cgroups::getCgroups(HIERARCHY);
      ASSERT_SOME(cgroups);
      foreach (const std::string& cgroup, cgroups.get()) {
        ASSERT_SOME(cgroups::removeCgroup(HIERARCHY, cgroup));
      }

      // Remove the hierarchy.
      ASSERT_SOME(cgroups::removeHierarchy(HIERARCHY));

      // Remove the directory if still exists.
      if (os::exists(HIERARCHY)) {
        os::rmdir(HIERARCHY);
      }
    }
  }
};


// A fixture which is used to name tests that expect NO hierarchy to
// exist in order to test the ability to create a hierarchy (since
// most likely existing hierarchies will have all or most subsystems
// attached rendering our ability to create a hierarchy fruitless).
class CgroupsNoHierarchyTest : public CgroupsTest
{
protected:
  static void SetUpTestCase()
  {
    CgroupsTest::SetUpTestCase();

    Try<std::set<std::string> > hierarchies = cgroups::hierarchies();
    ASSERT_SOME(hierarchies);
    if (!hierarchies.get().empty()) {
      std::cerr
        << "-------------------------------------------------------------\n"
        << "We cannot run any cgroups tests that require creating\n"
        << "hierarchies because you have the following hierarchies active:\n"
        << strings::trim(stringify(hierarchies.get()), " {},") << "\n"
        << "You can either remove those hierarchies, or disable\n"
        << "this test case (i.e., --gtest_filter=-CgroupsNoHierarchyTest.*).\n"
        << "-------------------------------------------------------------"
        << std::endl;
    }
  }
};


// A fixture that assumes ANY hierarchy is acceptable for use provided
// it has the subsystems attached that were specified in the
// constructor. If no hierarchy could be found that has all the
// required subsystems then we attempt to create a new hierarchy.
class CgroupsAnyHierarchyTest : public CgroupsTest
{
public:
  CgroupsAnyHierarchyTest(const std::string& _subsystems = "cpu")
    : subsystems(_subsystems) {}

protected:
  static void SetUpTestCase()
  {
    CgroupsTest::SetUpTestCase();
  }

  static void TearDownTestCase()
  {
    CgroupsTest::TearDownTestCase();
  }

  virtual void SetUp()
  {
    Try<std::set<std::string> > hierarchies = cgroups::hierarchies();
    ASSERT_SOME(hierarchies);
    foreach (const std::string& candidate, hierarchies.get()) {
      if (subsystems.empty()) {
        hierarchy = candidate;
        break;
      }

      // Check and see if this candidate meets our subsystem requirements.
      if (cgroups::checkHierarchy(candidate, subsystems).isSome()) {
        hierarchy = candidate;
        break;
      }
    }

    if (hierarchy.empty()) {
      // Create a hierarchy for testing.
      ASSERT_SOME(cgroups::createHierarchy(HIERARCHY, subsystems))
        << "-------------------------------------------------------------\n"
        << "We cannot run any cgroups tests that require\n"
        << "a hierarchy with subsystems '" << subsystems << "'\n"
        << "because we failed to find an existing hierarchy\n"
        << "or create a new one. You can either remove all existing\n"
        << "hierarchies, or disable this test case\n"
        << "(i.e., --gtest_filter=-"
        << ::testing::UnitTest::GetInstance()
             ->current_test_info()
             ->test_case_name() << ".*).\n"
        << "-------------------------------------------------------------";

      hierarchy = HIERARCHY;
    }

    // Create a cgroup (removing first if necessary) for the tests to use.
    if (cgroups::checkCgroup(hierarchy, "mesos_test").isSome()) {
      ASSERT_FUTURE_WILL_SUCCEED(cgroups::destroyCgroup(hierarchy, "mesos_test"))
        << "-------------------------------------------------------------\n"
        << "We failed to destroy our \"testing\" cgroup (most likely left\n"
        << "around from a previously failing test). This is a pretty\n"
        << "serious error, please report a bug!\n"
        << "-------------------------------------------------------------";
    }

    ASSERT_SOME(cgroups::createCgroup(hierarchy, "mesos_test"));
  }

  virtual void TearDown()
  {
    // Remove all *our* cgroups.
    Try<std::vector<std::string> > cgroups = cgroups::getCgroups(hierarchy);
    ASSERT_SOME(cgroups);
    foreach (const std::string& cgroup, cgroups.get()) {
      if (strings::startsWith(cgroup, "mesos_test")) {
        ASSERT_SOME(cgroups::removeCgroup(hierarchy, cgroup));
      }
    }

    // And destroy HIERARCHY in the event it needed to be created.
    CgroupsTest::TearDownTestCase();
  }

  const std::string subsystems; // Subsystems required to run tests.
  std::string hierarchy; // Path to the hierarchy being used.
};


class CgroupsAnyHierarchyWithCpuMemoryTest
  : public CgroupsAnyHierarchyTest
{
public:
  CgroupsAnyHierarchyWithCpuMemoryTest()
    : CgroupsAnyHierarchyTest("cpu,memory") {}
};


class CgroupsAnyHierarchyWithCpuMemoryFreezerTest
  : public CgroupsAnyHierarchyTest
{
public:
  CgroupsAnyHierarchyWithCpuMemoryFreezerTest()
    : CgroupsAnyHierarchyTest("cpu,memory,freezer") {}
};


TEST_F(CgroupsAnyHierarchyTest, ROOT_CGROUPS_Enabled)
{
  EXPECT_SOME_TRUE(cgroups::enabled("cpu"));
  EXPECT_SOME_TRUE(cgroups::enabled(",cpu"));
  EXPECT_SOME_TRUE(cgroups::enabled("cpu,memory"));
  EXPECT_SOME_TRUE(cgroups::enabled("cpu,memory,"));
  EXPECT_ERROR(cgroups::enabled("invalid"));
  EXPECT_ERROR(cgroups::enabled("cpu,invalid"));
  EXPECT_ERROR(cgroups::enabled(","));
  EXPECT_ERROR(cgroups::enabled(""));
}


TEST_F(CgroupsAnyHierarchyWithCpuMemoryTest, ROOT_CGROUPS_Busy)
{
  EXPECT_ERROR(cgroups::busy("invalid"));
  EXPECT_ERROR(cgroups::busy("cpu,invalid"));
  EXPECT_ERROR(cgroups::busy(","));
  EXPECT_ERROR(cgroups::busy(""));
  EXPECT_SOME_TRUE(cgroups::busy("cpu"));
  EXPECT_SOME_TRUE(cgroups::busy(",cpu"));
  EXPECT_SOME_TRUE(cgroups::busy("cpu,memory"));
  EXPECT_SOME_TRUE(cgroups::busy("cpu,memory,"));
}


TEST_F(CgroupsAnyHierarchyTest, ROOT_CGROUPS_Subsystems)
{
  Try<std::set<std::string> > names = cgroups::subsystems();
  ASSERT_SOME(names);

  Option<std::string> cpu;
  Option<std::string> memory;
  foreach (const std::string& name, names.get()) {
    if (name == "cpu") {
      cpu = name;
    } else if (name == "memory") {
      memory = name;
    }
  }

  EXPECT_SOME(cpu);
  EXPECT_SOME(memory);
}


TEST_F(CgroupsAnyHierarchyWithCpuMemoryTest, ROOT_CGROUPS_SubsystemsHierarchy)
{
  Try<std::set<std::string> > names = cgroups::subsystems(hierarchy);
  ASSERT_SOME(names);

  Option<std::string> cpu;
  Option<std::string> memory;
  foreach (const std::string& name, names.get()) {
    if (name == "cpu") {
      cpu = name;
    } else if (name == "memory") {
      memory = name;
    }
  }

  EXPECT_SOME(cpu);
  EXPECT_SOME(memory);
}


TEST_F(CgroupsNoHierarchyTest, ROOT_CGROUPS_CreateRemoveHierarchy)
{
  EXPECT_ERROR(cgroups::createHierarchy("/tmp", "cpu"));
  EXPECT_ERROR(cgroups::createHierarchy(HIERARCHY, "invalid"));
  ASSERT_SOME(cgroups::createHierarchy(HIERARCHY, "cpu,memory"));
  EXPECT_ERROR(cgroups::createHierarchy(HIERARCHY, "cpuset"));
  EXPECT_ERROR(cgroups::removeHierarchy("/tmp"));
  ASSERT_SOME(cgroups::removeHierarchy(HIERARCHY));
}


TEST_F(CgroupsAnyHierarchyTest, ROOT_CGROUPS_CheckHierarchy)
{
  EXPECT_ERROR(cgroups::checkHierarchy("/tmp-nonexist"));
  EXPECT_ERROR(cgroups::checkHierarchy("/tmp"));
  EXPECT_SOME(cgroups::checkHierarchy(hierarchy));
  EXPECT_SOME(cgroups::checkHierarchy(hierarchy + "/"));
  EXPECT_ERROR(cgroups::checkHierarchy(hierarchy + "/not_expected"));
}


TEST_F(CgroupsAnyHierarchyWithCpuMemoryTest, ROOT_CGROUPS_CheckHierarchySubsystems)
{
  EXPECT_ERROR(cgroups::checkHierarchy("/tmp-nonexist", "cpu"));
  EXPECT_ERROR(cgroups::checkHierarchy("/tmp", "cpu,memory"));
  EXPECT_ERROR(cgroups::checkHierarchy("/tmp", "cpu"));
  EXPECT_ERROR(cgroups::checkHierarchy("/tmp", "invalid"));
  EXPECT_SOME(cgroups::checkHierarchy(hierarchy, "cpu,memory"));
  EXPECT_SOME(cgroups::checkHierarchy(hierarchy, "memory"));
  EXPECT_ERROR(cgroups::checkHierarchy(hierarchy, "invalid"));
  EXPECT_ERROR(cgroups::checkHierarchy(hierarchy + "/not_expected", "cpu"));
}


TEST_F(CgroupsAnyHierarchyWithCpuMemoryTest, ROOT_CGROUPS_CreateRemoveCgroup)
{
  EXPECT_ERROR(cgroups::createCgroup("/tmp", "test"));
  EXPECT_ERROR(cgroups::createCgroup(hierarchy, "mesos_test_missing/1"));
  ASSERT_SOME(cgroups::createCgroup(hierarchy, "mesos_test_missing"));
  EXPECT_ERROR(cgroups::removeCgroup(hierarchy, "invalid"));
  ASSERT_SOME(cgroups::removeCgroup(hierarchy, "mesos_test_missing"));
}


TEST_F(CgroupsAnyHierarchyTest, ROOT_CGROUPS_GetCgroups)
{
  ASSERT_SOME(cgroups::createCgroup(hierarchy, "mesos_test1"));
  ASSERT_SOME(cgroups::createCgroup(hierarchy, "mesos_test2"));

  Try<std::vector<std::string> > cgroups = cgroups::getCgroups(hierarchy);
  ASSERT_SOME(cgroups);

  EXPECT_EQ(cgroups.get()[0], "mesos_test2");
  EXPECT_EQ(cgroups.get()[1], "mesos_test1");
  EXPECT_EQ(cgroups.get()[2], "mesos_test");

  ASSERT_SOME(cgroups::removeCgroup(hierarchy, "mesos_test1"));
  ASSERT_SOME(cgroups::removeCgroup(hierarchy, "mesos_test2"));
}


TEST_F(CgroupsAnyHierarchyTest, ROOT_CGROUPS_NestedCgroups)
{
  ASSERT_SOME(cgroups::createCgroup(hierarchy, "mesos_test/1"))
    << "-------------------------------------------------------------\n"
    << "We cannot run this test because it appears you do not have\n"
    << "a modern enough version of the Linux kernel. You won't be\n"
    << "able to use the cgroups isolation module, but feel free to\n"
    << "disable this test.\n"
    << "-------------------------------------------------------------";

  ASSERT_SOME(cgroups::createCgroup(hierarchy, "mesos_test/2"));

  Try<std::vector<std::string> > cgroups =
    cgroups::getCgroups(hierarchy, "mesos_test");
  ASSERT_SOME(cgroups);
  ASSERT_EQ(2u, cgroups.get().size());

  EXPECT_EQ(cgroups.get()[0], "mesos_test/2");
  EXPECT_EQ(cgroups.get()[1], "mesos_test/1");

  ASSERT_SOME(cgroups::removeCgroup(hierarchy, "mesos_test/1"));
  ASSERT_SOME(cgroups::removeCgroup(hierarchy, "mesos_test/2"));
}


TEST_F(CgroupsAnyHierarchyTest, ROOT_CGROUPS_ReadControl)
{
  EXPECT_ERROR(cgroups::readControl(hierarchy, "mesos_test", "invalid"));

  std::string pid = stringify(::getpid());

  Try<std::string> result = cgroups::readControl(hierarchy, "/", "tasks");
  ASSERT_SOME(result);
  EXPECT_TRUE(strings::contains(result.get(), pid));
}


TEST_F(CgroupsAnyHierarchyTest, ROOT_CGROUPS_WriteControl)
{
  EXPECT_ERROR(cgroups::writeControl(hierarchy,
                                     "mesos_test",
                                     "invalid",
                                     "invalid"));

  pid_t pid = ::fork();
  ASSERT_NE(-1, pid);

  if (pid) {
    // In parent process.
    ASSERT_SOME(cgroups::writeControl(
                    hierarchy, "mesos_test", "tasks", stringify(pid)));

    Try<std::set<pid_t> > tasks = cgroups::getTasks(hierarchy, "mesos_test");
    ASSERT_SOME(tasks);

    std::set<pid_t> pids = tasks.get();
    EXPECT_NE(pids.find(pid), pids.end());

    // Kill the child process.
    ASSERT_NE(-1, ::kill(pid, SIGKILL));

    // Wait for the child process.
    int status;
    EXPECT_NE(-1, ::waitpid((pid_t) -1, &status, 0));
  } else {
    // In child process.
    while (true);

    // Should not reach here.
    std::cerr << "Reach an unreachable statement!" << std::endl;
    abort();
  }
}


TEST_F(CgroupsAnyHierarchyTest, ROOT_CGROUPS_GetTasks)
{
  Try<std::set<pid_t> > tasks = cgroups::getTasks(hierarchy, "/");
  ASSERT_SOME(tasks);

  std::set<pid_t> pids = tasks.get();
  EXPECT_NE(pids.find(1), pids.end());
  EXPECT_NE(pids.find(::getpid()), pids.end());
}


TEST_F(CgroupsAnyHierarchyWithCpuMemoryTest, ROOT_CGROUPS_ListenEvent)
{
  ASSERT_SOME(cgroups::checkControl(hierarchy, "mesos_test", "memory.oom_control"))
    << "-------------------------------------------------------------\n"
    << "We cannot run this test because it appears you do not have\n"
    << "a modern enough version of the Linux kernel. You won't be\n"
    << "able to use the cgroups isolation module, but feel free to\n"
    << "disable this test.\n"
    << "-------------------------------------------------------------";

  // Disable oom killer.
  ASSERT_SOME(cgroups::writeControl(hierarchy,
                                    "mesos_test",
                                    "memory.oom_control",
                                    "1"));

  // Limit the memory usage of "mesos_test" to 64MB.
  size_t limit = 1024 * 1024 * 64;
  ASSERT_SOME(cgroups::writeControl(hierarchy,
                                    "mesos_test",
                                    "memory.limit_in_bytes",
                                    stringify(limit)));

  // Listen on oom events for "mesos_test" cgroup.
  Future<uint64_t> future =
    cgroups::listenEvent(hierarchy,
                         "mesos_test",
                         "memory.oom_control");
  ASSERT_FALSE(future.isFailed());

  // Test the cancellation.
  future.discard();

  // Test the normal operation below.
  future = cgroups::listenEvent(hierarchy,
                                "mesos_test",
                                "memory.oom_control");
  ASSERT_FALSE(future.isFailed());

  pid_t pid = ::fork();
  ASSERT_NE(-1, pid);

  if (pid) {
    // In parent process.
    future.await(Seconds(5.0));

    EXPECT_TRUE(future.isReady());

    // Kill the child process.
    EXPECT_NE(-1, ::kill(pid, SIGKILL));

    // Wait for the child process.
    int status;
    EXPECT_NE(-1, ::waitpid((pid_t) -1, &status, 0));
  } else {
    // In child process. We try to trigger an oom here.
    // Put self into the "mesos_test" cgroup.
    Try<Nothing> assign =
      cgroups::assignTask(hierarchy, "mesos_test", ::getpid());
    if (assign.isError()) {
      std::cerr << "Failed to assign cgroup: " << assign.error() << std::endl;
      abort();
    }

    // Blow up the memory.
    size_t limit = 1024 * 1024 * 512;
    void* buffer = NULL;

    if (posix_memalign(&buffer, getpagesize(), limit) != 0) {
      perror("Failed to allocate page-aligned memory, posix_memalign");
      abort();
    }

    // We use mlock and memset here to make sure that the memory
    // actually gets paged in and thus accounted for.
    if (mlock(buffer, limit) != 0) {
      perror("Failed to lock memory, mlock");
      abort();
    }

    if (memset(buffer, 1, limit) != 0) {
      perror("Failed to fill memory, memset");
      abort();
    }

    // Should not reach here.
    std::cerr << "OOM does not happen!" << std::endl;
    abort();
  }
}


TEST_F(CgroupsAnyHierarchyWithCpuMemoryFreezerTest, ROOT_CGROUPS_Freezer)
{
  int pipes[2];
  int dummy;
  ASSERT_NE(-1, ::pipe(pipes));

  pid_t pid = ::fork();
  ASSERT_NE(-1, pid);

  if (pid) {
    // In parent process.
    ::close(pipes[1]);

    // Wait until child has assigned the cgroup.
    ASSERT_LT(0, ::read(pipes[0], &dummy, sizeof(dummy)));
    ::close(pipes[0]);

    // Freeze the "mesos_test" cgroup.
    Future<bool> freeze = cgroups::freezeCgroup(hierarchy, "mesos_test");
    freeze.await(Seconds(5.0));
    ASSERT_TRUE(freeze.isReady());
    EXPECT_EQ(true, freeze.get());

    // Thaw the "mesos_test" cgroup.
    Future<bool> thaw = cgroups::thawCgroup(hierarchy, "mesos_test");
    thaw.await(Seconds(5.0));
    ASSERT_TRUE(thaw.isReady());
    EXPECT_EQ(true, thaw.get());

    // Kill the child process.
    ASSERT_NE(-1, ::kill(pid, SIGKILL));

    // Wait for the child process.
    int status;
    EXPECT_NE(-1, ::waitpid((pid_t) -1, &status, 0));
  } else {
    // In child process.
    ::close(pipes[0]);

    // Put self into the "mesos_test" cgroup.
    Try<Nothing> assign =
      cgroups::assignTask(hierarchy, "mesos_test", ::getpid());
    if (assign.isError()) {
      std::cerr << "Failed to assign cgroup: " << assign.error() << std::endl;
      abort();
    }

    // Notify the parent.
    if (::write(pipes[1], &dummy, sizeof(dummy)) != sizeof(dummy)) {
      perror("Failed to notify the parent");
      abort();
    }
    ::close(pipes[1]);

    // Infinite loop here.
    while (true);

    // Should not reach here.
    std::cerr << "Reach an unreachable statement!" << std::endl;
    abort();
  }
}


TEST_F(CgroupsAnyHierarchyWithCpuMemoryFreezerTest, ROOT_CGROUPS_KillTasks)
{
  int pipes[2];
  int dummy;
  ASSERT_NE(-1, ::pipe(pipes));

  pid_t pid = ::fork();
  ASSERT_NE(-1, pid);

  if (pid) {
    // In parent process.
    ::close(pipes[1]);

    // Wait until all children have assigned the cgroup.
    ASSERT_LT(0, ::read(pipes[0], &dummy, sizeof(dummy)));
    ASSERT_LT(0, ::read(pipes[0], &dummy, sizeof(dummy)));
    ASSERT_LT(0, ::read(pipes[0], &dummy, sizeof(dummy)));
    ASSERT_LT(0, ::read(pipes[0], &dummy, sizeof(dummy)));
    ::close(pipes[0]);

    Future<bool> future = cgroups::killTasks(hierarchy, "mesos_test");
    future.await(Seconds(5.0));
    ASSERT_TRUE(future.isReady());
    EXPECT_TRUE(future.get());

    int status;
    EXPECT_NE(-1, ::waitpid((pid_t) -1, &status, 0));
  } else {
    // In child process.

    // We create 4 child processes here using two forks to test the case in
    // which there are multiple active processes in the given cgroup.
    ::fork();
    ::fork();

    // Put self into "mesos_test" cgroup.
    Try<Nothing> assign =
      cgroups::assignTask(hierarchy, "mesos_test", ::getpid());
    if (assign.isError()) {
      std::cerr << "Failed to assign cgroup: " << assign.error() << std::endl;
      abort();
    }

    // Notify the parent.
    ::close(pipes[0]); // TODO(benh): Close after first fork?
    if (::write(pipes[1], &dummy, sizeof(dummy)) != sizeof(dummy)) {
      perror("Failed to notify the parent");
      abort();
    }
    ::close(pipes[1]);

    // Wait kill signal from parent.
    while (true);

    // Should not reach here.
    std::cerr << "Reach an unreachable statement!" << std::endl;
    abort();
  }
}


TEST_F(CgroupsAnyHierarchyWithCpuMemoryFreezerTest, ROOT_CGROUPS_DestroyCgroup)
{
  int pipes[2];
  int dummy;
  ASSERT_NE(-1, ::pipe(pipes));

  pid_t pid = ::fork();
  ASSERT_NE(-1, pid);

  if (pid) {
    // In parent process.
    ::close(pipes[1]);

    // Wait until all children have assigned the cgroup.
    ASSERT_LT(0, ::read(pipes[0], &dummy, sizeof(dummy)));
    ASSERT_LT(0, ::read(pipes[0], &dummy, sizeof(dummy)));
    ASSERT_LT(0, ::read(pipes[0], &dummy, sizeof(dummy)));
    ASSERT_LT(0, ::read(pipes[0], &dummy, sizeof(dummy)));
    ::close(pipes[0]);

    Future<bool> future = cgroups::destroyCgroup(hierarchy, "mesos_test");
    future.await(Seconds(5.0));
    ASSERT_TRUE(future.isReady());
    EXPECT_TRUE(future.get());

    int status;
    EXPECT_NE(-1, ::waitpid((pid_t) -1, &status, 0));
  } else {
    // In child process.

    // We create 4 child processes here using two forks to test the case in
    // which there are multiple active processes in the given cgroup.
    ::fork();
    ::fork();

    // Put self into "mesos_test" cgroup.
    Try<Nothing> assign =
      cgroups::assignTask(hierarchy, "mesos_test", ::getpid());
    if (assign.isError()) {
      std::cerr << "Failed to assign cgroup: " << assign.error() << std::endl;
      abort();
    }

    // Notify the parent.
    ::close(pipes[0]); // TODO(benh): Close after first fork?
    if (::write(pipes[1], &dummy, sizeof(dummy)) != sizeof(dummy)) {
      perror("Failed to notify the parent");
      abort();
    }
    ::close(pipes[1]);

    // Wait kill signal from parent.
    while (true) ;

    // Should not reach here.
    std::cerr << "Reach an unreachable statement!" << std::endl;
    abort();
  }
}
