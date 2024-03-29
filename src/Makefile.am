# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License

# Add 'subdir-objects' to build objects in respective subdirectories
# which is necessary for dealing with naming conflicts, i.e,,
# master/http.cpp and slave/http.cpp.
AUTOMAKE_OPTIONS = subdir-objects

include ../third_party/versions.am

# TODO(charles): Move these into an included automakefile and have
# them include $(top_builddir) as appropriate.
BOOST = third_party/boost-$(BOOST_VERSION)
DISTRIBUTE = third_party/distribute-$(DISTRIBUTE_VERSION)
PROTOBUF = third_party/protobuf-$(PROTOBUF_VERSION)
GLOG = third_party/glog-$(GLOG_VERSION)
GMOCK = third_party/gmock-$(GMOCK_VERSION)
GTEST = $(GMOCK)/gtest
LEVELDB = third_party/leveldb
ZOOKEEPER = third_party/zookeeper-$(ZOOKEEPER_VERSION)/src/c
LIBPROCESS = third_party/libprocess

# Unfortunatley, 'pkglibexecdir' and 'pkglocalstatedir' are not set
# before automake 1.11, so we need to set them manually (until we in
# the future assume an automake version).
pkglibexecdir = $(libexecdir)/$(PACKAGE)
pkglocalstatedir = $(localstatedir)/$(PACKAGE)

# Set install directory for webui artifacts (i.e., the location where
# anything with the webui_ prefix in this file will get
# installed). Also note that pkg*dir contains the package name in the
# path (in our case 'mesos') which is valuable because we'd prefer
# /path/to/mesos/webui versus something like /path/to/webui. Note that
# datadir (e.g., /usr/local/share) is for read-only "data" and
# localstatedir (e.g., /usr/local/var) is for modifiable "data".
webuidir = $(pkgdatadir)

# Protocol buffer compiler.
PROTOC = ../$(PROTOBUF)/src/protoc
PROTOCFLAGS = -I$(top_srcdir)/include/mesos -I$(srcdir)

# Initialize variables here so we can use += operator everywhere else.
lib_LTLIBRARIES =
noinst_LTLIBRARIES =
sbin_PROGRAMS =
bin_PROGRAMS =
pkglibexec_PROGRAMS =
dist_pkglibexec_SCRIPTS =
nobase_dist_webui_DATA =
nodist_sbin_SCRIPTS =
nobase_pkglocalstate_DATA =
check_PROGRAMS =
dist_check_SCRIPTS =
check_SCRIPTS =
BUILT_SOURCES =
TESTS =
CLEANFILES =
EXTRA_DIST =
PHONY_TARGETS =

# Add compiler and linker flags for pthreads.
AM_CXXFLAGS = $(PTHREAD_CFLAGS)
AM_LIBS = $(PTHREAD_LIBS)

# Setup CPPFLAGS that are used for most source files.
MESOS_CPPFLAGS = $(AM_CPPFLAGS)
MESOS_CPPFLAGS += -Wall -Werror
MESOS_CPPFLAGS += -DMESOS_WEBUI_DIR=\"$(webuidir)/webui\"
MESOS_CPPFLAGS += -DMESOS_LIBEXECDIR=\"$(pkglibexecdir)\"
MESOS_CPPFLAGS += -I$(top_srcdir)/include
MESOS_CPPFLAGS += -I$(top_srcdir)/$(LIBPROCESS)/include
MESOS_CPPFLAGS += -I../include
MESOS_CPPFLAGS += -I../$(BOOST)
MESOS_CPPFLAGS += -I../$(PROTOBUF)/src
MESOS_CPPFLAGS += -I../$(GLOG)/src

if WITH_INCLUDED_ZOOKEEPER
  MESOS_CPPFLAGS += -I../$(ZOOKEEPER)/include
  MESOS_CPPFLAGS += -I../$(ZOOKEEPER)/generated
endif


# README: we build the Mesos library out of a collection of
# convenience libraries (that is, libraries that do not get installed
# but we can use as building blocks to vary compile flags as necessary
# and then aggregate into final archives): libmesos_no_third_party.la
# libbuild.la, liblog.la, libjava.la.

# First, let's define necessary protocol buffer files.

MESOS_PROTO = $(top_srcdir)/include/mesos/mesos.proto

CXX_PROTOS = mesos.pb.cc mesos.pb.h
JAVA_PROTOS = java/generated/org/apache/mesos/Protos.java
PYTHON_PROTOS = python/src/mesos_pb2.py

BUILT_SOURCES += $(CXX_PROTOS) $(JAVA_PROTOS) $(PYTHON_PROTOS)
CLEANFILES += $(CXX_PROTOS) $(JAVA_PROTOS) $(PYTHON_PROTOS)

MESSAGES_PROTOS = messages/messages.pb.cc messages/messages.pb.h

BUILT_SOURCES += $(MESSAGES_PROTOS)
CLEANFILES += $(MESSAGES_PROTOS)

LOG_PROTOS = messages/log.pb.cc messages/log.pb.h

BUILT_SOURCES += $(LOG_PROTOS)
CLEANFILES += $(LOG_PROTOS)

STATE_PROTOS = messages/state.pb.cc messages/state.pb.h

BUILT_SOURCES += $(STATE_PROTOS)
CLEANFILES += $(STATE_PROTOS)


# Targets for generating protocol buffer code.
%.pb.cc %.pb.h: $(top_srcdir)/include/mesos/%.proto
	@$(MKDIR_P) $(@D)
	@$(PROTOC) $(PROTOCFLAGS) --cpp_out=. $^

%.pb.cc %.pb.h: %.proto
	@$(MKDIR_P) $(@D)
	@$(PROTOC) $(PROTOCFLAGS) --cpp_out=. $^

$(JAVA_PROTOS): $(MESOS_PROTO)
	@$(MKDIR_P)  $(@D)
	@$(PROTOC) $(PROTOCFLAGS) --java_out=java/generated $^

$(PYTHON_PROTOS): $(MESOS_PROTO)
	@$(MKDIR_P) $(@D)
	@$(PROTOC) $(PROTOCFLAGS) --python_out=python/src $^

# We even use a convenience library for most of Mesos so that we can
# exclude third party libraries so setuptools/distribute can build a
# self-contained Python library and statically link in the third party
# libraries themselves.
noinst_LTLIBRARIES += libmesos_no_third_party.la

nodist_libmesos_no_third_party_la_SOURCES = $(CXX_PROTOS) $(MESSAGES_PROTOS)

libmesos_no_third_party_la_SOURCES = sched/sched.cpp local/local.cpp	\
	master/allocator.cpp master/drf_sorter.cpp			\
	master/frameworks_manager.cpp master/http.cpp master/master.cpp	\
	master/slaves_manager.cpp slave/gc.cpp slave/state.cpp		\
	slave/slave.cpp slave/http.cpp slave/isolation_module.cpp	\
	slave/process_based_isolation_module.cpp slave/reaper.cpp	\
	launcher/launcher.cpp exec/exec.cpp common/lock.cpp		\
	detector/detector.cpp configurator/configurator.cpp		\
	common/date_utils.cpp common/resources.cpp			\
	common/attributes.cpp common/values.cpp files/files.cpp		\
	logging/logging.cpp zookeeper/zookeeper.cpp			\
	zookeeper/authentication.cpp zookeeper/group.cpp		\
	messages/messages.proto

pkginclude_HEADERS = $(top_srcdir)/include/mesos/executor.hpp	\
		     $(top_srcdir)/include/mesos/scheduler.hpp	\
		     $(top_srcdir)/include/mesos/mesos.proto

nodist_pkginclude_HEADERS = ../include/mesos/mesos.hpp mesos.pb.h

if OS_LINUX
  libmesos_no_third_party_la_SOURCES += slave/cgroups_isolation_module.cpp
  libmesos_no_third_party_la_SOURCES += slave/lxc_isolation_module.cpp
  libmesos_no_third_party_la_SOURCES += linux/cgroups.cpp
  libmesos_no_third_party_la_SOURCES += linux/fs.cpp
  libmesos_no_third_party_la_SOURCES += linux/proc.cpp
else
  EXTRA_DIST += slave/cgroups_isolation_module.cpp
  EXTRA_DIST += slave/lxc_isolation_module.cpp
  EXTRA_DIST += linux/cgroups.cpp
  EXTRA_DIST += linux/fs.cpp
  EXTRA_DIST += linux/proc.cpp
endif

EXTRA_DIST += slave/solaris_project_isolation_module.cpp

libmesos_no_third_party_la_SOURCES += common/attributes.hpp		\
	common/build.hpp common/date_utils.hpp common/factory.hpp	\
	common/protobuf_utils.hpp					\
	common/lock.hpp common/resources.hpp common/process_utils.hpp	\
	common/type_utils.hpp common/thread.hpp common/units.hpp	\
	common/values.hpp configurator/configuration.hpp		\
	configurator/configurator.hpp configurator/option.hpp		\
	detector/detector.hpp examples/utils.hpp files/files.hpp	\
	flags/flag.hpp flags/flags.hpp flags/loader.hpp			\
	flags/parse.hpp launcher/launcher.hpp linux/cgroups.hpp		\
	linux/fs.hpp linux/proc.hpp local/flags.hpp local/local.hpp	\
	logging/flags.hpp logging/logging.hpp master/allocator.hpp	\
	master/constants.hpp master/drf_sorter.hpp master/flags.hpp	\
	master/frameworks_manager.hpp					\
	master/hierarchical_allocator_process.hpp master/http.hpp	\
	master/master.hpp master/slaves_manager.hpp master/sorter.hpp	\
	messages/messages.hpp slave/constants.hpp			\
	slave/flags.hpp slave/gc.hpp slave/http.hpp			\
	slave/isolation_module.hpp slave/isolation_module_factory.hpp	\
	slave/cgroups_isolation_module.hpp				\
	slave/lxc_isolation_module.hpp					\
	slave/paths.hpp slave/state.hpp					\
	slave/process_based_isolation_module.hpp slave/reaper.hpp	\
	slave/slave.hpp slave/solaris_project_isolation_module.hpp	\
	tests/environment.hpp tests/script.hpp				\
	tests/assert.hpp                         			\
	tests/zookeeper_test.hpp tests/flags.hpp tests/utils.hpp	\
	tests/filter.hpp                                                \
	tests/zookeeper_test_server.hpp zookeeper/authentication.hpp	\
	zookeeper/group.hpp zookeeper/watcher.hpp			\
	zookeeper/zookeeper.hpp zookeeper/url.hpp

libmesos_no_third_party_la_CPPFLAGS = $(MESOS_CPPFLAGS)

libmesos_no_third_party_la_LIBADD = # Initialized to allow += below.

# Convenience library that *always* gets rebuilt to ensure accurate info.
noinst_LTLIBRARIES += libbuild.la
libbuild_la_SOURCES = common/build.cpp
libbuild_la_CPPFLAGS = $(AM_CPPFLAGS)
libbuild_la_CPPFLAGS += -DBUILD_DATE="\"$$(date '+%Y-%m-%d %H:%M:%S')\""
libbuild_la_CPPFLAGS += -DBUILD_TIME="\"$$(date '+%s')\""
libbuild_la_CPPFLAGS += -DBUILD_USER="\"$$USER\""

# We need to escape the build flags properly.
BUILD_FLAGS = $(echo $(MESOS_CPPFLAGS) $(CPPFLAGS) | sed 's/\"/\\\"/g') \
              $(echo $(AM_CFLAGS) $(CFLAGS) | sed 's/\"/\\\"/g') \
              $(echo $(AM_CXXFLAGS) $(CXXFLAGS) | sed 's/\"/\\\"/g')

# TODO(benh): Provide other build flags.

libbuild_la_CPPFLAGS += -DBUILD_FLAGS="\"$$BUILD_FLAGS\""

libmesos_no_third_party_la_LIBADD += libbuild.la


# Convenience library for building the replicated log in order to
# include the leveldb headers.
noinst_LTLIBRARIES += liblog.la
liblog_la_SOURCES = log/coordinator.cpp log/replica.cpp
liblog_la_SOURCES += log/coordinator.hpp log/replica.hpp log/log.hpp	\
  log/network.hpp messages/log.hpp messages/log.proto
nodist_liblog_la_SOURCES = $(LOG_PROTOS)
liblog_la_CPPFLAGS = -I../$(LEVELDB)/include $(MESOS_CPPFLAGS)

libmesos_no_third_party_la_LIBADD += liblog.la


# Convenience library for building "state" abstraction in order to
# include the leveldb headers.
noinst_LTLIBRARIES += libstate.la
libstate_la_SOURCES = state/leveldb.cpp state/zookeeper.cpp
libstate_la_SOURCES += state/leveldb.hpp state/serializer.hpp	\
  state/state.hpp state/zookeeper.hpp messages/state.hpp	\
  messages/state.proto
nodist_libstate_la_SOURCES = $(STATE_PROTOS)
libstate_la_CPPFLAGS = -I../$(LEVELDB)/include $(MESOS_CPPFLAGS)

libmesos_no_third_party_la_LIBADD += libstate.la

# The final result!
lib_LTLIBRARIES += libmesos.la

libmesos_la_SOURCES = $(MESOS_PROTO) # Part of the distribution.

libmesos_la_LDFLAGS = -release $(PACKAGE_VERSION) -shared

# Since we just include the convenience library (and no sources), we
# need to tell libtool to build this as a C++ library.
libmesos_la_LIBTOOLFLAGS = --tag=CXX

# Add the convenience library.
libmesos_la_LIBADD = libmesos_no_third_party.la

# For non-convenience libraries we need to link them in to make the shared
# library each time. (Currently, we don't support platforms where this is not
# possible.)
libmesos_la_LIBADD += ../$(PROTOBUF)/src/libprotobuf.la
libmesos_la_LIBADD += ../$(GLOG)/libglog.la

# We need to directly include the leveldb library in order to avoid
# the installed libmesos.la file to include leveldb in
# 'dependency_libs' (via '-L../third_party/leveldb -lleveldb').
libmesos_la_LIBADD += ../$(LEVELDB)/libleveldb.a

if WITH_INCLUDED_ZOOKEEPER
  libmesos_la_LIBADD += ../$(ZOOKEEPER)/libzookeeper_mt.la
endif

libmesos_la_LIBADD += ../$(LIBPROCESS)/libprocess.la


# Binaries.
sbin_PROGRAMS += mesos-master
mesos_master_SOURCES = master/main.cpp
mesos_master_CPPFLAGS = $(MESOS_CPPFLAGS)
mesos_master_LDADD = libmesos.la

sbin_PROGRAMS += mesos-slave
mesos_slave_SOURCES = slave/main.cpp
mesos_slave_CPPFLAGS = $(MESOS_CPPFLAGS)
mesos_slave_LDADD = libmesos.la

bin_PROGRAMS += mesos-local
mesos_local_SOURCES = local/main.cpp
mesos_local_CPPFLAGS = $(MESOS_CPPFLAGS)
mesos_local_LDADD = libmesos.la

pkglibexec_PROGRAMS += mesos-launcher
mesos_launcher_SOURCES = launcher/main.cpp
mesos_launcher_CPPFLAGS = $(MESOS_CPPFLAGS)
mesos_launcher_LDADD = libmesos.la

pkglibexec_PROGRAMS += mesos-executor
mesos_executor_SOURCES = launcher/executor.cpp
mesos_executor_CPPFLAGS = $(MESOS_CPPFLAGS)
mesos_executor_LDADD = libmesos.la

bin_PROGRAMS += mesos-log
mesos_log_SOURCES = log/main.cpp
mesos_log_CPPFLAGS = $(MESOS_CPPFLAGS)
mesos_log_LDADD = libmesos.la

# TODO(benh): Support Solaris.
# bin_PROGRAMS += mesos-projd
# mesos_projd_SOURCES = slave/projd.cpp
# mesos_projd_CPPFLAGS = $(MESOS_CPPFLAGS)
# mesos_projd_LDADD = libmesos.la

bin_PROGRAMS += mesos-mesos
mesos_mesos_SOURCES = mesos/main.cpp
mesos_mesos_CPPFLAGS = $(MESOS_CPPFLAGS)
mesos_mesos_LDADD = libmesos.la

# Need to distribute/install webui javascript.
nobase_dist_webui_DATA += webui/master/static/controllers.js		\
  webui/master/static/app.js webui/master/static/dashboard.js		\
  webui/master/static/popovers.js webui/master/static/jquery.pailer.js

# Need to distribute/install webui CSS.
nobase_dist_webui_DATA += webui/master/static/graph.css	\
  webui/master/static/mesos.css

# Need to distribute/install webui HTML.
nobase_dist_webui_DATA += webui/master/static/dashboard.html		\
  webui/master/static/framework.html					\
  webui/master/static/frameworks.html webui/master/static/home.html	\
  webui/master/static/index.html webui/master/static/pailer.html	\
  webui/master/static/slaves.html

# Need to distribute/install webui images.
nobase_dist_webui_DATA += webui/master/static/loading.gif

# Need to distribute/install third_party javascript.
nobase_dist_webui_DATA += webui/master/static/angular-1.0.0rc8.js	\
  webui/master/static/underscore-min.js					\
  webui/master/static/cubism.v1.js webui/master/static/d3.v2.js		\
  webui/master/static/relative-date.js

# Need to distribute/install bootstrap.
nobase_dist_webui_DATA +=							  \
  webui/master/static/bootstrap/css/bootstrap-responsive.css			  \
  webui/master/static/bootstrap/css/bootstrap.css				  \
  webui/master/static/bootstrap/css/docs.css					  \
  webui/master/static/bootstrap/ico/favicon.ico					  \
  webui/master/static/bootstrap/img/bird.png					  \
  webui/master/static/bootstrap/img/bootstrap-mdo-sfmoma-01.jpg			  \
  webui/master/static/bootstrap/img/bootstrap-mdo-sfmoma-02.jpg			  \
  webui/master/static/bootstrap/img/bootstrap-mdo-sfmoma-03.jpg			  \
  webui/master/static/bootstrap/img/browsers.png				  \
  webui/master/static/bootstrap/img/github-16px.png				  \
  webui/master/static/bootstrap/img/glyphicons/glyphicons_009_magic.png		  \
  webui/master/static/bootstrap/img/glyphicons/glyphicons_042_group.png		  \
  webui/master/static/bootstrap/img/glyphicons/glyphicons_079_podium.png	  \
  webui/master/static/bootstrap/img/glyphicons/glyphicons_082_roundabout.png	  \
  webui/master/static/bootstrap/img/glyphicons/glyphicons_155_show_thumbnails.png \
  webui/master/static/bootstrap/img/glyphicons/glyphicons_163_iphone.png	  \
  webui/master/static/bootstrap/img/glyphicons/glyphicons_214_resize_small.png	  \
  webui/master/static/bootstrap/img/glyphicons/glyphicons_266_book_open.png	  \
  webui/master/static/bootstrap/img/glyphicons-halflings-white.png		  \
  webui/master/static/bootstrap/img/glyphicons-halflings.png			  \
  webui/master/static/bootstrap/img/grid-18px-masked.png			  \
  webui/master/static/bootstrap/img/icon-css3.png				  \
  webui/master/static/bootstrap/img/icon-github.png				  \
  webui/master/static/bootstrap/img/icon-html5.png				  \
  webui/master/static/bootstrap/img/icon-twitter.png				  \
  webui/master/static/bootstrap/img/less-logo-large.png				  \
  webui/master/static/bootstrap/img/less-small.png				  \
  webui/master/static/bootstrap/img/responsive-illustrations.png		  \
  webui/master/static/bootstrap/js/application.js				  \
  webui/master/static/bootstrap/js/bootstrap-alert.js				  \
  webui/master/static/bootstrap/js/bootstrap-button.js				  \
  webui/master/static/bootstrap/js/bootstrap-carousel.js			  \
  webui/master/static/bootstrap/js/bootstrap-collapse.js			  \
  webui/master/static/bootstrap/js/bootstrap-dropdown.js			  \
  webui/master/static/bootstrap/js/bootstrap-modal.js				  \
  webui/master/static/bootstrap/js/bootstrap-popover.js				  \
  webui/master/static/bootstrap/js/bootstrap-scrollspy.js			  \
  webui/master/static/bootstrap/js/bootstrap-tab.js				  \
  webui/master/static/bootstrap/js/bootstrap-tooltip.js				  \
  webui/master/static/bootstrap/js/bootstrap-transition.js			  \
  webui/master/static/bootstrap/js/bootstrap-typeahead.js			  \
  webui/master/static/bootstrap/js/bootstrap.js					  \
  webui/master/static/bootstrap/js/bootstrap.min.js				  \
  webui/master/static/bootstrap/js/google-code-prettify/prettify.css		  \
  webui/master/static/bootstrap/js/google-code-prettify/prettify.js		  \
  webui/master/static/bootstrap/js/jquery.js					  \
  webui/master/static/bootstrap/js/README.md


# And need to install the killtree script.
dist_pkglibexec_SCRIPTS += scripts/killtree.sh


# And the deploy related stuff.
nodist_sbin_SCRIPTS += deploy/mesos-daemon.sh			\
  deploy/mesos-start-cluster.sh deploy/mesos-start-masters.sh	\
  deploy/mesos-start-slaves.sh deploy/mesos-stop-cluster.sh	\
  deploy/mesos-stop-masters.sh deploy/mesos-stop-slaves.sh

nobase_pkglocalstate_DATA += deploy/mesos-deploy-env.sh.template

# Need to explicitly add this because by default DATA files are not
# included in distributions.
EXTRA_DIST += deploy/mesos-deploy-env.sh.template


# And the configuration file.
nobase_pkglocalstate_DATA += conf/mesos.conf.template

# Need to explicitly add this because by default DATA files are not
# included in distributions.
EXTRA_DIST += conf/mesos.conf.template


# Java related files are listed outside if HAS_JAVA so we can add them
# to EXTRA_DIST unconditionally.

# Mesos JAR.
MESOS_JAR = mesos-$(PACKAGE_VERSION).jar
MESOS_JAR_SOURCE =							\
	$(srcdir)/java/src/org/apache/mesos/ExecutorDriver.java		\
	$(srcdir)/java/src/org/apache/mesos/Executor.java		\
	$(srcdir)/java/src/org/apache/mesos/Log.java			\
	$(srcdir)/java/src/org/apache/mesos/MesosExecutorDriver.java	\
	$(srcdir)/java/src/org/apache/mesos/MesosSchedulerDriver.java	\
	$(srcdir)/java/src/org/apache/mesos/SchedulerDriver.java	\
	$(srcdir)/java/src/org/apache/mesos/Scheduler.java		\
	$(srcdir)/java/src/org/apache/mesos/state/InMemoryState.java	\
	$(srcdir)/java/src/org/apache/mesos/state/State.java		\
	$(srcdir)/java/src/org/apache/mesos/state/Variable.java		\
	$(srcdir)/java/src/org/apache/mesos/state/ZooKeeperState.java
MESOS_JAR_GENERATED = $(JAVA_PROTOS)					\
	java/generated/org/apache/mesos/MesosNativeLibrary.java
EXTRA_DIST += $(MESOS_JAR_SOURCE)					\
	$(srcdir)/java/generated/org/apache/mesos/MesosNativeLibrary.java.in

# We build all the Java examples into their own JAR.
EXAMPLES_JAR = examples.jar
EXAMPLES_SOURCE =							\
	$(srcdir)/examples/java/TestExceptionFramework.java		\
	$(srcdir)/examples/java/TestExecutor.java			\
	$(srcdir)/examples/java/TestFramework.java			\
	$(srcdir)/examples/java/TestMultipleExecutorsFramework.java
EXTRA_DIST += $(EXAMPLES_SOURCE)


if HAS_JAVA
# Protocol buffers JAR.
PROTOBUF_JAR = ../protobuf-$(PROTOBUF_VERSION).jar

# TODO(charles): Move into third_party/Makefile.am.
$(PROTOBUF_JAR): # TODO(charles): Specify dependencies for the jar.
	@echo "Building protobuf-$(PROTOBUF_VERSION).jar ..."
	@$(MKDIR_P) ../$(PROTOBUF)/java/src/main/java
	@$(PROTOC) --java_out=../$(PROTOBUF)/java/src/main/java \
          -I../$(PROTOBUF)/src \
          ../$(PROTOBUF)/src/google/protobuf/descriptor.proto
	@$(JAVA_HOME)/bin/javac \
          -d ../$(PROTOBUF)/java/src/main/java \
          ../$(PROTOBUF)/java/src/main/java/com/google/protobuf/*.java \
          ../$(PROTOBUF)/java/src/main/java/com/google/protobuf/*.java
	@$(JAVA_HOME)/bin/jar cf $@ -C ../$(PROTOBUF)/java/src/main/java com

CLEANFILES += $(PROTOBUF_JAR)


$(MESOS_JAR): $(MESOS_JAR_SOURCE) $(MESOS_JAR_GENERATED) $(PROTOBUF_JAR)
	@echo "Building mesos-$(PACKAGE_VERSION).jar ..."
	@rm -rf java/classes/org/apache/mesos
	@$(MKDIR_P) java/classes/org/apache/mesos
	@$(JAVA_HOME)/bin/javac \
          -cp $(PROTOBUF_JAR) \
          -d java/classes $(MESOS_JAR_SOURCE) $(MESOS_JAR_GENERATED)
	@$(JAVA_HOME)/bin/jar cf $@ -C java/classes org

CLEANFILES += $(MESOS_JAR)


MESOS_SOURCES_JAR = ../src/mesos-$(PACKAGE_VERSION)-sources.jar

$(MESOS_SOURCES_JAR): $(MESOS_JAR_SOURCE)
	@rm -f $(MESOS_SOURCES_JAR)
	@$(JAVA_HOME)/bin/jar cf $@ -C $(srcdir)/java/src org
	@$(JAVA_HOME)/bin/jar uf $@ -C java/generated org

CLEANFILES += $(MESOS_SOURCES_JAR)


# Convenience library for JNI bindings.
# TODO(Charles Reiss): We really should be building the Java library
# with -module, which means it should be a different library than the
# main libmesos.so.
noinst_LTLIBRARIES += libjava.la

libjava_la_SOURCES = java/jni/convert.cpp java/jni/construct.cpp	\
	java/jni/org_apache_mesos_Log.cpp				\
	java/jni/org_apache_mesos_MesosSchedulerDriver.cpp		\
	java/jni/org_apache_mesos_MesosExecutorDriver.cpp		\
	java/jni/org_apache_mesos_state_Variable.cpp			\
	java/jni/org_apache_mesos_state_ZooKeeperState.cpp jvm/jvm.cpp

libjava_la_SOURCES += java/jni/convert.hpp java/jni/construct.hpp jvm/jvm.hpp

libjava_la_CPPFLAGS = $(MESOS_CPPFLAGS)
libjava_la_CPPFLAGS += $(JAVA_CPPFLAGS)
libjava_la_CPPFLAGS += -I$(srcdir)/java/jni -Ijava/jni

libjava_la_DEPENDENCIES = $(MESOS_PROTO)

libjava_la_LIBADD = $(JAVA_LDFLAGS)

# We don't add libjava.la to libmesos_no_third_party.la so we don't
# include the JNI bindings in the Python egg (but we might want to
# reconsider this in the future). We also need to add the JAVA_LDFLAGS
# to libmesos.la so on Linux we set the rpath for libmesos.so.
libmesos_la_LIBADD += libjava.la
libmesos_la_LIBADD += $(JAVA_LDFLAGS)

# We need to generate the JNI header files before we build the sources
# and the easiest way to do that with autotools is with the
# BUILT_SOURCES mechanism. The downside of this is that a user cannot
# run 'make libjava.la' (or any target that uses BUILT_SOURCES) but
# that should not matter as libjava.la is just a convenience library
# and should not get exposed.

nodist_libjava_la_SOURCES =					\
	java/jni/org_apache_mesos_MesosSchedulerDriver.h	\
	java/jni/org_apache_mesos_MesosExecutorDriver.h		\
	java/jni/org_apache_mesos_Log.h				\
	java/jni/org_apache_mesos_state_Variable.h		\
	java/jni/org_apache_mesos_state_ZooKeeperState.h

BUILT_SOURCES += java/jni/org_apache_mesos_MesosSchedulerDriver.h	\
	java/jni/org_apache_mesos_MesosExecutorDriver.h			\
	java/jni/org_apache_mesos_Log.h					\
	java/jni/org_apache_mesos_state_Variable.h			\
	java/jni/org_apache_mesos_state_ZooKeeperState.h


java/jni/org_apache_mesos_MesosSchedulerDriver.h: $(MESOS_JAR)
	@$(JAVA_HOME)/bin/javah -d java/jni -classpath	\
          $(PROTOBUF_JAR):java/classes			\
          org.apache.mesos.MesosSchedulerDriver

java/jni/org_apache_mesos_MesosExecutorDriver.h: $(MESOS_JAR)
	@$(JAVA_HOME)/bin/javah -d java/jni -classpath	\
          $(PROTOBUF_JAR):java/classes			\
          org.apache.mesos.MesosExecutorDriver

java/jni/org_apache_mesos_Log.h: $(MESOS_JAR)
	@$(JAVA_HOME)/bin/javah -d java/jni -classpath		\
          $(PROTOBUF_JAR):java/classes org.apache.mesos.Log

java/jni/org_apache_mesos_state_Variable.h: $(MESOS_JAR)
	@$(JAVA_HOME)/bin/javah -d java/jni -classpath		\
          $(PROTOBUF_JAR):java/classes org.apache.mesos.state.Variable

java/jni/org_apache_mesos_state_ZooKeeperState.h: $(MESOS_JAR)
	@$(JAVA_HOME)/bin/javah -d java/jni -classpath		\
          $(PROTOBUF_JAR):java/classes org.apache.mesos.state.ZooKeeperState

$(EXAMPLES_JAR): $(EXAMPLES_SOURCE)
	@echo "Building examples.jar ..."
	@$(MKDIR_P) examples/java
	@$(JAVA_HOME)/bin/javac -cp					\
          $(PROTOBUF_JAR):$(MESOS_JAR):$(srcdir)/examples/java		\
          -sourcepath $(srcdir)/examples/java -d examples/java	        \
          $(srcdir)/examples/java/*.java
	@$(JAVA_HOME)/bin/jar cf $@ -C examples/java .

CLEANFILES += $(EXAMPLES_JAR)

maven-install: $(MESOS_JAR) $(MESOS_SOURCES_JAR) java/mesos.pom
	mvn install:install-file -Dfile=$(MESOS_JAR)	\
          -Dsources=$(MESOS_SOURCES_JAR) -DpomFile=java/mesos.pom

PHONY_TARGETS += maven-install
endif # HAS_JAVA

# We remove mesos-*.jar here to make sure any older versions of the
# jar are removed.
clean-java:
	-rm -rf java/classes
	-rm -f examples/java/*.class
	-rm -f java/jni/org_apache_mesos*.h
	-rm -f mesos-*.jar

PHONY_TARGETS += clean-java


# Python files listed outside HAS_PYTHON so they are included with the
# distribution unconditionally.
EXTRA_DIST += python/src/mesos.py				\
	      python/native/mesos_executor_driver_impl.cpp	\
	      python/native/mesos_executor_driver_impl.hpp	\
	      python/native/mesos_scheduler_driver_impl.cpp	\
	      python/native/mesos_scheduler_driver_impl.hpp	\
	      python/native/module.cpp python/native/module.hpp	\
	      python/native/proxy_executor.cpp			\
	      python/native/proxy_executor.hpp			\
	      python/native/proxy_scheduler.cpp			\
	      python/native/proxy_scheduler.hpp

if HAS_PYTHON
# Used for building Python eggs.
DISTRIBUTE_EGG_SUFFIX = $(DISTRIBUTE_VERSION)$(PYTHON_EGG_PUREPY_POSTFIX).egg
DISTRIBUTE_EGG = \
  $(abs_top_builddir)/$(DISTRIBUTE)/dist/distribute-$(DISTRIBUTE_EGG_SUFFIX)

# Build a protobuf Python egg.
PROTOBUF_EGG_SUFFIX = $(PROTOBUF_VERSION)$(PYTHON_EGG_PUREPY_POSTFIX).egg
PROTOBUF_EGG = ../$(PROTOBUF)/python/dist/protobuf-$(PROTOBUF_EGG_SUFFIX)

$(PROTOBUF_EGG):
	@echo "Building protobuf Python egg ..."
	@cd ../$(PROTOBUF)/python && \
	  PYTHONPATH=$(DISTRIBUTE_EGG) $(PYTHON) setup.py bdist_egg

CLEANFILES += $(PROTOBUF_EGG)

# This builds a Python egg against libmesos_no_third_party.a that is
# self-contained. It currently depends on the libraries in third_party
# being built as .a's. (If this is changed, the setup.py will need to
# be edited).

# TODO(Charles Reiss): Figure out how to do a proper out-of-source
# build of these Python eggs. Right now we copy the entire source
# directories into the build directory.

# TODO(Charles Reiss): Additionally support building and installing
# Python libraries that link against the system libmesos.so. This
# should be the default mode if we eventually are able to rely on
# system versions of our dependencies (since we won't be able to
# create a standalone egg anyways).

MESOS_EGG = python/dist/mesos-$(PACKAGE_VERSION)$(PYTHON_EGG_POSTFIX).egg

$(MESOS_EGG): python/setup.py $(srcdir)/python/src/mesos.py	\
              $(PYTHON_PROTOS) libmesos_no_third_party.la	\
              $(PROTOBUF_EGG)
	@echo "Building Mesos Python egg ..."
	@if test "$(top_srcdir)" != "$(top_builddir)"; then \
	  $(MKDIR_P) python/src; \
	  cp -pf $(srcdir)/python/src/mesos.py python/src; \
	fi
	@LIBS="$(LIBS)" PYTHONPATH=$(DISTRIBUTE_EGG) \
	  $(PYTHON) python/setup.py bdist_egg

CLEANFILES += $(MESOS_EGG) python/build/temp.*/native/*.o python/build/lib.*/*

# Make sure the egg gets built via 'make all'.
all-local: $(MESOS_EGG)
endif # HAS_PYTHON

# Note that to clean up the copied Python files we have to 'chmod u+w
# python/native' because creating a distribution can eliminate write
# permissions for the owner which are conserved even after we 'cp -p'
# so we won't be able to delete the files until we update permissions.
#
# We remove mesos-*.egg here to make sure any older versions of the
# egg or versions for different architectures are removed.
clean-python:
	test "$(top_srcdir)" = "$(top_builddir)" || \
	  (chmod -R u+w python/native; \
	   rm -rf python/src/mesos.py python/native)
	-rm -rf python/src/mesos.egg-info python/build
	-rm -f python/dist/mesos-*.egg

PHONY_TARGETS += clean-python


# Test (make check) binaries.
check_PROGRAMS += test-framework
test_framework_SOURCES = examples/test_framework.cpp
test_framework_CPPFLAGS = $(MESOS_CPPFLAGS)
test_framework_LDADD = libmesos.la

check_PROGRAMS += test-executor
test_executor_SOURCES = examples/test_executor.cpp
test_executor_CPPFLAGS = $(MESOS_CPPFLAGS)
test_executor_LDADD = libmesos.la

check_PROGRAMS += long-lived-framework
long_lived_framework_SOURCES = examples/long_lived_framework.cpp
long_lived_framework_CPPFLAGS = $(MESOS_CPPFLAGS)
long_lived_framework_LDADD = libmesos.la

check_PROGRAMS += long-lived-executor
long_lived_executor_SOURCES = examples/long_lived_executor.cpp
long_lived_executor_CPPFLAGS = $(MESOS_CPPFLAGS)
long_lived_executor_LDADD = libmesos.la

check_PROGRAMS += no-executor-framework
no_executor_framework_SOURCES = examples/no_executor_framework.cpp
no_executor_framework_CPPFLAGS = $(MESOS_CPPFLAGS)
no_executor_framework_LDADD = libmesos.la

check_PROGRAMS += balloon-framework
balloon_framework_SOURCES = examples/balloon_framework.cpp
balloon_framework_CPPFLAGS = $(MESOS_CPPFLAGS)
balloon_framework_LDADD = libmesos.la

check_PROGRAMS += balloon-executor
balloon_executor_SOURCES = examples/balloon_executor.cpp
balloon_executor_CPPFLAGS = $(MESOS_CPPFLAGS)
balloon_executor_LDADD = libmesos.la

check_PROGRAMS += mesos-tests

mesos_tests_SOURCES = tests/main.cpp tests/utils.cpp tests/filter.cpp  	\
	              tests/environment.cpp				\
	              tests/master_tests.cpp tests/state_tests.cpp	\
	              tests/slave_state_tests.cpp			\
	              tests/gc_tests.cpp				\
	              tests/resource_offers_tests.cpp			\
	              tests/fault_tolerance_tests.cpp			\
	              tests/files_tests.cpp tests/flags_tests.cpp	\
	              tests/log_tests.cpp tests/resources_tests.cpp	\
	              tests/script.cpp					\
	              tests/examples_tests.cpp				\
	              tests/configurator_tests.cpp			\
	              tests/multihashmap_tests.cpp			\
	              tests/protobuf_io_tests.cpp			\
	              tests/stout_tests.cpp				\
	              tests/zookeeper_url_tests.cpp			\
	              tests/killtree_tests.cpp				\
	              tests/exception_tests.cpp				\
	              tests/attributes_tests.cpp			\
	              tests/master_detector_tests.cpp			\
	              tests/sorter_tests.cpp tests/allocator_tests.cpp	\
	              tests/logging_tests.cpp

mesos_tests_CPPFLAGS = $(MESOS_CPPFLAGS)
mesos_tests_CPPFLAGS += -DSOURCE_DIR=\"$(abs_top_srcdir)\"
mesos_tests_CPPFLAGS += -DBUILD_DIR=\"$(abs_top_builddir)\"
mesos_tests_CPPFLAGS += -I../$(GTEST)/include
mesos_tests_CPPFLAGS += -I../$(GMOCK)/include

mesos_tests_LDADD = ../third_party/libgmock.la libmesos.la

mesos_tests_DEPENDENCIES = # Initialized to allow += below.

if OS_LINUX
  mesos_tests_SOURCES += tests/cgroups_isolation_tests.cpp
  mesos_tests_SOURCES += tests/cgroups_tests.cpp
  mesos_tests_SOURCES += tests/fs_tests.cpp
  mesos_tests_SOURCES += tests/proc_tests.cpp
endif

if HAS_JAVA
  mesos_tests_SOURCES += tests/zookeeper_test.cpp		\
                         tests/zookeeper_test_server.cpp	\
                         tests/zookeeper_tests.cpp		\
                         tests/allocator_zookeeper_tests.cpp
  mesos_tests_CPPFLAGS += $(JAVA_CPPFLAGS)
  mesos_tests_CPPFLAGS += -DZOOKEEPER_VERSION=\"$(ZOOKEEPER_VERSION)\"
  mesos_tests_LDFLAGS = $(JAVA_LDFLAGS) $(AM_LDFLAGS)
  mesos_tests_DEPENDENCIES += $(EXAMPLES_JAR)

  EXAMPLESSCRIPTSJAVA = examples/java/test-framework			\
			examples/java/test-executor			\
			examples/java/test-exception-framework		\
			examples/java/test-multiple-executors-framework

  check_SCRIPTS += $(EXAMPLESCRIPTSJAVA)
  mesos_tests_DEPENDENCIES += $(EXAMPLESCRIPTSJAVA)
endif

if HAS_PYTHON
  mesos_tests_DEPENDENCIES += $(MESOS_EGG)

  EXAMPLESCRIPTSPYTHON = examples/python/test_framework.py	\
			 examples/python/test-framework		\
			 examples/python/test_executor.py	\
			 examples/python/test-executor

  check_SCRIPTS += $(EXAMPLESCRIPTSPYTHON)
  mesos_tests_DEPENDENCIES += $(EXAMPLESCRIPTSPYTHON)
endif

EXTRA_DIST += examples/python/test_framework.py \
	      examples/python/test_executor.py


dist_check_SCRIPTS +=				\
  tests/balloon_framework_test.sh		\
  tests/test_framework_test.sh			\
  tests/no_executor_framework_test.sh		\
  tests/java_exception_test.sh			\
  tests/java_framework_test.sh			\
  tests/python_framework_test.sh		\
  tests/killtree_test.sh

TESTS += mesos-tests

clean-local: clean-java clean-python

.PHONY: $(PHONY_TARGETS)
