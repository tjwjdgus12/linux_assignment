#include <linux/kernel.h>

asmlinkage long __x64_sys_mycall(void) {
	printk("20172864 Seo Jeong Hyeon\n");

	return 0;
}
