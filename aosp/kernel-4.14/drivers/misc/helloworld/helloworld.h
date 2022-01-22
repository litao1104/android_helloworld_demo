/*
 * The helloworld driver
 *
 * A demo linux driver for study and practice
 *
 * Copyleft (c) 2022-2222 litao1104@gmail.com
 *
 *
 * 版本:		 V1.0
 * 最后修改： 2022-01-18
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#ifndef _HELLOWORLD_H_
#define _HELLOWORLD_H_

#define HELLO_EVENT_PRINT_NAME 0x01
#define HELLO_EVENT_PRINT_AGE	0x02
#define HELLO_EVENT_PRINT_NUM	0x03

struct hello_event {
	char name[32];
	int  age;
	int  num;
};

struct hello_list_data {
	int helloween;				 //链表节点的数据域
	struct list_head hello_node; //链表节点的指针域
};

struct helloworld_data {
	/*
	 * 字符设备，创建成功后。
	 * 1. 你会在/sys/dev/char 目录下看到以major:minor设备号的形式命名的本字符设备，作者的是/sys/dev/char/481:0
	 * 2. 你会在/dev 目录下看到/dev/helloworld设备。
	 */
	struct cdev chrdev;

	struct class *hello_class;   //在本实验中用于创建/sys/class/helloworld
	struct device *hello_device; //这个device指针，在本实验中用于在/sys/class/helloworld下创建设备helloworld
	struct list_head hello_list; //链表头，用于存放struct hello_list_data结构体类型的节点

	struct mutex hello_mutex_lock; //互斥锁成员变量

	/*
	 * 一个抽象的任务，既可以放在内核共享工作队列中执行，也可以放到本驱动创建的私有工作队列中执行。放入工作队列的方式：
	 * 1. shedule_work，共享工作队列*system_wq中执行。
	 * 2. queue_work，私有工作队列中执行，如本驱动的*hello_workqueue。当然了，你非要用这个API传入*system_wq指针，也没问题。
	 */
	struct work_struct hello_work;

	/*
	 * 一个抽象的延迟任务，既可以放在内核共享工作队列中执行，也可以放到本驱动创建的私有工作队列中执行。放入工作队列的方式：
	 * 1. shedule_delayed_work，共享工作队列*system_wq中执行。
	 * 2. queue_delayed_work，私有工作队列中执行，如本驱动的*hello_workqueue。当然了，你非要用这个API传入*system_wq指针，也没问题。
	 */
	struct delayed_work hello_delayed_work;

	struct workqueue_struct *hello_wq; //本驱动用于实践的私有工作队列，在初始化时调用create_singlethread_workqueue创建。

	struct hrtimer hello_hrtimer; //用于linux高精度定时器实践

	struct notifier_block hello_fb_notifier; //用于亮灭屏事件通知链回调实践

	struct notifier_block hello_demo_notifier; //用于亮灭屏事件通知链回调实践

	struct hello_event demo_event;

	int is_hello_mutex_manual_locked; //用于标记是否已经被手动上互斥锁
	int hello_kitty; //用于/sys/class/helloworld/helloworld/hello_kitty读写实践，其他一些实践也用到此成员变量。
	int hello_pl;	 //用于/sys/devices/platform/helloworld/hello_pl读写实践，还有一个路径/sys/bus/platform/devices/helloworld/hello_pl
};

extern int hello_register_client(struct notifier_block *nb);
extern int hello_unregister_client(struct notifier_block *nb);
extern int hello_notifier_call_chain(unsigned long val, void *v);

#endif
