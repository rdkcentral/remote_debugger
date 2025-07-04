# Copyright 2023 Comcast Cable Communications Management, LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
#

# Define the program name and the source files
bin_PROGRAMS = remotedebugger_gtest

# Define the include directories
COMMON_CPPFLAGS = -I../ -I../../ -I./mocks -I/usr/include/cjson -I/usr/include/nettle -I/usr/include/msgpack -I/usr/local/include -DGTEST_ENABLE

# Define the libraries to link against
COMMON_LDADD = -lgtest -lgtest_main -lgmock_main -lgmock -lcjson -lmsgpackc -lgcov

# Define the compiler flags
COMMON_CXXFLAGS = -frtti -fprofile-arcs -ftest-coverage

# Define the source files
remotedebugger_gtest_SOURCES = rrdUnitTestRunner.cpp 

# Apply common properties to each program
remotedebugger_gtest_CPPFLAGS = $(COMMON_CPPFLAGS)
remotedebugger_gtest_LDADD = $(COMMON_LDADD)
remotedebugger_gtest_CXXFLAGS = $(COMMON_CXXFLAGS)
