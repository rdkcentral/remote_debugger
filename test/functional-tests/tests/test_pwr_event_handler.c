//#include "PowerController.h"   // or the real header
//#include "rrdIarmEvents.h"     // if needed
#include "power_controller.h"
#include "rrdInterface.h"
#include "rbus.h"
//#include "rrdMain.h"
//int msqid = 0;
#define SUBDOC_NAME_SZ 64
typedef uint32_t (*getVersion)(char *);
typedef int (*setVersion)(char *, uint32_t);
//devicePropertiesData devPropData;
//typedef int PowerController_PowerState_t;
//
void * RRDEventThreadFunc(void *arg);
extern rbusHandle_t rrdRbusHandle;
//rdk_logger_init("/etc/debug.ini");
//    printf("Logger initialized\n");
//    RDK_LOG(RDK_LOG_INFO, "LOG.RDK.REMOTEDEBUGGER", "Test log entry before handler\n");
typedef struct _blobRegInfo
{
    int version;
    char subdoc_name[SUBDOC_NAME_SZ];
} blobRegInfo, *PblobRegInfo;

int main()
{
  rdk_logger_init("/etc/debug.ini");
    //printf("Logger initialized\n");
  //  RDK_LOG(RDK_LOG_INFO, "LOG.RDK.REMOTEDEBUGGER", "Test log entry before handler\n");
shadowMain1();
        pthread_t RRDTR69ThreadID;

//if ((msqid = msgget(key, IPC_CREAT | 0666 )) < 0)
//    {
//        RDK_LOG(RDK_LOG_ERROR,LOG_REMDEBUG,"[%s:%d]:Message Queue ID Creation failed, msqid=%d!!!\n",__FUNCTION__,__LINE__,msqid);
//        exit(1);
//    }
//    RDK_LOG(RDK_LOG_DEBUG,LOG_REMDEBUG,"[%s:%d]:SUCCESS: Message Queue ID Creation Done, msqid=%d...\n",__FUNCTION__,__LINE__,msqid);

    //rdk_logger_init(DEBUG_INI_FILE);
   RRD_subscribe();
    /* Create Thread for listening TR69 events */
//pthread_create (&RRDTR69ThreadID, NULL, RRDEventThreadFunc, NULL);
//pthread_join(RRDTR69ThreadID, NULL);
printf("Logger initialized\n");
    PowerController_PowerState_t state1 = POWER_STATE_STANDBY_DEEP_SLEEP;/* assign a valid enum value */;
    PowerController_PowerState_t state2 = POWER_STATE_STANDBY_DEEP_SLEEP;/* assign another valid value */;
    void* userdata = NULL; // or a real pointer if needed
    _pwrManagerEventHandler(state1, state2, userdata);


    rbusError_t rc = rbus_open(&rrdRbusHandle, "TestComponent");
    if (rc != RBUS_ERROR_SUCCESS) {
        printf("Failed to open RBUS handle!\n");
        return 1;
    }

// pthread_create (&RRDTR69ThreadID, NULL, RRDEventThreadFunc, NULL);
//    pthread_join(RRDTR69ThreadID, NULL);
 //    rbus_close(rrdRbusHandle);
    state2 = POWER_STATE_ON;
//pthread_create (&RRDTR69ThreadID, NULL, RRDEventThreadFunc, NULL);
//pthread_join(RRDTR69ThreadID, NULL);
    _pwrManagerEventHandler(state1, state2, userdata);
 // pthread_create (&RRDTR69ThreadID, NULL, RRDEventThreadFunc, NULL);
 // pthread_join(RRDTR69ThreadID, NULL);
RRDEventThreadFunc(NULL);
    return 0;
}

void register_sub_docs(blobRegInfo *bInfo, int numOfSubdocs, getVersion getv, setVersion setv)
{


}

