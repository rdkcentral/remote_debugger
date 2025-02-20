#!/bin/sh
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

ENABLE_COV=false

if [ "x$1" = "x--enable-cov" ]; then
      echo "Enabling coverage options"
      export CXXFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"
      export CFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"
      export LDFLAGS="-lgcov --coverage"
      ENABLE_COV=true
fi

export TOP_DIR=`pwd`
cd ./src/unittest/

echo "********************"
echo "**** RUNNING UT STARTS ****"
echo "********************"
automake --add-missing
autoreconf --install

echo "********************"
echo "**** BUILD RDK REMOTE DEBUGGER UT ****"
echo "********************"
./configure
make

echo "********************"
echo "**** RUN RDK REMOTE DEBUGGER UT ****"
echo "********************"
./remotedebugger_gtest
gRRDUTret=$?

if [ "0x$gRRDUTret" != "0x0"  ]; then
   echo "Error!!! RDK Remote Debugger UT FAILED. EXIT!!!"
   exit 1
fi

echo "********************"
echo "**** CAPTURE RDK REMOTE DEBUGGER COVERAGE DATA ****"
echo "********************"
if [ "$ENABLE_COV" = true ]; then
    echo "Generating coverage report"
    lcov --capture --directory . --output-file coverage.info
    lcov --remove coverage.info '/usr/*' --output-file coverage.info
    lcov --list coverage.info
fi

cd $TOP_DIR
echo "********************"
echo "**** RUNNING UT ENDS ****"
echo "********************"
