WORKDIR=`pwd`
# Build and install critical dependency
export ROOT=/usr
export INSTALL_DIR=${ROOT}/local
mkdir -p $INSTALL_DIR
apt-get install libdirectfb-dev
cd $ROOT
#Build rbus
git clone https://github.com/rdkcentral/rbus
cmake -Hrbus -Bbuild/rbus -DBUILD_FOR_DESKTOP=ON -DCMAKE_BUILD_TYPE=Debug
make -C build/rbus && make -C build/rbus install
#Build wdmp-c
git clone https://github.com/xmidt-org/wdmp-c.git
sed -i '/WDMP_ERR_SESSION_IN_PROGRESS/a\    WDMP_ERR_INTERNAL_ERROR,\n    WDMP_ERR_DEFAULT_VALUE,' /usr/wdmp-c/src/wdmp-c.h
cd $ROOT
#Build rdk-logger
git clone https://github.com/rdkcentral/rdk_logger.git
wget --no-check-certificate https://sourceforge.net/projects/log4c/files/log4c/1.2.4/log4c-1.2.4.tar.gz/download -O log4c-1.2.4.tar.gz
tar -xvf log4c-1.2.4.tar.gz
cd log4c-1.2.4
./configure
make && make install
cd $ROOT/rdk_logger
autoreconf -i
./configure
make sudo make LOG4C_LIBS="-L/usr/lib/x86_64-linux-gnu"
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
make && sudo make install
#Build rfc
cd ${ROOT}
git clone git@github.com:rdkcentral/rfc.git
cd rfc
autoreconf -i
./configure --enable-rfctool=yes --enable-tr181set=yes
cd rfcapi
make librfcapi_la_CPPFLAGS="-I/usr/include/cjson"
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
git clone https://github.com/rurban/safeclib
cd $ROOT/safeclib
autoreconf --install
./configure --prefix=${INSTALL_DIR} && make && make install
cd $ROOT
git clone https://github.com/rdkcentral/iarmbus.git
cd iarmbus
git checkout iarmbus_52984
make INCLUDE_FILES="-I/usr/include/glib-2.0 -I/usr/iarmbus/core/include -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/dbus-1.0/ -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/safeclib/src/str -I/usr/local/include/safeclib" IARMDaemonMain_LDADD="-L/usr/local/lib -lsafec"
make install
cd $ROOT
git clone https://github.com/rdkcentral/iarmmgrs.git
cd $WORKDIR
autoreconf -i
autoupdate
./configure --prefix=${INSTALL_DIR}
make remotedebugger_CFLAGS="-I/usr/iarmbus/core/include -I/usr/iarmbus/core  -I/usr/include/cjson -I/usr/local/include/wdmp-c -I/usr/local/include/rbus -I/usr/WebconfigFramework/include -I/usr/iarmmgrs/rdmmgr/include -I/usr/iarmmgrs/hal/include -I/usr/local/include/trower-base64" remotedebugger_LDFLAGS="-L/usr/local/lib -lsafec -lrdkloggers -lcjson -lIARMBus -lrfcapi -lrbus -lmsgpackc -lsecure_wrapper -lsafec -lwebconfig_framework -ltr181api  -L/usr/local/lib/x86_64-linux-gnu -ltrower-base64 -L/usr/lib/x86_64-linux-gnu"
