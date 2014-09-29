/*!**************************************************************
 *@file gcmon.c
 *@brief    Java Virtual Machine Tool Interfaces
 *@author   zhaohm3
 *@date 2014-9-15 17:53
 *@note
 *
 ****************************************************************/

#include "gcmon/gcmon.h"
#include "os/os.h"
#include "sample/sample.h"
#include "ana/ana.h"
#include "args/args.h"
#include "file/file.h"
#include "perf/perf.h"

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

typedef jobject(JNICALL *Perf_Attach_t)(JNIEnv *, jobject, jstring, int, int);

GPrivate jvmtiEnv *gpJvmtiEnv = NULL;               //!< JVMTI开发环境
GPrivate jvmtiEnv gJvmtiEnv = NULL;                 //!< gJvmtiEnv = *gpJvmtiEnv;
GPrivate jvmtiCapabilities gCapabilities = { 0 };   //!< 控制JVMTI接口的对外提供情况
GPrivate jvmtiEventCallbacks gCallbacks = { 0 };    //!< 控制JVMTI接口回调函数
GPrivate jrawMonitorID gMonitorID = NULL;           //!< 管程变量，用于同步
GPrivate Addr_t gPerfMemory = NULL;                 //!< 用于存放JVM性能计数器的共享内存区的地址
GPrivate RBTreeP_t gpPerfTree = NULL;               //!< 通过pPerfMemory构建的性能树
GPrivate Perf_Attach_t gfnPerf_Attach = NULL;       //!< jvm动态库中Perf_Attach接口的地址

GPrivate jvmtiError gcmon_init_jvmtienv();
GPrivate void gcmon_clear_jvmtienv();

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
GPrivate void gcmon_get_perf_address(jvmtiEnv *jvmti_env, JNIEnv* jni_env)
{
    if (gfnPerf_Attach != NULL && NULL == gPerfMemory)
    {
        jobject buf = gfnPerf_Attach(jni_env, NULL, NULL, 0, 0);

        gPerfMemory = (Addr_t)(*jni_env)->GetDirectBufferAddress(jni_env, buf);

        //! 如果gPerfMemory为空，表示JVM设置了-XX:-UsePerfData选项
        if (NULL == gPerfMemory)
        {
            gcmon_clear_jvmtienv();
        }
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
GPrivate void gcmon_init_perf_tree()
{
    if (gPerfMemory != NULL && NULL == gpPerfTree)
    {
        /* perf_print_verbose(gPerfMemory);*/
        gpPerfTree = pdi_build_tree(gPerfMemory);
    }
}

/*!
*@brief        对外接口，获取存放性能计数器的红黑树指针
*@author       zhaohm3
*@retval
*@note
* 
*@since    2014-9-22 15:20
*@attention
* 
*/
GPublic RBTreeP_t gcmon_get_perf_tree()
{
    return gpPerfTree;
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
GPrivate jvmtiError gcmon_monitor_enter()
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
GPrivate jvmtiError gcmon_monitor_exit()
{
    return gJvmtiEnv->RawMonitorExit(gpJvmtiEnv, gMonitorID);
}

/*!
*@brief        获取OOM的类型
*@author       zhaohm3
*@param[in]    szDescription
*@retval
*@note
*
*@since    2014-9-23 15:56
*@attention
*
*/
GPrivate Int32_t gcmon_get_oom_type(const char* szDescription)
{
    Int32_t sdwSize = ARRAY_SIZE(gaszExhaustMsg);
    Int32_t i = 0;

    for (i = 0; i < sdwSize; i++)
    {
        if (0 == os_strcmp(gaszExhaustMsg[i], szDescription))
        {
            return i;
        }
    }

    return 0;
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
GPrivate void JNICALL JVMInitVM(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread)
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
GPrivate void JNICALL JVMStartVM(jvmtiEnv *jvmti_env, JNIEnv* jni_env)
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
GPrivate void JNICALL JVMException(jvmtiEnv *jvmti_env,
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
GPrivate void JNICALL JVMExceptionCatch(jvmtiEnv *jvmti_env,
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
GPrivate void JNICALL JVMMethodEntry(jvmtiEnv *jvmti_env, JNIEnv* jni_env,
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
GPrivate void JNICALL JVMNativeMethodBind(jvmtiEnv *jvmti_env,
    JNIEnv* jni_env,
    jthread thread,
    jmethodID method,
    void* address,
    void** new_address_ptr)
{
    GCMON_PRINT_FUNC();

    //! 获取Perf_Attach的地址
    if (NULL == gfnPerf_Attach)
    {
        String_t szName = NULL;
        String_t szSig = NULL;
        String_t szGsig = NULL;
        jvmtiError error = JVMTI_ERROR_NONE;

        GASSERT(gpJvmtiEnv == jvmti_env);
        error = gcmon_monitor_enter();

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
                && 0 == os_strcmp(szName, "attach")
                && 0 == os_strcmp(szSig, "(Ljava/lang/String;II)Ljava/nio/ByteBuffer;"))
            {
                gfnPerf_Attach = (Perf_Attach_t)address;

                //! 通过Perf_Attach接口，Attach到JVM，获取PerfMemory地址
                gcmon_get_perf_address(jvmti_env, jni_env);
            }

            /*
            gcmon_debug_msg("%s --> method = 0x%p "
                "\t address = 0x%p "
                "\t new_address_ptr = 0x%p "
                "\t *new_address_ptr = 0x%p "
                "\t name = %s "
                "\t sig = %s "
                "\t gsig = %s \n",
                __FUNCTION__,
                method,
                address,
                new_address_ptr,
                *new_address_ptr,
                szName,
                szSig,
                szGsig);
            */

            gJvmtiEnv->Deallocate(gpJvmtiEnv, szName);
            gJvmtiEnv->Deallocate(gpJvmtiEnv, szSig);
            gJvmtiEnv->Deallocate(gpJvmtiEnv, szGsig);
        }

        error = gcmon_monitor_exit();
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
GPrivate void JNICALL JVMResourceExhausted(jvmtiEnv *jvmti_env,
    JNIEnv* jni_env,
    jint flags,
    const void* reserved,
    const char* description)
{
    GCMON_PRINT_FUNC();
    ana_OOM(gcmon_get_perf_tree(), gcmon_get_oom_type(description));
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
GPrivate void JNICALL JVMGarbageCollectionStart(jvmtiEnv *jvmti_env)
{
    GCMON_PRINT_FUNC();

    if (NULL == gpPerfTree)
    {
        //! 通过gPerfMemory构建性能树
        GASSERT(NULL == gpPerfTree);
        gcmon_init_perf_tree();

        //! 性能计数器红黑树构建好了后，随机初始化性能采样项
        GASSERT(gpPerfTree != NULL);
        sample_init(gpPerfTree);
    }

    sample_doit("Start  GC ");
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
GPrivate void JNICALL JVMGarbageCollectionFinish(jvmtiEnv *jvmti_env)
{
    GCMON_PRINT_FUNC();
    sample_doit("Finish GC ");
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
GPrivate void JVMZeroCapabilities()
{
    os_memset(&gCapabilities, 0, sizeof(jvmtiCapabilities));
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
GPrivate void JVMInitCapabilities()
{
    JVMZeroCapabilities();
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
GPrivate void JVMZeroCallbacks()
{
    os_memset(&gCallbacks, 0, sizeof(jvmtiEventCallbacks));
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
GPrivate void JVMInitCallbacks()
{
    JVMZeroCallbacks();
    /*
    gCallbacks.VMInit = JVMInitVM;
    gCallbacks.VMStart = JVMStartVM;
    gCallbacks.Exception = JVMException;
    gCallbacks.ExceptionCatch = JVMExceptionCatch;
    gCallbacks.MethodEntry = JVMMethodEntry;
    */
    gCallbacks.NativeMethodBind = JVMNativeMethodBind;
    gCallbacks.ResourceExhausted = JVMResourceExhausted;
    gCallbacks.GarbageCollectionStart = JVMGarbageCollectionStart;
    gCallbacks.GarbageCollectionFinish = JVMGarbageCollectionFinish;
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
GPrivate void JVMSetEventNotificationMode(jvmtiEventMode mode)
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
*@brief        设置gcmon的jvmti环境
*@author       zhaohm3
*@retval
*@note
*
*@since    2014-9-15 18:05
*@attention
*
*/
GPrivate jvmtiError gcmon_init_jvmtienv()
{
    jvmtiError error = JVMTI_ERROR_NONE;

    JVMInitCapabilities();
    error = gJvmtiEnv->AddCapabilities(gpJvmtiEnv, &gCapabilities);
    GCMON_CHECK_ERROR(error, "ERROR: Can't Set JVMTI Capabilities.", ERROR);

    JVMInitCallbacks();
    error = gJvmtiEnv->SetEventCallbacks(gpJvmtiEnv, &gCallbacks, sizeof(jvmtiEventCallbacks));
    GCMON_CHECK_ERROR(error, "ERROR: Can't Set JVMTI Callbacks.", ERROR);

    JVMSetEventNotificationMode(JVMTI_ENABLE);

ERROR:
    return error;
}

/*!
*@brief        初始化JVM的jvmti环境
*@author       zhaohm3
*@retval
*@note
*
*@since    2014-9-15 18:05
*@attention
*
*/
GPrivate jvmtiError JVMInitJvmtiEnv()
{
    jvmtiError error = JVMTI_ERROR_NULL_POINTER;

    GCMON_CHECK_COND(gpJvmtiEnv != NULL && gJvmtiEnv != NULL, ERROR);
    error = gJvmtiEnv->CreateRawMonitor(gpJvmtiEnv, "GC Monitor", &gMonitorID);
    GCMON_CHECK_ERROR(error, "ERROR: Can't Create Raw Monitor.", ERROR);

    error = gcmon_init_jvmtienv();
    GCMON_CHECK_ERROR(error, "ERROR: Can't Init JVMTI Env.", ERROR);

ERROR:
    return error;
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
GPrivate void gcmon_clear_jvmtienv()
{
    JVMZeroCapabilities();
    gJvmtiEnv->AddCapabilities(gpJvmtiEnv, &gCapabilities);

    JVMZeroCallbacks();
    gJvmtiEnv->SetEventCallbacks(gpJvmtiEnv, &gCallbacks, sizeof(jvmtiEventCallbacks));

    JVMSetEventNotificationMode(JVMTI_DISABLE);
}

/*!
*@brief        清理JVM的jvmti环境
*@author       zhaohm3
*@retval
*@note
*
*@since    2014-9-15 18:05
*@attention
*
*/
GPrivate void JVMClearJvmtiEnv()
{
    if (NULL == gpJvmtiEnv || NULL == gJvmtiEnv)
    {
        return;
    }

    if (gMonitorID != NULL)
    {
        gJvmtiEnv->DestroyRawMonitor(gpJvmtiEnv, gMonitorID);
    }

    gcmon_clear_jvmtienv();
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
        os_printf("ERROR: Unable to access JVMTI!");
        return ret;
    }

    gJvmtiEnv = *gpJvmtiEnv;
    error = JVMInitJvmtiEnv();
    if (error != JVMTI_ERROR_NONE)
    {
        JVMClearJvmtiEnv();
        return JNI_ERR;
    }

    args_init_agentargs(options);
    file_open_all();

    return JNI_OK;
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
    JVMClearJvmtiEnv();
    rbtree_free(gpPerfTree);
    file_close_all();
    args_free_agentargs();
}
