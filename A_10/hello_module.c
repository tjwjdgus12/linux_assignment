#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/delay.h>

int counter = 0;

int test_thread(void *_arg)
{
	int* arg = (int*)_arg;
	__sync_fetch_and_add(&counter, 1);
	printk("arg: %d, counter : %d\n", *arg, counter);
	kfree(arg);
	return 0;
}

void thread_create(void)
{
	int i;
	for(i = 0; i < 4; i++){
		int* arg = (int*)kmalloc(sizeof(int), GFP_KERNEL);
		*arg = i;
		kthread_run(&test_thread, (void*)arg, "test_thread");
	}
}

int __init hello_module_init(void)
{
	thread_create();
	printk(KERN_EMERG "Hello Module! \n");
	return 0;
}

void __exit hello_module_cleanup(void)
{
	printk("Bye Module!\n");
}

module_init(hello_module_init);
module_exit(hello_module_cleanup);

MODULE_LICENSE("GPL");
