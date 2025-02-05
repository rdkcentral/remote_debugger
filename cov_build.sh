WORKDIR=`pwd`
# Build and install critical dependency
export ROOT=/usr
export INSTALL_DIR=${ROOT}/local
mkdir -p $INSTALL_DIR
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
cp ./tr181api.h /usr/local/include
cd $ROOT
rm -rf iarmmgrs
rm -rf iarmbus
git clone https://github.com/rdkcentral/iarmmgrs.git
cp /usr/iarmmgrs/rdmmgr/include/rdmMgr.h /usr/local/include
git clone https://github.com/rdkcentral/iarmbus.git
cp /usr/iarmbus/core/include/libIBusDaemon.h /usr/local/include
cp /usr/iarmbus/core/include/libIBus.h /usr/local/include
cp /usr/iarmbus/core/libIARMCore.h /usr/local/include
cp /usr/iarmmgrs/hal/include/pwrMgr.h /usr/local/include/

git clone https://github.com/rdkcentral/tr69hostif.git
cd tr69hostif
cd ./src/unittest/stubs
g++ -fPIC -shared -o libIARMBus.so iarm_stubs.cpp  -I/usr/tr69hostif/src/hostif/parodusClient/pal -I/usr/tr69hostif/src/unittest/stubs -I/usr/tr69hostif/src/hostif/parodusClient/waldb -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/tr69hostif/src/hostif/include -I/usr/tr69hostif/src/hostif/profiles/DeviceInfo -I/usr/tr69hostif/src/hostif/parodusClient/pal -fpermissive
cp libIARMBus.so /usr/local/lib
cp libIBus.h /usr/local/include
cp libIARM.h /usr/local/include

cd $WORKDIR
autoreconf -i
autoupdate
./configure --prefix=${INSTALL_DIR} --enable-iarmbusSupport=yes
make remotedebugger_CFLAGS="-I/usr/include/cjson -I/usr/local/include/wdmp-c -I/usr/local/include/rbus -I/usr/local/include -I/usr/local/include/trower-base64" remotedebugger_LDFLAGS="-L/usr/local/lib -lrdkloggers -lcjson -lrfcapi -lrbus -lmsgpackc -lsecure_wrapper -lwebconfig_framework -lIARMBus -ltr181api  -L/usr/local/lib/x86_64-linux-gnu -ltrower-base64 -L/usr/lib/x86_64-linux-gnu"
make install

