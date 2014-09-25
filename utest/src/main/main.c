
#include "utest.h"
#include "file/file.h"

static void urun()
{
    GPublic void utest_rbtree_run();
    GPublic void utest_list_run();
    UItem_t aItems[] =
    {
        { 0, TRUE, NULL, NULL, "退出单元测试" },
        { 1, TRUE, utest_rbtree_run, NULL, "进入红黑树测试单元" },
        { 2, TRUE, utest_list_run, NULL, "进入链表测试单元" },
    };
    file_open_fdebug();
    utest_run(aItems, ARRAY_SIZE(aItems));
    file_close_fdebug();
}

int main(int argc, char **argv, char **env)
{
    urun();
    return 0;
}
