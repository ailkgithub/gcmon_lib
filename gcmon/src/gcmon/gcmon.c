/*!**************************************************************
 *@file gcmon.c
 *@brief    Java Virtual Machine Tool Interfaces
 *@author   zhaohm3
 *@date 2014-9-15 17:53
 *@note
 * 
 ****************************************************************/

#include "share/share.h"
#include "perf/perf.h"


GPrivate jvmtiEnv *gpJvmtiEnv = NULL;                   
GPrivate jvmtiEnv gJvmtiEnv = NULL;
GPrivate jvmtiCapabilities gCapabilities = { 0 };
GPrivate jvmtiEventCallbacks gCallbacks = { 0 };
GPrivate jrawMonitorID gMonitorID = NULL;

typedef jobject(JNICALL *Perf_Attach_t)(JNIEnv *, jobject, jstring, int, int);
GPrivate Perf_Attach_t gfnPerf_Attach = NULL;
GPrivate void *gPerfMemory = NULL;

/*!
*@brief        获取JVM共享的PerfMemory地址
*@author       zhaohm3
*@param[in]    jvmti_env
*@param[in]    jni_env
*@retval
*@note
* 
*@since    2014-9-15 17:54
*@attention
* 
*/
GPrivate void GetPerfMemoryAddress(jvmtiEnv *jvmti_env, JNIEnv* jni_env)
{
    if (gfnPerf_Attach != NULL && NULL == gPerfMemory)
    {
        jobject buf = gfnPerf_Attach(jni_env, NULL, NULL, 0, 0);

        gPerfMemory = (*jni_env)->GetDirectBufferAddress(jni_env, buf);
    }
}

/*!
*@brief        多线程环境，进入临界区
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-15 17:55
*@attention
* 
*/
GPrivate jvmtiError RawMonitorEnter()
{
    return gJvmtiEnv->RawMonitorEnter(gpJvmtiEnv, gMonitorID);
}

/*!
*@brief        多线程环境，离开临界区
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-15 17:55
*@attention
* 
*/
GPrivate jvmtiError RawMonitorExit()
{
    return gJvmtiEnv->RawMonitorExit(gpJvmtiEnv, gMonitorID);
}

/*!
*@brief        VMInit接口回调函数
*@author       zhaohm3
*@param[in]    jvmti_env
*@param[in]    jni_env
*@param[in]    thread
*@retval
*@note
* 
*@since    2014-9-15 17:57
*@attention
* 
*/
GPrivate void JNICALL InitVM(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread)
{
    GCMON_PRINT_FUNC();
}

/*!
*@brief        VMStart接口回调函数
*@author       zhaohm3
*@param[in]    jvmti_env
*@param[in]    jni_env
*@retval
*@note
* 
*@since    2014-9-15 17:57
*@attention
* 
*/
GPrivate void JNICALL StartVM(jvmtiEnv *jvmti_env, JNIEnv* jni_env)
{
    GCMON_PRINT_FUNC();
}

/*!
*@brief        MethodEntry接口回调函数
*@author       zhaohm3
*@param[in]    jvmti_env
*@param[in]    jni_env
*@param[in]    thread
*@param[in]    method
*@retval
*@note
* 
*@since    2014-9-15 17:58
*@attention
* 
*/
GPrivate void JNICALL EntryMethod(jvmtiEnv *jvmti_env, JNIEnv* jni_env,
    jthread thread, jmethodID method)
{
    /*GCMON_PRINT_FUNC();*/
}

/*!
*@brief        GarbageCollectionStart接口回调函数
*@author       zhaohm3
*@param[in]    jvmti_env
*@retval
*@note
* 
*@since    2014-9-15 17:58
*@attention
* 
*/
GPrivate void JNICALL StartGarbageCollection(jvmtiEnv *jvmti_env)
{
    GCMON_PRINT_FUNC();
    perf_memory_analyze(gPerfMemory);
}

/*!
*@brief        FinishGarbageCollection接口回调函数
*@author       zhaohm3
*@param[in]    jvmti_env
*@retval
*@note
* 
*@since    2014-9-15 17:59
*@attention
* 
*/
GPrivate void JNICALL FinishGarbageCollection(jvmtiEnv *jvmti_env)
{
    GCMON_PRINT_FUNC();
    perf_memory_analyze(gPerfMemory);
}

/*!
*@brief        NativeMethodBind接口回调函数
*@author       zhaohm3
*@param[in]    jvmti_env
*@param[in]    jni_env
*@param[in]    thread
*@param[in]    method
*@param[in]    address
*@param[in]    new_address_ptr
*@retval
*@note
* 
*@since    2014-9-15 17:59
*@attention
* 
*/
GPrivate void JNICALL BindNativeMethod(jvmtiEnv *jvmti_env,
                                       JNIEnv* jni_env,
                                       jthread thread,
                                       jmethodID method,
                                       void* address,
                                       void** new_address_ptr)
{
    /*GCMON_PRINT_FUNC();*/

    if (NULL == gfnPerf_Attach)
    {
        String_t szName = NULL;
        String_t szSig = NULL;
        String_t szGsig = NULL;
        jvmtiError error = JVMTI_ERROR_NONE;

        GASSERT(gpJvmtiEnv == jvmti_env);
        error = RawMonitorEnter();

        if (JVMTI_ERROR_NONE == error && NULL == gfnPerf_Attach)
        {
            error = gJvmtiEnv->GetMethodName(gpJvmtiEnv, method, &szName, &szSig, &szGsig);

            if (JVMTI_ERROR_NONE == error
                && address != NULL
                && new_address_ptr != NULL
                && address == *new_address_ptr
                && szName != NULL
                && szSig != NULL
                && NULL == szGsig
                && 0 == strcmp(szName, "attach")
                && 0 == strcmp(szSig, "(Ljava/lang/String;II)Ljava/nio/ByteBuffer;"))
            {
                gfnPerf_Attach = (Perf_Attach_t)address;
                GetPerfMemoryAddress(jvmti_env, jni_env);
            }

            gcmon_debug_msg("%s --> method = 0x%p \t address = 0x%p \t new_address_ptr = 0x%p \t *new_address_ptr = 0x%p name = %s \t sig = %s \t gsig = %s \n",
                __FUNCTION__, method, address, new_address_ptr, *new_address_ptr, szName, szSig, szGsig);

            gJvmtiEnv->Deallocate(gpJvmtiEnv, szName);
            gJvmtiEnv->Deallocate(gpJvmtiEnv, szSig);
            gJvmtiEnv->Deallocate(gpJvmtiEnv, szGsig);
        }

        error = RawMonitorExit();
    }
}

/*!
*@brief        将全局变量gCapabilities清零
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-15 18:00
*@attention
* 
*/
GPrivate void ZeroCapabilities()
{
    memset(&gCapabilities, 0, sizeof(jvmtiCapabilities));
}

/*!
*@brief        初始化gcmon的gCapabilities环境
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-15 18:00
*@attention
* 
*/
GPrivate void InitCapabilities()
{
    ZeroCapabilities();
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
}

/*!
*@brief        将全局变量gCallbacks清零
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-15 18:00
*@attention
* 
*/
GPrivate void ZeroCallbacks()
{
    memset(&gCallbacks, 0, sizeof(jvmtiEventCallbacks));
}

/*!
*@brief        初始化gcmon的gCallbacks环境
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-15 18:01
*@attention
* 
*/
GPrivate void InitCallbacks()
{
    ZeroCallbacks();
    gCallbacks.VMInit = InitVM;
    gCallbacks.VMStart = StartVM;
    gCallbacks.MethodEntry = EntryMethod;
    gCallbacks.NativeMethodBind = BindNativeMethod;
    gCallbacks.GarbageCollectionStart = StartGarbageCollection;
    gCallbacks.GarbageCollectionFinish = FinishGarbageCollection;
}

/*!
*@brief        开启或者关闭gcmon的gCapabilities事件
*@author       zhaohm3
*@param[in]    mode
*@retval
*@note
* 
*@since    2014-9-15 18:02
*@attention
* 
*/
GPrivate void SetEventNotificationMode(jvmtiEventMode mode)
{
    gJvmtiEnv->SetEventNotificationMode(gpJvmtiEnv, mode, JVMTI_EVENT_VM_INIT, NULL);
    gJvmtiEnv->SetEventNotificationMode(gpJvmtiEnv, mode, JVMTI_EVENT_VM_START, NULL);
    gJvmtiEnv->SetEventNotificationMode(gpJvmtiEnv, mode, JVMTI_EVENT_EXCEPTION, NULL);
    gJvmtiEnv->SetEventNotificationMode(gpJvmtiEnv, mode, JVMTI_EVENT_EXCEPTION_CATCH, NULL);
    gJvmtiEnv->SetEventNotificationMode(gpJvmtiEnv, mode, JVMTI_EVENT_METHOD_ENTRY, NULL);
    gJvmtiEnv->SetEventNotificationMode(gpJvmtiEnv, mode, JVMTI_EVENT_METHOD_EXIT, NULL);
    gJvmtiEnv->SetEventNotificationMode(gpJvmtiEnv, mode, JVMTI_EVENT_NATIVE_METHOD_BIND, NULL);
    gJvmtiEnv->SetEventNotificationMode(gpJvmtiEnv, mode, JVMTI_EVENT_MONITOR_WAIT, NULL);
    gJvmtiEnv->SetEventNotificationMode(gpJvmtiEnv, mode, JVMTI_EVENT_MONITOR_WAITED, NULL);
    gJvmtiEnv->SetEventNotificationMode(gpJvmtiEnv, mode, JVMTI_EVENT_MONITOR_CONTENDED_ENTER, NULL);
    gJvmtiEnv->SetEventNotificationMode(gpJvmtiEnv, mode, JVMTI_EVENT_MONITOR_CONTENDED_ENTERED, NULL);
    gJvmtiEnv->SetEventNotificationMode(gpJvmtiEnv, mode, JVMTI_EVENT_GARBAGE_COLLECTION_START, NULL);
    gJvmtiEnv->SetEventNotificationMode(gpJvmtiEnv, mode, JVMTI_EVENT_GARBAGE_COLLECTION_FINISH, NULL);
}

/*!
*@brief        清除gcmon所设置的jvmti环境
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-15 18:05
*@attention
* 
*/
GPrivate void ClearJvmtiEnv()
{
    if (NULL == gpJvmtiEnv || NULL == gJvmtiEnv)
    {
        return;
    }

    if (gMonitorID != NULL)
    {
        gJvmtiEnv->DestroyRawMonitor(gpJvmtiEnv, gMonitorID);
    }

    ZeroCapabilities();
    gJvmtiEnv->AddCapabilities(gpJvmtiEnv, &gCapabilities);

    ZeroCallbacks();
    gJvmtiEnv->SetEventCallbacks(gpJvmtiEnv, &gCallbacks, sizeof(jvmtiEventCallbacks));

    SetEventNotificationMode(JVMTI_DISABLE);
}

/*!
*@brief        设置gcmon的jvmti环境
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-15 18:05
*@attention
* 
*/
GPrivate jvmtiError InitJvmtiEnv()
{
    jvmtiError error = JVMTI_ERROR_NONE;

    error = gJvmtiEnv->CreateRawMonitor(gpJvmtiEnv, "GC Monitor", &gMonitorID);
    GCMON_CHECK_ERROR(error, "ERROR: Can't Create Raw Monitor!", ERROR);

    InitCapabilities();
    error = gJvmtiEnv->AddCapabilities(gpJvmtiEnv, &gCapabilities);
    GCMON_CHECK_ERROR(error, "ERROR: Can't Set JVMTI Capabilities.", ERROR);

    InitCallbacks();
    error = gJvmtiEnv->SetEventCallbacks(gpJvmtiEnv, &gCallbacks, sizeof(jvmtiEventCallbacks));
    GCMON_CHECK_ERROR(error, "ERROR: Can't Set Callbacks.", ERROR);

    SetEventNotificationMode(JVMTI_ENABLE);

ERROR:
    return error;
}

/*!
*@brief        jvmti的Agent_OnLoad接口实现，用于初始化gcmon环境
*@author       zhaohm3
*@param[in]    jvm
*@param[in]    options
*@param[in]    reserved
*@retval
*@note
* 
*@since    2014-9-15 18:07
*@attention
* 
*/
JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved)
{
    jint ret = JNI_ERR;
    jvmtiError error = JVMTI_ERROR_NONE;

    GCMON_PRINT_FUNC();

    ret = (*jvm)->GetEnv(jvm, (void **)&gpJvmtiEnv, JVMTI_VERSION_1_1);
    if (ret != JNI_OK || NULL == gpJvmtiEnv)
    {
        printf("ERROR: Unable to access JVMTI!");
        return ret;
    }

    gJvmtiEnv = *gpJvmtiEnv;
    error = InitJvmtiEnv();
    GCMON_CHECK_ERROR(error, "ERROR: Can't Init JVMTI Env.", ERROR);

    gcmon_debug_fopen();
    return JNI_OK;

ERROR:
    ClearJvmtiEnv();;
    return JNI_ERR;
}

/*!
*@brief        jvmti的Agent_OnUnload接口的实现，拥有清理gcmon资源
*@author       zhaohm3
*@param[in]    vm
*@retval
*@note
* 
*@since    2014-9-15 18:08
*@attention
* 
*/
JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm)
{
    GCMON_PRINT_FUNC();
    ClearJvmtiEnv();
    gcmon_debug_fclose();
}
