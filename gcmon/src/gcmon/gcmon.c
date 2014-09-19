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
#include "status/status.h"

GPrivate jvmtiEnv *gpJvmtiEnv = NULL;               //!< JVMTI开发环境
GPrivate jvmtiEnv gJvmtiEnv = NULL;                 //!< gJvmtiEnv = *gpJvmtiEnv;
GPrivate jvmtiCapabilities gCapabilities = { 0 };   //!< 控制JVMTI接口的对外提供情况
GPrivate jvmtiEventCallbacks gCallbacks = { 0 };    //!< 控制JVMTI接口回调函数
GPrivate jrawMonitorID gMonitorID = NULL;           //!< 管程变量，用于同步

typedef jobject(JNICALL *Perf_Attach_t)(JNIEnv *, jobject, jstring, int, int);
GPrivate Perf_Attach_t gfnPerf_Attach = NULL;       //!< jvm动态库中Perf_Attach接口的地址
GPrivate void *gPerfMemory = NULL;                  //!< 用于存放JVM性能计数器的共享内存区的地址
GPrivate RBTreeP_t gpPerfTree = NULL;               //!< 通过pPerfMemory构建的性能树

//! 用于处理java.lang.OutOfMemoryError异常
#define GOOM_HEAP_SPACE 0                           //!< Java heap space
#define GOOM_OVERHEAD_LIMIT 1                       //!< GC overhead limit exceeded
#define GOOM_NATIVE_THREAD 2                        //!< unable to create new native thread
#define GOOM_PERM_SPACE 3                           //!< PermGen space
#define GOOM_ARRAY_SIZE 4                           //!< Requested array size exceeds VM limit

//! 当发生OutOfMemoryError异常时，由JVM抛出的内存资源耗尽的提示
GPrivate String_t gaszExhaustMsg[] =
{
    //! MaxHeapSize耗尽，需要增加MaxHeapSize
    [GOOM_HEAP_SPACE] = "Java heap space",

    //! GC时间超过98%，而回收的heap却不足2%(UseGCOverheadLimit\GCTimeLimit\GCHeapFreeLimit)
    [GOOM_OVERHEAD_LIMIT] = "GC overhead limit exceeded",

    //! 线程数过多、线程栈太大、HeapSize太大导致能够用于创建Thread的空间太小
    [GOOM_NATIVE_THREAD] = "unable to create new native thread",

    //! 永久代空间太小(MaxPermSize)
    [GOOM_PERM_SPACE] = "PermGen space",

    //! 创建的数组元素个数太多，导致无法分配内存
    [GOOM_ARRAY_SIZE] = "Requested array size exceeds VM limit"
};

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
*@brief        通过gPerfMemory构建性能树
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-18 16:49
*@attention
* 
*/
GPrivate void BuildPerfMemoryTree()
{
    if (gPerfMemory != NULL && NULL == gpPerfTree)
    {
        gpPerfTree = pdi_build_tree(gPerfMemory);
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
*@brief        Exception接口回调函数
*@author       zhaohm3
*@param[in]    jvmti_env
*@param[in]    jni_env
*@param[in]    thread
*@param[in]    method
*@param[in]    location
*@param[in]    exception
*@param[in]    catch_method
*@param[in]    catch_location
*@retval
*@note
* 
*@since    2014-9-16 14:27
*@attention
* 
*/
GPrivate void JNICALL ExceptionEvent(jvmtiEnv *jvmti_env,
    JNIEnv* jni_env,
    jthread thread,
    jmethodID method,
    jlocation location,
    jobject exception,
    jmethodID catch_method,
    jlocation catch_location)
{
    GCMON_PRINT_FUNC();
}

/*!
*@brief        ExceptionCatch接口回调函数
*@author       zhaohm3
*@param[in]    jvmti_env
*@param[in]    jni_env
*@param[in]    thread
*@param[in]    method
*@param[in]    location
*@param[in]    exception
*@retval
*@note
* 
*@since    2014-9-16 14:27
*@attention
* 
*/
GPrivate void JNICALL CatchExceptionEvent(jvmtiEnv *jvmti_env,
    JNIEnv* jni_env,
    jthread thread,
    jmethodID method,
    jlocation location,
    jobject exception)
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
    GCMON_PRINT_FUNC();
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

    //! 获取Perf_Attach的地址
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

                //! 通过Perf_Attach接口，Attach到JVM，获取PerfMemory地址
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
*@brief        ResourceExhausted接口回调函数
*@author       zhaohm3
*@param[in]    jvmti_env
*@param[in]    jni_env
*@param[in]    flags
*@param[in]    reserved
*@param[in]    description
*@retval
*@note
* 
*@since    2014-9-16 14:41
*@attention
* 
*/
GPrivate void JNICALL ResourceExhaustedEvent(jvmtiEnv *jvmti_env,
    JNIEnv* jni_env,
    jint flags,
    const void* reserved,
    const char* description)
{
    GCMON_PRINT_FUNC();
    gcmon_debug_msg("%s --> Exception MSG : %s\n", __FUNCTION__, description);
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
    /*GCMON_PRINT_FUNC();
    perf_memory_analyze(gPerfMemory);*/

    if (NULL == gpPerfTree)
    {
        //! 通过gPerfMemory构建性能树
        GASSERT(NULL == gpPerfTree);
        BuildPerfMemoryTree();

        GASSERT(gpPerfTree != NULL);
        status_init(gpPerfTree);
    }

    status_sample("Start  GC ");
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
    /*GCMON_PRINT_FUNC();*/
    perf_memory_analyze(gPerfMemory);
    status_sample("Finish GC ");
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
    gCapabilities.can_generate_resource_exhaustion_heap_events = 1;
    gCapabilities.can_generate_resource_exhaustion_threads_events = 1;
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
    /*!
    gCallbacks.VMInit = InitVM;
    gCallbacks.VMStart = StartVM;
    gCallbacks.Exception = ExceptionEvent;
    gCallbacks.ExceptionCatch = CatchExceptionEvent;
    gCallbacks.MethodEntry = EntryMethod;
    */
    gCallbacks.NativeMethodBind = BindNativeMethod;
    gCallbacks.ResourceExhausted = ResourceExhaustedEvent;
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
    gJvmtiEnv->SetEventNotificationMode(gpJvmtiEnv, mode, JVMTI_EVENT_RESOURCE_EXHAUSTED, NULL);
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
    ClearJvmtiEnv();
    return JNI_ERR;
}

/*!
*@brief        jvmti的Agent_OnUnload接口的实现，用于清理gcmon资源
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
    rbtree_free(gpPerfTree);
    gcmon_debug_fclose();
}
