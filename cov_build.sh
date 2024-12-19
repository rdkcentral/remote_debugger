WORKDIR=`pwd`
# Build and install critical dependency
export ROOT=/usr
export INSTALL_DIR=${ROOT}/local
mkdir -p $INSTALL_DIR

cd $ROOT
#Build rbus
git clone https://github.com/rdkcentral/rbus
cmake -Hrbus -Bbuild/rbus -DBUILD_FOR_DESKTOP=ON -DCMAKE_BUILD_TYPE=Debug
make -C build/rbus && make -C build/rbus install
git clone https://github.com/djbclark/directfb-core-DirectFB.git
#Build dbus
git clone -b master https://github.com/d-bus/dbus.git
cmake -Hdbus -Bbuild/dbus -DBUILD_FOR_DESKTOP=ON -DCMAKE_BUILD_TYPE=Debug
make -C build/dbus && make -C build/dbus install

#Build wdmp-c
git clone https://github.com/xmidt-org/wdmp-c.git
cd wdmp-c
mkdir build
cd build
cmake ..
make
make install
sed -i '/WDMP_ERR_SESSION_IN_PROGRESS/a\    WDMP_ERR_INTERNAL_ERROR,\n    WDMP_ERR_DEFAULT_VALUE,' /usr/local/include/wdmp-c/wdmp-c.h
cd $ROOT
#Build rdk-logger
git clone https://github.com/rdkcentral/rdk_logger.git
apt install liblog4c-dev
cd $ROOT/rdk_logger
autoreconf -i
./configure
make LOG4C_LIBS="-L/usr/lib/x86_64-linux-gnu"
make install
#Build Webconfig framework
cd ${ROOT}
git  clone https://github.com/rdkcentral/WebconfigFramework.git
cd WebconfigFramework
autoreconf -i
export CFLAGS="-I/usr/local/include/rbus -I/usr/local/include/rtmessage"
./configure --prefix=/usr/local
make && make install
#Build libsyswrapper
cd ${ROOT}
git clone https://github.com/rdkcentral/libSyscallWrapper.git
cd ${ROOT}/libSyscallWrapper
autoupdate
autoreconf -i
./configure --prefix=${INSTALL_DIR}
make && make install
#Build rfc
cd ${ROOT}
git clone https://github.com/rdkcentral/rfc.git
cd rfc
autoreconf -i
./configure --enable-rfctool=yes --enable-tr181set=yes
cd rfcapi
make librfcapi_la_CPPFLAGS="-I/usr/include/cjson -I/usr/rfc/rfcMgr/gtest/mocks"
make install
cd /usr/rfc/tr181api
g++ -fPIC -shared -o libtr181api.so tr181api.cpp -I/usr/local/include/wdmp-c
mv ./libtr181api.so /usr/local/lib
mv ./tr181api.h /usr/local/include
#Build trower-base64
cd ${ROOT}
git clone https://github.com/xmidt-org/trower-base64.git
cd ${ROOT}/trower-base64
meson setup build
ninja -C build
ninja -C build install
cd $ROOT
git clone https://github.com/rdkcentral/iarmmgrs.git
cd $WORKDIR
autoreconf -i
autoupdate
./configure --prefix=${INSTALL_DIR}
make remotedebugger_CFLAGS="-I/usr/iarmbus/core/include -I/usr/iarmbus/core  -I/usr/include/cjson -I/usr/local/include/wdmp-c -I/usr/local/include/rbus -I/usr/WebconfigFramework/include -I/usr/iarmmgrs/rdmmgr/include -I/usr/iarmmgrs/hal/include -I/usr/local/include/trower-base64" remotedebugger_LDFLAGS="-L/usr/local/lib -lsafec -lrdkloggers -lcjson -lIARMBus -lrfcapi -lrbus -lmsgpackc -lsecure_wrapper -lsafec -lwebconfig_framework -ltr181api  -L/usr/local/lib/x86_64-linux-gnu -ltrower-base64 -L/usr/lib/x86_64-linux-gnu"
