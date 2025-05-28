CC = gcc
CFLAGS = -DUSE_L2_SUPPORT -DIARMBUS_SUPPORT -DPWRMGR_PLUGIN -Iremote_debugger/src -I/usr/local/include -I/usr/local/include/rbus -I/usr/local/include/cjson -I/usr/local/include/trower-base64
LDFLAGS = -lcjson -lmsgpackc -lIARMBus -lwebconfig_framework -lrfcapi -ltr181api -lsecure_wrapper -lpthread  -ltrower-base64 -lrbus -lrdkloggers

# List all object files needed for linking.
# Add additional .c files below if you get linker errors for missing symbols.
OBJS = remote_debugger/src/rrdMain.c \
       remote_debugger/src/rrdIarmEvents.c \
       test_pwr_event_handler.c \
       remote_debugger/src/rrdRunCmdThread.c \
       remote_debugger/src/rrdCommandSanity.c \
       remote_debugger/src/rrdEventProcess.c \
       remote_debugger/src/rrdInterface.c \
       remote_debugger/src/rrdJsonParser.c \
       remote_debugger/src/rrdDynamic.c \
       remote_debugger/src/rrdExecuteScript.c \
       remote_debugger/src/rrdMsgPackDecoder.c
TARGET = test_pwr_event_handler

all: $(TARGET)

$(TARGET): $(OBJS)
        $(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

clean:
        rm -f $(TARGET) *.o
