#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

bin_PROGRAMS = remotedebugger

remotedebuggerincludedir = $(includedir)/rrd
remotedebuggerinclude_HEADERS = rrdCommon.h rrdInterface.h \
        rrd_archive.h rrd_config.h rrd_logproc.h rrd_sysinfo.h rrd_upload.h

remotedebugger_SOURCES = rrdMain.c rrdEventProcess.c rrdJsonParser.c rrdRunCmdThread.c rrdCommandSanity.c rrdDynamic.c rrdExecuteScript.c rrdMsgPackDecoder.c rrdInterface.c uploadRRDLogs.c rrd_config.c rrd_sysinfo.c rrd_logproc.c rrd_archive.c rrd_upload.c

remotedebugger_CFLAGS = -I$(top_srcdir)/include/rrd -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/trower-base64/ -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rbus/  -I$(PKG_CONFIG_SYSROOT_DIR)${includedir} $(CJSON_CFLAGS)
AM_LDFLAGS="-lpthread -lrdkloggers -lmsgpackc -ltrower-base64 -lwebconfig_framework -lrbus -lsecure_wrapper "
remotedebugger_LDADD = -lfwutils -luploadstblogs
if IARMBUS_ENABLE
remotedebugger_SOURCES += rrdIarmEvents.c
remotedebugger_CFLAGS += -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rdk/iarmbus/ -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rdk/iarmmgrs/rdmmgr -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rdk/iarmmgrs-hal -DIARMBUS_SUPPORT
AM_LDFLAGS += "-lIARMBus -lrfcapi -ltr181api"
endif
remotedebugger_LDFLAGS = $(AM_LDFLAGS) $(CJSON_LDFLAGS) $(CJSON_LIBS) -luploadstblogs -fno-common
