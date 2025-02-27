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

bin_PROGRAMS = remotedebugger

remotedebuggerincludedir = $(includedir)/rrd
remotedebuggerinclude_HEADERS = rrdCommon.h rrdInterface.h

remotedebugger_SOURCES = rrdMain.c rrdEventProcess.c rrdJsonParser.c rdmdownload.c rrdRunCmdThread.c rrdCommandSanity.c rrdExecuteScript.c rrdMsgPackDecoder.c rrdInterface.c
remotedebugger_CFLAGS = -I$(top_srcdir)/include/rrd -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/trower-base64/ -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rbus/ $(CJSON_CFLAGS)
AM_LDFLAGS="-lpthread -lrdkloggers -lmsgpackc -ltrower-base64 -lwebconfig_framework -lrbus -lsecure_wrapper"
if IARMBUS_ENABLE
remotedebugger_SOURCES += rrdIarmEvents.c rrdDeepSleep.c
remotedebugger_CFLAGS += -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rdk/iarmbus/ -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rdk/iarmmgrs/rdmmgr -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rdk/iarmmgrs-hal -DIARMBUS_SUPPORT
AM_LDFLAGS += "-lIARMBus -lrfcapi -ltr181api"
endif
remotedebugger_LDFLAGS = $(AM_LDFLAGS) $(CJSON_LDFLAGS) $(CJSON_LIBS) -fno-common
