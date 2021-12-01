#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/delay.h>

#define BILLION 1000000000

int counter = 0;
int insert_done = 0;
int search_done = 0;
int remove_done = 0;
unsigned long long result_time = 0;
spinlock_t counter_lock;
struct rw_semaphore counter_rwse;
struct mutex my_mutex;

struct my_node{
	struct list_head entry;
	int data;
};

struct list_head my_list;

void calclock(struct timespec64 *spclock, unsigned long long *total_time, unsigned long long *total_count)
{
	unsigned long long ns_begin, ns_end, time_delay;
	ns_begin = spclock[0].tv_sec * BILLION + spclock[0].tv_nsec;
	ns_end = spclock[1].tv_sec * BILLION + spclock[1].tv_nsec;
	time_delay = ns_end - ns_begin;

	__sync_fetch_and_add(total_time, time_delay);
	__sync_fetch_and_add(total_count, 1);
}

int spinlock_op(void *_arg)
{
	struct timespec64 spclock[2];
	unsigned long long total_time_save;
	unsigned long long total_count_save;
	
	int i;
	struct my_node *new;
	struct my_node *current_node;
	
	total_time_save = 0;
	total_count_save = 0;

	for(i = 0; i < 25000; i++){
		spin_lock(&counter_lock);
		new = kmalloc(sizeof(struct my_node), GFP_KERNEL);
		new->data = __sync_fetch_and_add(&counter, 1);
		ktime_get_ts64(&spclock[0]);
		list_add(&new->entry, &my_list);
		ktime_get_ts64(&spclock[1]);
		spin_unlock(&counter_lock);
		calclock(spclock, &total_time_save, &total_count_save);
	}
	__sync_fetch_and_add(&insert_done, 1);
	__sync_fetch_and_add(&result_time, total_time_save);
		
	if(insert_done == 4){
		printk("Spinlock linked list insert time: %lluns\n", result_time);
		counter = 0;
		result_time = 0;
	}
	
	while(insert_done < 4){
		msleep(100);
	}
	
	///////////////////////////////////////////////////////////
	
	total_time_save = 0;
	total_count_save = 0;
	
	current_node = NULL;

	for(i = 0; i < 25000; i++){
		spin_lock(&counter_lock);
		ktime_get_ts64(&spclock[0]);
		list_for_each_entry(current_node, &my_list, entry){
			if(current_node->data == counter){
				break;
			}
		}
		ktime_get_ts64(&spclock[1]);
		spin_unlock(&counter_lock);
		calclock(spclock, &total_time_save, &total_count_save);
		__sync_fetch_and_add(&counter, 1);
	}
	__sync_fetch_and_add(&search_done, 1);
	__sync_fetch_and_add(&result_time, total_time_save);

	if(search_done == 4){
		printk("Spinlock linked list search time: %lluns\n", result_time);
		counter = 0;
		result_time = 0;
	}
	
	while(search_done < 4){
		msleep(100);
	}
	
	//////////////////////////////////////////////////////////////
	
	total_time_save = 0;
	total_count_save = 0;

	for(i = 0; i < 25000; i++){
		//printk("current node value: %d\n", current_node->data);
		spin_lock(&counter_lock);
		current_node = list_first_entry(&my_list, struct my_node, entry);
		ktime_get_ts64(&spclock[0]);
		list_del(&current_node->entry);
		ktime_get_ts64(&spclock[1]);
		kfree(current_node);
		spin_unlock(&counter_lock);
		calclock(spclock, &total_time_save, &total_count_save);	
	}
	__sync_fetch_and_add(&remove_done, 1);
	__sync_fetch_and_add(&result_time, total_time_save);

	if(remove_done == 4){
		printk("Spinlock linked list delete time: %lluns\n", result_time);
		counter = 0;
		result_time = 0;
	}
	
	do_exit(0);
}

int semaphore_op(void *_arg)
{
	struct timespec64 spclock[2];
	unsigned long long total_time_save;
	unsigned long long total_count_save;
	
	int i;
	struct my_node *new;
	struct my_node *current_node;
	
	total_time_save = 0;
	total_count_save = 0;

	for(i = 0; i < 25000; i++){
		down_write(&counter_rwse);
		new = kmalloc(sizeof(struct my_node), GFP_KERNEL);
		new->data = __sync_fetch_and_add(&counter, 1);
		ktime_get_ts64(&spclock[0]);
		list_add(&new->entry, &my_list);
		ktime_get_ts64(&spclock[1]);
		up_write(&counter_rwse);
		calclock(spclock, &total_time_save, &total_count_save);
	}
	__sync_fetch_and_add(&insert_done, 1);
	__sync_fetch_and_add(&result_time, total_time_save);
		
	if(insert_done == 4){
		printk("Semaphore linked list insert time: %lluns\n", result_time);
		counter = 0;
		result_time = 0;
	}
	
	while(insert_done < 4){
		msleep(100);
	}
	
	///////////////////////////////////////////////////////////
	
	total_time_save = 0;
	total_count_save = 0;
	
	current_node = NULL;

	for(i = 0; i < 25000; i++){
		down_write(&counter_rwse);
		ktime_get_ts64(&spclock[0]);
		list_for_each_entry(current_node, &my_list, entry){
			if(current_node->data == counter){
				break;
			}
		}
		ktime_get_ts64(&spclock[1]);
		up_write(&counter_rwse);
		calclock(spclock, &total_time_save, &total_count_save);
		__sync_fetch_and_add(&counter, 1);
	}
	__sync_fetch_and_add(&search_done, 1);
	__sync_fetch_and_add(&result_time, total_time_save);

	if(search_done == 4){
		printk("Semaphore linked list search time: %lluns\n", result_time);
		counter = 0;
		result_time = 0;
	}
	
	while(search_done < 4){
		msleep(100);
	}
	
	//////////////////////////////////////////////////////////////
	
	total_time_save = 0;
	total_count_save = 0;

	for(i = 0; i < 25000; i++){
		//printk("current node value: %d\n", current_node->data);
		down_write(&counter_rwse);
		current_node = list_first_entry(&my_list, struct my_node, entry);
		ktime_get_ts64(&spclock[0]);
		list_del(&current_node->entry);
		ktime_get_ts64(&spclock[1]);
		kfree(current_node);
		up_write(&counter_rwse);
		calclock(spclock, &total_time_save, &total_count_save);	
	}
	__sync_fetch_and_add(&remove_done, 1);
	__sync_fetch_and_add(&result_time, total_time_save);

	if(remove_done == 4){
		printk("Semaphore linked list delete time: %lluns\n", result_time);
		counter = 0;
		result_time = 0;
	}
	
	do_exit(0);
}

int mutex_op(void *_arg)
{
	struct timespec64 spclock[2];
	unsigned long long total_time_save;
	unsigned long long total_count_save;
	
	int i;
	struct my_node *new;
	struct my_node *current_node;
	
	total_time_save = 0;
	total_count_save = 0;

	for(i = 0; i < 25000; i++){
		mutex_lock(&my_mutex);
		new = kmalloc(sizeof(struct my_node), GFP_KERNEL);
		new->data = __sync_fetch_and_add(&counter, 1);
		ktime_get_ts64(&spclock[0]);
		list_add(&new->entry, &my_list);
		ktime_get_ts64(&spclock[1]);
		mutex_unlock(&my_mutex);
		calclock(spclock, &total_time_save, &total_count_save);
	}
	__sync_fetch_and_add(&insert_done, 1);
	__sync_fetch_and_add(&result_time, total_time_save);
		
	if(insert_done == 4){
		printk("Mutex linked list insert time: %lluns\n", result_time);
		counter = 0;
		result_time = 0;
	}
	
	while(insert_done < 4){
		msleep(100);
	}
	
	///////////////////////////////////////////////////////////
	
	total_time_save = 0;
	total_count_save = 0;
	
	current_node = NULL;

	for(i = 0; i < 25000; i++){
		mutex_lock(&my_mutex);
		ktime_get_ts64(&spclock[0]);
		list_for_each_entry(current_node, &my_list, entry){
			if(current_node->data == counter){
				break;
			}
		}
		ktime_get_ts64(&spclock[1]);
		mutex_unlock(&my_mutex);
		calclock(spclock, &total_time_save, &total_count_save);
		__sync_fetch_and_add(&counter, 1);
	}
	__sync_fetch_and_add(&search_done, 1);
	__sync_fetch_and_add(&result_time, total_time_save);

	if(search_done == 4){
		printk("Mutex linked list search time: %lluns\n", result_time);
		counter = 0;
		result_time = 0;
	}
	
	while(search_done < 4){
		msleep(100);
	}
	
	//////////////////////////////////////////////////////////////
	
	total_time_save = 0;
	total_count_save = 0;

	for(i = 0; i < 25000; i++){
		//printk("current node value: %d\n", current_node->data);
		mutex_lock(&my_mutex);
		current_node = list_first_entry(&my_list, struct my_node, entry);
		ktime_get_ts64(&spclock[0]);
		list_del(&current_node->entry);
		ktime_get_ts64(&spclock[1]);
		kfree(current_node);
		mutex_unlock(&my_mutex);
		calclock(spclock, &total_time_save, &total_count_save);	
	}
	__sync_fetch_and_add(&remove_done, 1);
	__sync_fetch_and_add(&result_time, total_time_save);

	if(remove_done == 4){
		printk("Mutex linked list delete time: %lluns\n", result_time);
		counter = 0;
		result_time = 0;
	}
	
	do_exit(0);
}

static int __init my_mod_init(void)
{
	int i;
	INIT_LIST_HEAD(&my_list);
	
	printk("%s, Entering module\n", __func__);
	
	/*	
	spin_lock_init(&counter_lock);
	for(i = 0; i < 4; i++){
		kthread_run(&spinlock_op, NULL, "spinlock");
	}
	
	
	init_rwsem(&counter_rwse);
	for(i = 0; i < 4; i++){
		kthread_run(&semaphore_op, NULL, "semaphore");
	}
	
	*/
	mutex_init(&my_mutex);
	for(i = 0; i < 4; i++){
		kthread_run(&mutex_op, NULL, "mutex");
	}
	
	
	return 0;
}

static void __exit my_mod_exit(void)
{
	printk("%s, Exiting module\n", __func__);
}

module_init(my_mod_init);
module_exit(my_mod_exit);

MODULE_LICENSE("GPL");
