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

//! ������OutOfMemoryError�쳣ʱ����JVM�׳����ڴ���Դ�ľ�����ʾ
GPrivate String_t gaszExhaustMsg[] =
{
    //! MaxHeapSize�ľ�����Ҫ����MaxHeapSize
    [GOOM_HEAP_SPACE] = "Java heap space",

    //! GCʱ�䳬��98%�������յ�heapȴ����2%(UseGCOverheadLimit\GCTimeLimit\GCHeapFreeLimit)
    [GOOM_OVERHEAD_LIMIT] = "GC overhead limit exceeded",

    //! �߳������ࡢ�߳�ջ̫��HeapSize̫�����ܹ����ڴ���Thread�Ŀռ�̫С
    [GOOM_NATIVE_THREAD] = "unable to create new native thread",

    //! ���ô��ռ�̫С(MaxPermSize)
    [GOOM_PERM_SPACE] = "PermGen space",

    //! ����������Ԫ�ظ���̫�࣬�����޷������ڴ�
    [GOOM_ARRAY_SIZE] = "Requested array size exceeds VM limit"
};

typedef jobject(JNICALL *Perf_Attach_t)(JNIEnv *, jobject, jstring, int, int);

GPrivate jvmtiEnv *gpJvmtiEnv = NULL;               //!< JVMTI��������
GPrivate jvmtiEnv gJvmtiEnv = NULL;                 //!< gJvmtiEnv = *gpJvmtiEnv;
GPrivate jvmtiCapabilities gCapabilities = { 0 };   //!< ����JVMTI�ӿڵĶ����ṩ���
GPrivate jvmtiEventCallbacks gCallbacks = { 0 };    //!< ����JVMTI�ӿڻص�����
GPrivate jrawMonitorID gMonitorID = NULL;           //!< �̱ܳ���������ͬ��
GPrivate Addr_t gPerfMemory = NULL;                 //!< ���ڴ��JVM���ܼ������Ĺ����ڴ����ĵ�ַ
GPrivate RBTreeP_t gpPerfTree = NULL;               //!< ͨ��pPerfMemory������������
GPrivate Perf_Attach_t gfnPerf_Attach = NULL;       //!< jvm��̬����Perf_Attach�ӿڵĵ�ַ

GPrivate jvmtiError gcmon_init_jvmtienv();
GPrivate void gcmon_clear_jvmtienv();

/*!
*@brief        ��ȡJVM�����PerfMemory��ַ
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

        //! ���gPerfMemoryΪ�գ���ʾJVM������-XX:-UsePerfDataѡ��
        if (NULL == gPerfMemory)
        {
            gcmon_clear_jvmtienv();
        }
    }
}

/*!
*@brief        ͨ��gPerfMemory����������
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
*@brief        ����ӿڣ���ȡ������ܼ������ĺ����ָ��
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
*@brief        ���̻߳����������ٽ���
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
*@brief        ���̻߳������뿪�ٽ���
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
*@brief        ��ȡOOM������
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
*@brief        VMInit�ӿڻص�����
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
*@brief        VMStart�ӿڻص�����
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
*@brief        Exception�ӿڻص�����
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
*@brief        ExceptionCatch�ӿڻص�����
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
*@brief        MethodEntry�ӿڻص�����
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
*@brief        NativeMethodBind�ӿڻص�����
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

    //! ��ȡPerf_Attach�ĵ�ַ
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

                //! ͨ��Perf_Attach�ӿڣ�Attach��JVM����ȡPerfMemory��ַ
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
*@brief        ResourceExhausted�ӿڻص�����
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
*@brief        GarbageCollectionStart�ӿڻص�����
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
        //! ͨ��gPerfMemory����������
        GASSERT(NULL == gpPerfTree);
        gcmon_init_perf_tree();

        //! ���ܼ�����������������˺������ʼ�����ܲ�����
        GASSERT(gpPerfTree != NULL);
        sample_init(gpPerfTree);
    }

    sample_doit("Start  GC ");
}

/*!
*@brief        FinishGarbageCollection�ӿڻص�����
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
*@brief        ��ȫ�ֱ���gCapabilities����
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
*@brief        ��ʼ��gcmon��gCapabilities����
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
*@brief        ��ȫ�ֱ���gCallbacks����
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
*@brief        ��ʼ��gcmon��gCallbacks����
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
*@brief        �������߹ر�gcmon��gCapabilities�¼�
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
*@brief        ����gcmon��jvmti����
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
*@brief        ��ʼ��JVM��jvmti����
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
*@brief        ���gcmon�����õ�jvmti����
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
*@brief        ����JVM��jvmti����
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
*@brief        jvmti��Agent_OnLoad�ӿ�ʵ�֣����ڳ�ʼ��gcmon����
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
*@brief        jvmti��Agent_OnUnload�ӿڵ�ʵ�֣���������gcmon��Դ
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
