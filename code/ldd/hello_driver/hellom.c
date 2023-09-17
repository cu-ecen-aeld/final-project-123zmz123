#include <linux/module.h>

static int __init helloworld_init(void){
    pr_info("Hello BBB!\n");
    return 0;
}


static void __exit helloworld_exit(void){
    pr_info("Bye Bye BBB!\n");
}
module_init(helloworld_init);
module_exit(helloworld_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZhangMingzhe");
MODULE_DESCRIPTION("A simple ko to prove it can be run on beagle bone");
MODULE_INFO(board,"Beagle Bone Black");
