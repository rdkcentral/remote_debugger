##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2018 RDK Management
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
##########################################################################

WORKDIR=`pwd`
# Build and install critical dependency
export ROOT=/usr
export INSTALL_DIR=/usr/local
mkdir -p $INSTALL_DIR


# Build tr181api from rfc source code
cd ${ROOT}

# Clone all dependencies if not present already
if [ ! -d rfc ]; then
    git clone https://github.com/rdkcentral/rfc.git
fi

if [ ! -d iarmmgrs ]; then
    git clone https://github.com/rdkcentral/iarmmgrs.git
fi

if [ ! -d iarmbus ]; then
    git clone https://github.com/rdkcentral/iarmbus.git
fi

if [ ! -d tr69hostif ]; then
    git clone https://github.com/rdkcentral/tr69hostif.git
fi

cd rfc
autoreconf -i
./configure --enable-rfctool=yes --enable-tr181set=yes
cd rfcapi
make librfcapi_la_CPPFLAGS="-I/usr/include/cjson -I/usr/rfc/rfcMgr/gtest/mocks"
make install
cd /usr/rfc/tr181api
g++ -fPIC -shared -o libtr181api.so tr181api.cpp -I/usr/local/include/wdmp-c
mv ./libtr181api.so /usr/local/lib
cp ./tr181api.h /usr/local/include

# Install header files alone from armmgrs repositories
cd $ROOT

cp /usr/iarmmgrs/rdmmgr/include/rdmMgr.h /usr/local/include

# Install header files alone from iarmbus repositories
cp /usr/iarmbus/core/include/libIBusDaemon.h /usr/local/include
cp /usr/iarmbus/core/include/libIBus.h /usr/local/include
cp /usr/iarmbus/core/libIARMCore.h /usr/local/include
cp /usr/iarmmgrs/hal/include/pwrMgr.h /usr/local/include/

# Build and install stubs from tr69hostif

cd tr69hostif
cp ./src/hostif/parodusClient/waldb/data-model/data-model-generic.xml /etc
cd ./src/unittest/stubs
g++ -fPIC -shared -o libIARMBus.so iarm_stubs.cpp  -I/usr/tr69hostif/src/hostif/parodusClient/pal -I/usr/tr69hostif/src/unittest/stubs -I/usr/tr69hostif/src/hostif/parodusClient/waldb -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/tr69hostif/src/hostif/include -I/usr/tr69hostif/src/hostif/profiles/DeviceInfo -I/usr/tr69hostif/src/hostif/parodusClient/pal -fpermissive
cp libIARMBus.so /usr/local/lib
cp libIBus.h /usr/local/include
cp libIARM.h /usr/local/include

cd $WORKDIR
autoreconf -i
autoupdate
./configure --prefix=${INSTALL_DIR} --enable-iarmbusSupport=yes --enable-L2support=yes
make remotedebugger_CFLAGS="-I/usr/include/cjson -I/usr/local/include/wdmp-c -I/usr/local/include/rbus -I/usr/local/include -I/usr/local/include/trower-base64 -DIARMBUS_SUPPORT -DUSECOV -DUSE_L2_SUPPORT" remotedebugger_LDFLAGS="-L/usr/local/lib -lrdkloggers -lcjson -lrfcapi -lrbus -lmsgpackc -lsecure_wrapper -lwebconfig_framework -lIARMBus -ltr181api  -L/usr/local/lib/x86_64-linux-gnu -ltrower-base64 -L/usr/lib/x86_64-linux-gnu"
make install
