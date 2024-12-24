WORKDIR=`pwd`
# Build and install critical dependency
export ROOT=/usr
export INSTALL_DIR=${ROOT}/local
mkdir -p $INSTALL_DIR
#Build Webconfig framework
cd ${ROOT}
git  clone https://github.com/rdkcentral/WebconfigFramework.git
cd WebconfigFramework
autoreconf -i
export CFLAGS="-I/usr/local/include/rbus -I/usr/local/include/rtmessage"
./configure --prefix=/usr/local
make && make install
#Build rfc
cd ${ROOT}
git clone https://github.com/rdkcentral/rfc.git
cd rfc
autoreconf -i
./configure --enable-rfctool=yes --enable-tr181set=yes
cd rfcapi
cp /usr/local/lib/pkgconfig/libcjson.pc /usr/local/lib/pkgconfig/cjson.pc
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
make remotedebugger_CFLAGS="-I/usr/include/cjson -I/usr/local/include/wdmp-c -I/usr/local/include/rbus -I/usr/WebconfigFramework/include -I/usr/iarmmgrs/rdmmgr/include -I/usr/iarmmgrs/hal/include -I/usr/local/include/trower-base64" remotedebugger_LDFLAGS="-L/usr/local/lib -lsafec -lrdkloggers -lcjson -lrfcapi -lrbus -lmsgpackc -lsecure_wrapper -lwebconfig_framework -ltr181api  -L/usr/local/lib/x86_64-linux-gnu -ltrower-base64 -L/usr/lib/x86_64-linux-gnu"
