#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/ktime.h>

#define BILLION 1000000000

const int SIZE[3] = {1000, 10000, 100000};

struct my_node{
	struct list_head entry;
	int data;
};

struct timespec64 spclock[2];
unsigned long long total_time_save;
unsigned long long total_count_save;

void calclock(struct timespec64 *spclock, unsigned long long *total_time, unsigned long long *total_count)
{
	unsigned long long ns_begin, ns_end, time_delay;
	ns_begin = spclock[0].tv_sec * BILLION + spclock[0].tv_nsec;
	ns_end = spclock[1].tv_sec * BILLION + spclock[1].tv_nsec;
	time_delay = ns_end - ns_begin;

	__sync_fetch_and_add(total_time, time_delay);
	__sync_fetch_and_add(total_count, 1);
}

void struct_example(void)
{
	int N;
	for(N = 0; N < 3; N++){
		printk("< SIZE: %d >\n", SIZE[N]);

		struct list_head my_list;
		INIT_LIST_HEAD(&my_list);

		total_time_save = 0;
		total_count_save = 0;

		int i;
		for(i = 0; i < SIZE[N]; i++){
			struct my_node *new = kmalloc(sizeof(struct my_node), GFP_KERNEL);
			new->data = i;
			ktime_get_ts64(&spclock[0]);
			list_add(&new->entry, &my_list);
			ktime_get_ts64(&spclock[1]);
			calclock(spclock, &total_time_save, &total_count_save);
		}

		printk("[INSERT] time: %lluns count: %llu\n", total_time_save, total_count_save);

		total_time_save = 0;
		total_count_save = 0;
		
		struct my_node *current_node = NULL;

		int data_to_find;
		for(data_to_find = 0; data_to_find < SIZE[N]; data_to_find++){
			ktime_get_ts64(&spclock[0]);
			list_for_each_entry(current_node, &my_list, entry){
				if(current_node->data == data_to_find){
					//printk(">> found %d\n", data_to_find);
					break;
				}
			}
			ktime_get_ts64(&spclock[1]);
			calclock(spclock, &total_time_save, &total_count_save);
		}

		printk("[SEARCH] time: %lluns count: %llu\n", total_time_save, total_count_save);

		total_time_save = 0;
		total_count_save = 0;

		struct my_node *tmp;
		list_for_each_entry_safe(current_node, tmp, &my_list, entry){
			//printk("current node value: %d\n", current_node->data);
			ktime_get_ts64(&spclock[0]);
			list_del(&current_node->entry);
			ktime_get_ts64(&spclock[1]);
			kfree(current_node);
			calclock(spclock, &total_time_save, &total_count_save);	
		}

		printk("[DELETE] time: %lluns count: %llu\n", total_time_save, total_count_save);

		printk("\n");
	}
}

int __init hello_module_init(void)
{
	struct_example();
	printk("Hello Module!\n");
	return 0;
}

void __exit hello_module_cleanup(void)
{
	printk("Bye Module!\n");
}

module_init(hello_module_init);
module_exit(hello_module_cleanup);
MODULE_LICENSE("GPL");
