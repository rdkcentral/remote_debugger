CC = gcc
CFLAGS = -DIARMBUS_SUPPORT -DUSE_L2_SUPPORT -DPWRMGR_PLUGIN  -Iremote_debugger/src -I/usr/local/include -I/usr/local/include/rbus -I/usr/local/include/cjson -I/usr/local/include/trower-base64 -I../../../src -I./
LDFLAGS = -lcjson -lmsgpackc -lIARMBus -lwebconfig_framework -lrfcapi -ltr181api -lsecure_wrapper -lpthread  -ltrower-base64 -lrbus -lrdkloggers

# List all object files needed for linking.
# Add additional .c files below if you get linker errors for missing symbols.
OBJS = ../../../src/rrdIarmEvents.c \
       deepsleep_main.c \
       ../../../src/rrdRunCmdThread.c \
       ../../../src/rrdCommandSanity.c \
       ../../../src/rrdEventProcess.c \
       ../../../src/rrdInterface.c \
       ../../../src/rrdJsonParser.c \
       ../../../src/rrdDynamic.c \
       ../../../src/rrdExecuteScript.c \
       ../../../src/rrdMsgPackDecoder.c
TARGET = test_pwr_event_handler

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o
