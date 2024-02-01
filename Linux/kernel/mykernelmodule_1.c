
#include <linux/module.h>     /* Needed by all modules */
#include <linux/kernel.h>     /* Needed for KERN_INFO */
#include <linux/init.h>       /* Needed for the macros */

static char* whoisthis = "Mommy";
static int countpeople = 1;

module_param(countpeople, int, S_IRUGO);
module_param(whoisthis, charp, S_IRUGO);

static int __init m_init(void)
{
    pr_debug("parameters test module is loaded\n");
    for (int i = 0; i < countpeople; ++i) {
        pr_info("#%d Hello, %s\n", i, whoisthis);
    }
    return 0;
}

static void __exit m_exit(void)
{
    pr_debug("parameters test module is unloaded\n");
}

module_init(m_init);
module_exit(m_exit);

MODULE_LICENSE("GPL");

