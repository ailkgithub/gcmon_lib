
#include "share/share.h"

GPrivate jvmtiEnv *g_pJvmtiEnv = NULL;
GPrivate jvmtiCapabilities g_Capabilities = { 0 };
GPrivate jvmtiEventCallbacks g_Callbacks = { 0 };
GPrivate jrawMonitorID g_MonitorID = NULL;
GPrivate jvmtiError g_Error = JVMTI_ERROR_NONE;


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

    result = jvm->GetEnv((void **)&g_pJvmtiEnv, JVMTI_VERSION_1_0);

    if (result != JNI_OK || g_pJvmtiEnv == NULL)
    {
        printf("ERROR: Unable to access JVMTI!");
        return JNI_ERR;
    }

    memset(&g_Capabilities, 0, sizeof(jvmtiCapabilities));
    g_Capabilities.can_generate_garbage_collection_events = 1;

    g_Error = g_pJvmtiEnv->AddCapabilities(&g_Capabilities);

    if (g_Error != JVMTI_ERROR_NONE)
    {
        printf("ERROR: Can't get JVMTI capabilities");
        return JNI_ERR;
    }

    memset(&g_Callbacks, 0, sizeof(jvmtiEventCallbacks));
    g_Callbacks.GarbageCollectionStart = StartGarbageCollection;
    g_Callbacks.GarbageCollectionFinish = FinishGarbageCollection;

    g_Error = g_pJvmtiEnv->SetEventCallbacks(&g_Callbacks, sizeof(jvmtiEventCallbacks));

    if (g_Error != JVMTI_ERROR_NONE)
    {
        printf("ERROR: Can't set jvmti callback!");
        return JNI_ERR;
    }

    g_pJvmtiEnv->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_GARBAGE_COLLECTION_START, NULL);
    g_pJvmtiEnv->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_GARBAGE_COLLECTION_FINISH, NULL);

    return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm)
{
}

