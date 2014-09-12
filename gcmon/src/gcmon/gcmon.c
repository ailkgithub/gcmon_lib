
#include "share/share.h"

GPrivate jvmtiEnv *gpEnv = NULL;
GPrivate jvmtiEnv gEnv = NULL;
GPrivate jvmtiCapabilities gCapabilities = { 0 };
GPrivate jvmtiEventCallbacks gCallbacks = { 0 };
GPrivate jrawMonitorID gMonitorID = NULL;
GPrivate jvmtiError gError = JVMTI_ERROR_NONE;

typedef jobject(JNICALL *Perf_Attach_t)(JNIEnv *env, jobject unused, jstring user, int vmid, int mode);
GPrivate Perf_Attach_t gfnPerf_Attach = NULL;
GPrivate void *gPerfMemory = NULL;

GPrivate void GetPerfMemoryAddress(jvmtiEnv *jvmti_env, JNIEnv* jni_env)
{
    if (gfnPerf_Attach != NULL && NULL == gPerfMemory)
    {
        jobject buf = gfnPerf_Attach(jni_env, NULL, NULL, 0, 0);

        gPerfMemory = (*jni_env)->GetDirectBufferAddress(jni_env, buf);
    }
}

GPrivate void RawMonitorEnter()
{
    gError = (*gpEnv)->RawMonitorEnter(gpEnv, gMonitorID);
}

GPrivate void RawMonitorExit()
{
    gError = (*gpEnv)->RawMonitorExit(gpEnv, gMonitorID);
}


GPrivate void JNICALL StartGarbageCollection(jvmtiEnv *jvmti_env)
{
  printf("%s\n", __FUNCTION__);
}

GPrivate void JNICALL FinishGarbageCollection(jvmtiEnv *jvmti_env)
{
    printf("%s\n", __FUNCTION__);
}

GPrivate void JNICALL EntryMethod(jvmtiEnv *jvmti_env, JNIEnv* jni_env,
                                  jthread thread, jmethodID method)
{
    printf("%s\n", __FUNCTION__);
}

GPrivate void JNICALL InitVM(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread)
{
    printf("%s\n", __FUNCTION__);
}

GPrivate void JNICALL StartVM(jvmtiEnv *jvmti_env, JNIEnv* jni_env)
{
    printf("%s\n", __FUNCTION__);
}

GPrivate void JNICALL BindNativeMethod(jvmtiEnv *jvmti_env,
                                       JNIEnv* jni_env,
                                       jthread thread,
                                       jmethodID method,
                                       void* address,
                                       void** new_address_ptr)
{
    printf("%s\n", __FUNCTION__);

    if (NULL == gfnPerf_Attach)
    {
        String_t szName = NULL;
        String_t szSig = NULL;
        String_t szGsig = NULL;

        RawMonitorEnter();

        gEnv->GetMethodName(gpEnv, method, &szName, &szSig, &szGsig);

        if (address != NULL
            && new_address_ptr != NULL
            && address == *new_address_ptr
            && szName != NULL
            && szSig != NULL
            && NULL == szGsig
            && 0 == strcmp(szName, "attach")
            && 0 == strcmp(szSig, "(Ljava/lang/String;II)Ljava/nio/ByteBuffer;"))
        {
            gfnPerf_Attach = (Perf_Attach_t)address;
        }

        gcmon_debug_msg("%s --> method = 0x%p \t address = 0x%p \t new_address_ptr = 0x%p \t *new_address_ptr = 0x%p name = %s \t sig = %s \t gsig = %s \n",
            __FUNCTION__, method, address, new_address_ptr, *new_address_ptr, szName, szSig, szGsig);

        gEnv->Deallocate(gpEnv, szName);
        gEnv->Deallocate(gpEnv, szSig);
        gEnv->Deallocate(gpEnv, szGsig);
        RawMonitorExit();
    }

    GetPerfMemoryAddress(jvmti_env, jni_env);
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

    gEnv = pEnv = *gpEnv;

    memset(&gCapabilities, 0, sizeof(jvmtiCapabilities));
    gCapabilities.can_get_owned_monitor_info = 1;
    gCapabilities.can_get_current_contended_monitor = 1;
    gCapabilities.can_get_monitor_info = 1;
    gCapabilities.can_generate_method_entry_events = 1;
    gCapabilities.can_generate_method_exit_events = 1;
    gCapabilities.can_generate_monitor_events = 1;
    gCapabilities.can_generate_vm_object_alloc_events = 1;
    gCapabilities.can_generate_native_method_bind_events = 1;
    gCapabilities.can_generate_garbage_collection_events = 1;
    gCapabilities.can_generate_object_free_events = 1;
    gCapabilities.can_generate_exception_events = 1;
    gCapabilities.can_generate_all_class_hook_events = 1;

    gError = pEnv->AddCapabilities(gpEnv, &gCapabilities);

    if (gError != JVMTI_ERROR_NONE)
    {
        printf("ERROR: Can't get JVMTI capabilities");
        return JNI_ERR;
    }

    memset(&gCallbacks, 0, sizeof(jvmtiEventCallbacks));
    gCallbacks.GarbageCollectionStart = StartGarbageCollection;
    gCallbacks.GarbageCollectionFinish = FinishGarbageCollection;
    gCallbacks.MethodEntry = EntryMethod;
    gCallbacks.VMInit = InitVM;
    gCallbacks.VMStart = StartVM;
    gCallbacks.NativeMethodBind = BindNativeMethod;

    gError = pEnv->SetEventCallbacks(gpEnv, &gCallbacks, sizeof(jvmtiEventCallbacks));

    if (gError != JVMTI_ERROR_NONE)
    {
        printf("ERROR: Can't Set Callbacks!");
        return JNI_ERR;
    }

    gError = pEnv->CreateRawMonitor(gpEnv, "GC Monitor", &gMonitorID);

    if (gError != JVMTI_ERROR_NONE)
    {
        printf("ERROR: Can't Create Raw Monitor!");
        return JNI_ERR;
    }

    pEnv->SetEventNotificationMode(gpEnv, JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, NULL);
    pEnv->SetEventNotificationMode(gpEnv, JVMTI_ENABLE, JVMTI_EVENT_VM_START, NULL);
    pEnv->SetEventNotificationMode(gpEnv, JVMTI_ENABLE, JVMTI_EVENT_EXCEPTION, NULL);
    pEnv->SetEventNotificationMode(gpEnv, JVMTI_ENABLE, JVMTI_EVENT_EXCEPTION_CATCH, NULL);
    pEnv->SetEventNotificationMode(gpEnv, JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, NULL);
    pEnv->SetEventNotificationMode(gpEnv, JVMTI_ENABLE, JVMTI_EVENT_METHOD_EXIT, NULL);
    pEnv->SetEventNotificationMode(gpEnv, JVMTI_ENABLE, JVMTI_EVENT_NATIVE_METHOD_BIND, NULL);
    pEnv->SetEventNotificationMode(gpEnv, JVMTI_ENABLE, JVMTI_EVENT_MONITOR_WAIT, NULL);
    pEnv->SetEventNotificationMode(gpEnv, JVMTI_ENABLE, JVMTI_EVENT_MONITOR_WAITED, NULL);
    pEnv->SetEventNotificationMode(gpEnv, JVMTI_ENABLE, JVMTI_EVENT_MONITOR_CONTENDED_ENTER, NULL);
    pEnv->SetEventNotificationMode(gpEnv, JVMTI_ENABLE, JVMTI_EVENT_MONITOR_CONTENDED_ENTERED, NULL);
    pEnv->SetEventNotificationMode(gpEnv, JVMTI_ENABLE, JVMTI_EVENT_GARBAGE_COLLECTION_START, NULL);
    pEnv->SetEventNotificationMode(gpEnv, JVMTI_ENABLE, JVMTI_EVENT_GARBAGE_COLLECTION_FINISH, NULL);

    gcmon_debug_fopen();

    return JNI_OK;
}

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm)
{
    gEnv->DestroyRawMonitor(gpEnv, gMonitorID);
    gcmon_debug_fclose();
}
