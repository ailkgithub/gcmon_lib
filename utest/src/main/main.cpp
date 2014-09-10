
#include "utest.h"

static void urun()
{
    GPublic void utest_rbtree_run();
    UItem_t aItems[] =
    {
        { 0, TRUE, NULL, NULL, "ÍË³öµ¥Ôª²âÊÔ" },
        { 1, TRUE, utest_rbtree_run, NULL, "²âÊÔºìºÚÊ÷" }
    };

    utest_run(aItems, ARRAY_SIZE(aItems));
}

int main(int argc, char **argv, char **env)
{
    urun();
    return 0;
}
