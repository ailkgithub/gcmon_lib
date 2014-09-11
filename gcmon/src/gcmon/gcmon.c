
#include "share/share.h"

GPrivate jvmtiEnv *gpEnv = NULL;
GPrivate jvmtiCapabilities gCapabilities = { 0 };
GPrivate jvmtiEventCallbacks gCallbacks = { 0 };
GPrivate jrawMonitorID gMonitorID = NULL;
GPrivate jvmtiError gError = JVMTI_ERROR_NONE;


GPrivate void JNICALL StartGarbageCollection(jvmtiEnv *jvmti_env)
{
    printf("%s\n", __FUNCTION__);
}

GPrivate void JNICALL FinishGarbageCollection(jvmtiEnv *jvmti_env)
{
    printf("%s\n", __FUNCTION__);
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved)
{
    jint result = JNI_ERR;
    JavaVM pJvm = *jvm;
    jvmtiEnv pEnv = NULL;

    result = pJvm->GetEnv(jvm, (void **)&gpEnv, JVMTI_VERSION_1_1);

    if (result != JNI_OK || NULL == gpEnv)
    {
        printf("ERROR: Unable to access JVMTI!");
        return JNI_ERR;
    }

    pEnv = *gpEnv;

    memset(&gCapabilities, 0, sizeof(jvmtiCapabilities));
    gCapabilities.can_generate_garbage_collection_events = 1;

    gError = pEnv->AddCapabilities(gpEnv, &gCapabilities);

    if (gError != JVMTI_ERROR_NONE)
    {
        printf("ERROR: Can't get JVMTI capabilities");
        return JNI_ERR;
    }

    memset(&gCallbacks, 0, sizeof(jvmtiEventCallbacks));
    gCallbacks.GarbageCollectionStart = StartGarbageCollection;
    gCallbacks.GarbageCollectionFinish = FinishGarbageCollection;

    gError = pEnv->SetEventCallbacks(gpEnv, &gCallbacks, sizeof(jvmtiEventCallbacks));

    if (gError != JVMTI_ERROR_NONE)
    {
        printf("ERROR: Can't set jvmti callback!");
        return JNI_ERR;
    }

    pEnv->SetEventNotificationMode(gpEnv, JVMTI_ENABLE, JVMTI_EVENT_GARBAGE_COLLECTION_START, NULL);
    pEnv->SetEventNotificationMode(gpEnv, JVMTI_ENABLE, JVMTI_EVENT_GARBAGE_COLLECTION_FINISH, NULL);

    return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm)
{
}
