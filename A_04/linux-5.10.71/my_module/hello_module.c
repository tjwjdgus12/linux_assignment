#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/ktime.h>

#define BILLION 1000000000

const int SIZE[3] = {1000,10000,100000};

struct my_node{
	struct rb_node node;
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

void RB_example(void)
{
	int N;
	for(N = 0; N < 3; N++){
		printk("< SIZE: %d >\n", SIZE[N]);

		struct rb_root root = RB_ROOT;

		total_time_save = 0;
		total_count_save = 0;

		int data_to_insert;
		for(data_to_insert = 0; data_to_insert < SIZE[N]; data_to_insert++){
			ktime_get_ts64(&spclock[0]);
			struct rb_node **new = &(root.rb_node), *parent = NULL;
			
			while(*new){
				struct my_node *this = container_of(*new, struct my_node, node);
				
				parent = *new;
				if(data_to_insert < this->data){
					new = &((*new)->rb_left);
				}
				else if(data_to_insert > this->data){
					new = &((*new)->rb_right);
				}
				else{
					return;
				}
			}

			struct my_node *new_node = kmalloc(sizeof(struct my_node), GFP_KERNEL);
			new_node->data = data_to_insert;
			rb_link_node(&new_node->node, parent, new);
			rb_insert_color(&new_node->node, &root);

			ktime_get_ts64(&spclock[1]);
			calclock(spclock, &total_time_save, &total_count_save);
		}

		printk("[INSERT] time: %lluns count: %llu\n", total_time_save, total_count_save);

		total_time_save = 0;
		total_count_save = 0;
		
		int data_to_find;
		for(data_to_find = 0; data_to_find < SIZE[N]; data_to_find++){
			
			ktime_get_ts64(&spclock[0]);
			
			struct rb_node *cur = root.rb_node;
			while(cur){
				struct my_node *now_node = container_of(cur, struct my_node, node);
				
				if(data_to_find < now_node->data){
					cur = cur->rb_left;
				}
				else if(data_to_find > now_node->data){
					cur = cur->rb_right;
				}
				else{
					//printk("*** found %d ***\n", data_to_find);
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

		int i;
		for(i = 0; i < SIZE[N]; i++){
			ktime_get_ts64(&spclock[0]);

			rb_erase(root.rb_node, &root);

			ktime_get_ts64(&spclock[1]);
			calclock(spclock, &total_time_save, &total_count_save);
		}

		printk("[DELETE] time: %lluns count: %llu\n", total_time_save, total_count_save);

		printk("\n");
	}
}

int __init hello_module_init(void)
{
	RB_example();
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
