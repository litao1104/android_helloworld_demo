/*
 * The helloworld driver
 *
 * A demo linux driver for study and practice
 *
 * Copyleft (c) 2022-2222 litao1104@gmail.com
 *
 *
 * 版本:         V1.0
 * 创建： 2022-01-18，基于arm64 linux kernel 4.14
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/fb.h>
#include <linux/notifier.h>

#include "helloworld.h"

//#define REGISTER_PLATFORM_DEVICE_BUT_NOT_USE_DEVICETREE

#define HELLOWORLD_PROC_FS_NAME  "helloworld"
#define HELLOWORLD_CLASS_NAME    "helloworld"
#define HELLOWORLD_DEVICE_NAME   "helloworld"
#define HELLOWORLD_CHRDEV_NAME   "helloworld"
#define HELLOWORLD_MISC_DEV_NAME "helloworld_misc_dev"

#define HELLOWORLD_PLATFORM_DEVICE_NAME "helloworld"
#define HELLOWORLD_DT_COMPATIBLE_NAME   "litao,helloworld"

#define DELAYED_WORK_INTERVAL_TIME_IN_MS 1000

/* 此函数的地址将赋值到file_operations结构体变量的read函数指针 */
static ssize_t helloworld_read(struct file *filp, char __user *buffer, size_t count, loff_t *ppos)
{
	struct helloworld_data *data = PDE_DATA(file_inode(filp));
	char tmp[32] = {0};
	int len;

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL)
		return -EFAULT;

	mutex_lock(&data->hello_mutex_lock);

	pr_info("%s: line %d: hello_kitty = %d\n", __func__, __LINE__, data->hello_kitty);
	len = sprintf(tmp, "%d\n", data->hello_kitty);

	pr_info("%s: line %d: tmp = %s, len = %d\n", __func__, __LINE__, tmp, len);

	mutex_unlock(&data->hello_mutex_lock);

	return simple_read_from_buffer(buffer, count, ppos, tmp, len);
}

/* 此函数的地址将赋值到file_operations结构体变量的write函数指针 */
static ssize_t helloworld_write(struct file *filp, const char __user *buffer, size_t count, loff_t *ppos)
{
	struct helloworld_data *data = PDE_DATA(file_inode(filp));
	char tmp[32] = {0};
	int ret;

	pr_info("%s: line %d: count = %d\n", __func__, __LINE__, count);

	if (copy_from_user(tmp, buffer, sizeof(tmp)))
		return -EFAULT;

	if (data == NULL)
		return -EFAULT;

	mutex_lock(&data->hello_mutex_lock);

	ret = sscanf(tmp, "%d", &data->hello_kitty);

	pr_info("%s: line %d: tmp = %s\n", __func__, __LINE__, tmp);
	pr_info("%s: line %d: hello_kitty = %d\n", __func__, __LINE__, data->hello_kitty);

	mutex_unlock(&data->hello_mutex_lock);

	return count;
}

/* 此函数的地址将赋值到file_operations结构体变量的open函数指针 */
static int helloworld_open(struct inode *node, struct file *filp)
{
	pr_info("%s: line %d\n", __func__, __LINE__);

	return 0;
}

/* 此函数的地址将赋值到file_operations结构体变量的release函数指针 */
static int helloworld_release(struct inode *node, struct file *filp)
{
	pr_info("%s: line %d\n", __func__, __LINE__);

	return 0;
}

/* file_operations结构体变量 */
static struct file_operations helloworld_ops = {
	.owner = THIS_MODULE,
	.read = helloworld_read,
	.write = helloworld_write,
	.open = helloworld_open,
	.release = helloworld_release,
};

/* miscdevice结构体变量 */
static struct miscdevice helloworld_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = HELLOWORLD_MISC_DEV_NAME,
	.fops = &helloworld_ops,
};

/* 此函数的地址将赋值到device_attribute结构体变量的show函数指针，函数名不一定非要带show字样 */
static ssize_t hello_kitty_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	struct helloworld_data *data = dev_get_drvdata(dev);

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return -EFAULT;
	}

	mutex_lock(&data->hello_mutex_lock);

	pr_info("%s: line %d: hello_kitty = %d\n", __func__, __LINE__, data->hello_kitty);

	mutex_unlock(&data->hello_mutex_lock);

	return snprintf(buf, PAGE_SIZE, "%d\n", data->hello_kitty);
}

/* 此函数的地址将赋值到device_attribute结构体变量的store函数指针，函数名不一定非要带store字样 */
static ssize_t hello_kitty_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	struct helloworld_data *data = dev_get_drvdata(dev);

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return -EFAULT;
	}

	mutex_lock(&data->hello_mutex_lock);

	if (kstrtos32(buf, 10, &data->hello_kitty) == 0) {
		pr_info("%s: line %d: hello_kitty = %d\n", __func__, __LINE__, data->hello_kitty);
	} else {
		pr_err("invalid format = '%s'\n", buf);
	}

	mutex_unlock(&data->hello_mutex_lock);

	return count;
}

/*
 * 实际上，DEVICE_ATTR的作用就是定义了一个struct device_attribute结构体变量 dev_attr_hello_kitty,
 * 并且将hello_kitty_show函数地址和hello_kitty_store函数地址给
 * dev_attr_hello_kitty结构体变量的函数指针成员赋值
 */
static DEVICE_ATTR(hello_kitty, S_IRUGO | S_IWUSR, hello_kitty_show, hello_kitty_store);

/*
 * 此函数的地址将赋值到device_attribute结构体变量的show函数指针，函数名不一定非要带show字样
 *
 * 用于链表操作实践，cat /sys/class/helloworld/helloworld/hello_list，则遍历链表data->hello_list的所有节点，并将
 * 所有节点的成员node->helloween返回和打印
 */
static ssize_t hello_list_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	struct helloworld_data *data = dev_get_drvdata(dev);
    struct hello_list_data *node;

	ssize_t offset = 0;
	int ret;

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return -EFAULT;
	}

	mutex_lock(&data->hello_mutex_lock);

	if (!list_empty(&data->hello_list)) {
		pr_info("%s: line %d: hello_list is OK\n", __func__, __LINE__);
	} else {
		pr_info("%s: line %d: hello_list is empty!\n", __func__, __LINE__);

		mutex_unlock(&data->hello_mutex_lock); //如果前面已上锁，则异常返回时一定要解锁！！！
		return -EFAULT;
	}

    list_for_each_entry(node, &data->hello_list, hello_node) {
        pr_info("%s: line %d: helloween = %d\n", __func__, __LINE__, node->helloween);
		ret = snprintf(&buf[offset], PAGE_SIZE, "%d\n", node->helloween);
		offset += ret;
    }

	mutex_unlock(&data->hello_mutex_lock);

	return offset;
}

/*
 * 此函数的地址将赋值到device_attribute结构体变量的store函数指针，函数名不一定非要带store字样
 *
 * 用于链表操作实践，echo 1 > /sys/class/helloworld/helloworld/hello_list，则从内核堆区申请大小struct hello_list_data的内存，
 * 将创建的节点添加到链表data->hello_list,该链表的类型是struct list_head 。
 */
static ssize_t hello_list_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	struct helloworld_data *data = dev_get_drvdata(dev);
	struct hello_list_data *node = NULL;

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return -EFAULT;
	}

	mutex_lock(&data->hello_mutex_lock);

	node = kzalloc(sizeof(struct hello_list_data), GFP_KERNEL);
	if (node == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);

		mutex_unlock(&data->hello_mutex_lock); //如果前面已上锁，则异常返回时一定要解锁！！！
		return -EFAULT;
	}

	if (kstrtos32(buf, 10, &node->helloween) == 0) {
		pr_info("%s: line %d: helloween = %d\n", __func__, __LINE__, node->helloween);
	} else {
		pr_err("invalid format = '%s'\n", buf);
	}

	list_add_tail(&node->hello_node, &data->hello_list);

	mutex_unlock(&data->hello_mutex_lock);

	return count;
}

/*
 * 实际上，DEVICE_ATTR的作用就是定义了一个struct device_attribute结构体变量 dev_attr_hello_list,
 * 并且将hello_kitty_show函数地址和hello_kitty_store函数地址给
 * dev_attr_hello_kitty结构体变量的函数指针成员赋值
 * 用于链表操作实践
 */
static DEVICE_ATTR(hello_list, S_IRUGO | S_IWUSR, hello_list_show, hello_list_store);

/* 此函数的地址将赋值到device_attribute结构体变量的show函数指针，函数名不一定非要带show字样 */
static ssize_t hello_mutex_lock_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	struct helloworld_data *data = dev_get_drvdata(dev);

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return -EFAULT;
	}

	pr_info("%s: line %d: is_hello_mutex_manual_locked = %d\n", __func__, __LINE__, data->is_hello_mutex_manual_locked);

	return snprintf(buf, PAGE_SIZE, "%d\n", data->is_hello_mutex_manual_locked);
}

/* 此函数的地址将赋值到device_attribute结构体变量的store函数指针，函数名不一定非要带store字样 */
static ssize_t hello_mutex_lock_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	struct helloworld_data *data = dev_get_drvdata(dev);
	int val;

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return -EFAULT;
	}

	if (kstrtos32(buf, 10, &val) == 0) {
		pr_info("%s: line %d: val = %d\n", __func__, __LINE__, val);
	} else {
		pr_err("invalid format = '%s'\n", buf);
	}

	if ((val != 0) && (val != 1)) {
		pr_err("%s: line %d: invalid value!\n", __func__, __LINE__);
		return -EFAULT;
	}

	if (val) {
		if (data->is_hello_mutex_manual_locked == 0) {
			pr_info("%s: line %d: start to mutex lock\n", __func__, __LINE__);

			data->is_hello_mutex_manual_locked = 1;
			mutex_lock(&data->hello_mutex_lock);
		} else {
			pr_err("%s: line %d: warning: mutex already locked\n", __func__, __LINE__);
		}
	} else {
		if (data->is_hello_mutex_manual_locked == 1) {
			pr_info("%s: line %d: start to mutex unlock\n", __func__, __LINE__);

			data->is_hello_mutex_manual_locked = 0;
			mutex_unlock(&data->hello_mutex_lock);
		} else {
			pr_err("%s: line %d: warning: mutex already unlocked\n", __func__, __LINE__);
		}
	}

	return count;
}

/*
 * 实际上，DEVICE_ATTR的作用就是定义了一个struct device_attribute结构体变量 dev_attr_hello_mutex_lock,
 * 并且将hello_mutex_lock_show函数地址和hello_mutex_lock_store函数地址给
 * dev_attr_hello_mutex_lock结构体变量的函数指针成员赋值
 *
 * /sys/class/helloworld/helloworld/hello_mutex_lock的用于互斥锁实验
 * echo 1 > /sys/class/helloworld/helloworld/hello_mutex_lock，上锁
 * echo 0 > /sys/class/helloworld/helloworld/hello_mutex_lock，解锁
 * 当上锁后，再echo 1 > /sys/class/helloworld/helloworld/hello_kitty，观察会有什么现象
 */
static DEVICE_ATTR(hello_mutex_lock, S_IRUGO | S_IWUSR, hello_mutex_lock_show, hello_mutex_lock_store);

/* 此函数的地址将赋值到device_attribute结构体变量的store函数指针，函数名不一定非要带store字样 */
static ssize_t hello_schedule_work_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	struct helloworld_data *data = dev_get_drvdata(dev);
	int val;

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return -EFAULT;
	}

	if (kstrtos32(buf, 10, &val) == 0) {
		pr_info("%s: line %d: val = %d\n", __func__, __LINE__, val);
	} else {
		pr_err("invalid format = '%s'\n", buf);
	}

	if (val != 1) {
		pr_err("%s: line %d: invalid value!\n", __func__, __LINE__);
		return -EFAULT;
	}

	schedule_work(&data->hello_work);

	return count;
}

/*
 * 实际上，DEVICE_ATTR的作用就是定义了一个struct device_attribute结构体变量 dev_attr_hello_schedule_work,
 * 并且将hello_schedule_work_store函数地址给dev_attr_hello_schedule_work结构体变量的函数指针成员赋值
 *
 * /sys/class/helloworld/helloworld/hello_schedule_work的用于共享工作队列实验
 * echo 1 > /sys/class/helloworld/helloworld/hello_schedule_work，放入共享工作队列中执行。
 */
static DEVICE_ATTR(hello_schedule_work, S_IWUSR, NULL, hello_schedule_work_store);

/* 此函数的地址将赋值到device_attribute结构体变量的store函数指针，函数名不一定非要带store字样 */
static ssize_t hello_schedule_delayed_work_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	struct helloworld_data *data = dev_get_drvdata(dev);
	int val;

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return -EFAULT;
	}

	if (kstrtos32(buf, 10, &val) == 0) {
		pr_info("%s: line %d: val = %d\n", __func__, __LINE__, val);
	} else {
		pr_err("invalid format = '%s'\n", buf);
	}

	if ((val != 0) && (val != 1)) {
		pr_err("%s: line %d: invalid value!\n", __func__, __LINE__);
		return -EFAULT;
	}

	if (val) {
		schedule_delayed_work(&data->hello_delayed_work, msecs_to_jiffies(DELAYED_WORK_INTERVAL_TIME_IN_MS));
	} else {
		cancel_delayed_work_sync(&data->hello_delayed_work);
	}

	return count;
}

/*
 * 实际上，DEVICE_ATTR的作用就是定义了一个struct device_attribute结构体变量 dev_attr_hello_schedule_delayed_work,
 * 并且将hello_schedule_delayed_work_store函数地址给dev_attr_hello_schedule_delayed_work结构体变量的函数指针成员赋值
 *
 * /sys/class/helloworld/helloworld/hello_schedule_delayed_work的用于抽象延迟任务实验
 * echo 1 > /sys/class/helloworld/helloworld/hello_schedule_delayed_work，将抽象延迟任务放入共享工作队列中执行。
 * echo 0 > /sys/class/helloworld/helloworld/hello_schedule_delayed_work，取消共享工作队列中的的抽象延迟任务。
 */
static DEVICE_ATTR(hello_schedule_delayed_work, S_IWUSR, NULL, hello_schedule_delayed_work_store);

/* 此函数的地址将赋值到device_attribute结构体变量的store函数指针，函数名不一定非要带store字样 */
static ssize_t hello_queue_work_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	struct helloworld_data *data = dev_get_drvdata(dev);
	int val;

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return -EFAULT;
	}

	if (kstrtos32(buf, 10, &val) == 0) {
		pr_info("%s: line %d: val = %d\n", __func__, __LINE__, val);
	} else {
		pr_err("invalid format = '%s'\n", buf);
	}

	if (val != 1) {
		pr_err("%s: line %d: invalid value!\n", __func__, __LINE__);
		return -EFAULT;
	}

	if (data->hello_wq != NULL)
		queue_work(data->hello_wq, &data->hello_work);
	else
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);

	return count;
}

/*
 * 实际上，DEVICE_ATTR的作用就是定义了一个struct device_attribute结构体变量 dev_attr_hello_queue_work,
 * 并且将hello_queue_work_store函数地址给dev_attr_hello_queue_work结构体变量的函数指针成员赋值
 *
 * /sys/class/helloworld/helloworld/hello_queue_work的用于私有工作队列实验
 * echo 1 > /sys/class/helloworld/helloworld/hello_queue_work，放入私有工作队列hello_wq中执行。
 */
static DEVICE_ATTR(hello_queue_work, S_IWUSR, NULL, hello_queue_work_store);

/* 此函数的地址将赋值到device_attribute结构体变量的store函数指针，函数名不一定非要带store字样 */
static ssize_t hello_queue_delayed_work_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	struct helloworld_data *data = dev_get_drvdata(dev);
	int val;

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return -EFAULT;
	}

	if (kstrtos32(buf, 10, &val) == 0) {
		pr_info("%s: line %d: val = %d\n", __func__, __LINE__, val);
	} else {
		pr_err("invalid format = '%s'\n", buf);
	}

	if ((val != 0) && (val != 1)) {
		pr_err("%s: line %d: invalid value!\n", __func__, __LINE__);
		return -EFAULT;
	}

	if (data->hello_wq != NULL) {
		if (val) {
			queue_delayed_work(data->hello_wq, &data->hello_delayed_work, msecs_to_jiffies(DELAYED_WORK_INTERVAL_TIME_IN_MS));
		} else {
			cancel_delayed_work_sync(&data->hello_delayed_work);
		}
	} else {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
	}

	return count;
}

/*
 * 实际上，DEVICE_ATTR的作用就是定义了一个struct device_attribute结构体变量 dev_attr_hello_queue_delayed_work,
 * 并且将hello_queue_delayed_work_store函数地址给dev_attr_hello_queue_delayed_work结构体变量的函数指针成员赋值
 *
 * /sys/class/helloworld/helloworld/hello_queue_delayed_work的用于抽象延迟任务实验
 * echo 1 > /sys/class/helloworld/helloworld/hello_queue_delayed_work，将抽象延迟任务放入私有工作队列中执行。
 * echo 0 > /sys/class/helloworld/helloworld/hello_queue_delayed_work，取消私有工作队列中的的抽象延迟任务。
 */
static DEVICE_ATTR(hello_queue_delayed_work, S_IWUSR, NULL, hello_queue_delayed_work_store);

/* 此函数的地址将赋值到device_attribute结构体变量的store函数指针，函数名不一定非要带store字样 */
static ssize_t hello_hrtimer_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	struct helloworld_data *data = dev_get_drvdata(dev);
	int val;

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return -EFAULT;
	}

	if (kstrtos32(buf, 10, &val) == 0) {
		pr_info("%s: line %d: val = %d\n", __func__, __LINE__, val);
	} else {
		pr_err("invalid format = '%s'\n", buf);
	}

	if ((val != 0) && (val != 1)) {
		pr_err("%s: line %d: invalid value!\n", __func__, __LINE__);
		return -EFAULT;
	}

	if (val) {
		hrtimer_start(&data->hello_hrtimer, ktime_set(1, 0), HRTIMER_MODE_REL);
	} else {
		hrtimer_cancel(&data->hello_hrtimer);
	}

	return count;
}

/*
 * 实际上，DEVICE_ATTR的作用就是定义了一个struct device_attribute结构体变量 dev_attr_hello_hrtimer,
 * 并且将hello_hrtimer_store函数地址给dev_attr_hello_hrtimer结构体变量的函数指针成员赋值
 *
 * /sys/class/helloworld/helloworld/hello_hrtimer的用于linux高精度定时器实验
 * echo 1 > /sys/class/helloworld/helloworld/hello_hrtimer，启动定时器。
 * echo 0 > /sys/class/helloworld/helloworld/hello_hrtimer，取消定时器。
 */
static DEVICE_ATTR(hello_hrtimer, S_IWUSR, NULL, hello_hrtimer_store);

/* 此函数的地址将赋值到device_attribute结构体变量的store函数指针，函数名不一定非要带store字样 */
static ssize_t hello_notifier_demo_event_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	struct helloworld_data *data = dev_get_drvdata(dev);
	int val;
	int ret;

	struct hello_event event = {
		.name = "litao",
		.age = 18,
		.num = 110,
	};

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return -EFAULT;
	}

	if (kstrtos32(buf, 10, &val) == 0) {
		pr_info("%s: line %d: val = %d\n", __func__, __LINE__, val);
	} else {
		pr_err("invalid format = '%s'\n", buf);
	}

	if ((val != HELLO_EVENT_PRINT_NAME) &&
		(val != HELLO_EVENT_PRINT_AGE) &&
		(val != HELLO_EVENT_PRINT_NUM)) {
		pr_err("%s: line %d: invalid value!\n", __func__, __LINE__);
		return -EFAULT;
	}

	ret = hello_notifier_call_chain((unsigned long)val, (void*)&event); //发出事件通知链

	return count;
}

/*
 * 实际上，DEVICE_ATTR的作用就是定义了一个struct device_attribute结构体变量 dev_attr_hello_notifier_demo_event,
 * 并且将hello_notifier_demo_event_store函数地址给dev_attr_hello_notifier_demo_event结构体变量的函数指针成员赋值
 *
 * /sys/class/helloworld/helloworld/hello_notifier_demo_event用于hello notifer call的事件通知链实验
 * echo 1 > /sys/class/helloworld/helloworld/hello_notifier_demo_event，发出事件通知，事件类型是HELLO_EVENT_PRINT_NAME
 * echo 2 > /sys/class/helloworld/helloworld/hello_notifier_demo_event，发出事件通知，事件类型是HELLO_EVENT_PRINT_AGE
 * echo 3 > /sys/class/helloworld/helloworld/hello_notifier_demo_event，发出事件通知，事件类型是HELLO_EVENT_PRINT_NUM
 *
 * 发出事件通知后，回调函数hello_demo_notifier_callback就会执行
 */
static DEVICE_ATTR(hello_notifier_demo_event, S_IWUSR, NULL, hello_notifier_demo_event_store);

/* 此函数的地址将赋值到device_attribute结构体变量的show函数指针，函数名不一定非要带show字样 */
static ssize_t hello_pl_show(struct device* dev, struct device_attribute* attr, char* buf)
{
	struct helloworld_data *data = dev_get_drvdata(dev);

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return -EFAULT;
	}

	mutex_lock(&data->hello_mutex_lock);

	pr_info("%s: line %d: hello_pl = %d\n", __func__, __LINE__, data->hello_pl);

	mutex_unlock(&data->hello_mutex_lock);

	return snprintf(buf, PAGE_SIZE, "%d\n", data->hello_pl);
}

/* 此函数的地址将赋值到device_attribute结构体变量的store函数指针，函数名不一定非要带store字样 */
static ssize_t hello_pl_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
	struct helloworld_data *data = dev_get_drvdata(dev);

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return -EFAULT;
	}

	mutex_lock(&data->hello_mutex_lock);

	if (kstrtos32(buf, 10, &data->hello_pl) == 0) {
		pr_info("%s: line %d: hello_pl = %d\n", __func__, __LINE__, data->hello_pl);
	} else {
		pr_err("invalid format = '%s'\n", buf);
	}

	mutex_unlock(&data->hello_mutex_lock);

	return count;
}

/*
 * 实际上，DEVICE_ATTR的作用就是定义了一个struct device_attribute结构体变量 dev_attr_hello_pl,
 * 并且将hello_pl_show函数地址和hello_pl_store函数地址给
 * dev_attr_hello_pl结构体变量的函数指针成员赋值
 */
static DEVICE_ATTR(hello_pl, S_IRUGO | S_IWUSR, hello_pl_show, hello_pl_store);

/*
 * 此函数hello_work_func与struct helloworld_data结构的成员struct work_struct hello_work绑定，
 * 所绑定的hello_work既可以放到共享工作队列中执行，也可以放到私有工作队列中执行。
 */
static void hello_work_func(struct work_struct *work)
{
	/*
	 * container_of，通过hello_work成员变量的地址和该成员相对于struct helloworld_data首地址的偏移量，
	 * 就可以算出struct helloworld_data的首地址。
	 */
	struct helloworld_data *data = container_of(work, struct helloworld_data, hello_work);

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return;
	}

	pr_info("%s: line %d: hello_kitty = %d\n", __func__, __LINE__, data->hello_kitty);

	return;
}

/*
 * 此函数hello_delayed_work_func与struct helloworld_data结构的成员struct delayed_work hello_delayed_work绑定，
 * 所绑定的hello_delayed_work既可以放到共享工作队列中执行，也可以放到私有工作队列中执行。
 */
static void hello_delayed_work_func(struct work_struct *work)
{
	/*
	 * container_of，通过hello_work成员变量的成员work的地址和该成员变量的成员work相对于struct helloworld_data首地址的偏移量，
	 * 就可以算出struct helloworld_data的首地址。
	 */
	struct helloworld_data *data = container_of(work, struct helloworld_data, hello_delayed_work.work);

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return;
	}

	pr_info("%s: line %d: hello_kitty = %d\n", __func__, __LINE__, data->hello_kitty);

	schedule_delayed_work(&data->hello_delayed_work, msecs_to_jiffies(DELAYED_WORK_INTERVAL_TIME_IN_MS));

	return;
}

/*
 * 此函数hello_hrtimer_func与struct helloworld_data结构的成员struct hrtimer hello_hrtimer绑定，
 * 也就是hello_hrtimer的定时器超时回调函数。
 */
static enum hrtimer_restart hello_hrtimer_func(struct hrtimer *timer)
{
	/*
	 * container_of，通过hello_hrtimer成员变量的地址和该成员变量相对于struct helloworld_data首地址的偏移量，
	 * 就可以算出struct helloworld_data的首地址。
	 */
	struct helloworld_data *data = container_of(timer, struct helloworld_data, hello_hrtimer);

	pr_info("%s: line %d\n", __func__, __LINE__);

	if (data == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
		return HRTIMER_NORESTART;
	}

	pr_info("%s: line %d\n", __func__, __LINE__);

	hrtimer_start(&data->hello_hrtimer, ktime_set(1, 0), HRTIMER_MODE_REL);

	return HRTIMER_NORESTART;
}

/* fb亮灭屏事件通知链的回调函数 */
int hello_fb_notifier_callback(struct notifier_block *nb, unsigned long event, void *data)
{
	struct helloworld_data *hello_data = container_of(nb, struct helloworld_data, hello_fb_notifier);

	struct fb_event *evdata = data;
	int *blank;

	pr_info("%s: line %d: event = %d\n", __func__, __LINE__, (int)event);

	if (evdata && evdata->data && event == FB_EVENT_BLANK && data) {
		blank = evdata->data;
		pr_info("%s: line %d: *blank = %d\n", __func__, __LINE__, *blank);

		switch (*blank) {
		case FB_BLANK_UNBLANK:
			/* 在终端设备亮屏流程中，添加你需要做的事情，例如： */
			pr_info("%s: line %d: hello FB_BLANK_UNBLANK\n", __func__, __LINE__);

			hello_data->hello_kitty = 12345;
			schedule_work(&hello_data->hello_work);
			break;

		case FB_BLANK_POWERDOWN:
		case FB_BLANK_HSYNC_SUSPEND:
		case FB_BLANK_VSYNC_SUSPEND:
		case FB_BLANK_NORMAL:
			/* 在终端设备熄屏流程中，添加你需要做的事情，例如： */
			pr_info("%s: line %d: hello FB_BLANK_POWERDOWN\n", __func__, __LINE__);

			hello_data->hello_kitty = 67890;
			if (hello_data->hello_wq != NULL)
				queue_work(hello_data->hello_wq, &hello_data->hello_work);
			break;
		}
	}

	return 0;
}

/*
 * 从这里往下，是notfier的demo实践，helloworld是广播发出者，即事件的通知者，
 * 与fb的notifier没有任何关系，不要然在一起。
 */

/*
 * 定义一个结构体变量，相当于struct blocking_notifier_head hello_notifier_list;
 *
 * 当调用fb_notifier_call_chain发出事件通知时，会遍历链表hello_notifier_list，
 * 挨个调用此链表每一个notifier_block类型节点的notfiler_call函数指针。
 */
static BLOCKING_NOTIFIER_HEAD(hello_notifier_list);

/*
 *	提供给被通知者注册的API，谁想接收fb_notifier_call_chain的事件，谁就调用注册。
 *	参数notifier_block *nb，是一个即将添加到链表hello_notifier_list的节点。
 */
int hello_register_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&hello_notifier_list, nb);
}
EXPORT_SYMBOL(hello_register_client);

/*
 *	提供给被通知者取消注册的API，谁不再想接收fb_notifier_call_chain的事件，谁就调用取消注册。
 *	参数notifier_block *nb，是一个即将从链表hello_notifier_list中删除的节点。
 */
int hello_unregister_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&hello_notifier_list, nb);
}
EXPORT_SYMBOL(hello_unregister_client);

/*
 * 调用此API发出hello事件通知，并调用链表hello_notifier_list中所有notifier_block节点的notifier_call函数指针。
 * 参数：
 * val: 可以代表事件类型，在各被通知者定义的callback函数中可获取此值。
 * *v : 调用此API发出hello事件的通知者想要传递的数据地址，在各被通知者定义的callback函数中可获取此地址。
 */
int hello_notifier_call_chain(unsigned long val, void *v)
{
	return blocking_notifier_call_chain(&hello_notifier_list, val, v);
}
EXPORT_SYMBOL_GPL(hello_notifier_call_chain);

/*
 * hello event事件通知的回调函数。
 * 本驱动是个特例，自己通知自己。在hello_notifer_demo_event_store函数中发出通知。
 * 演示如下：
 */
int hello_demo_notifier_callback(struct notifier_block *nb, unsigned long event, void *data)
{
	struct helloworld_data *hello_data = container_of(nb, struct helloworld_data, hello_demo_notifier);

	struct hello_event *evdata = data;

	pr_info("%s: line %d: event = %d\n", __func__, __LINE__, (int)event);

	if (hello_data && evdata && data) {
		switch (event) {
		case HELLO_EVENT_PRINT_NAME:
			memcpy(hello_data->demo_event.name, evdata->name, sizeof(hello_data->demo_event.name));
			pr_info("%s: line %d: name = %s sizeof name is %d\n", __func__, __LINE__,
					hello_data->demo_event.name, sizeof(hello_data->demo_event.name));
			break;

		case HELLO_EVENT_PRINT_AGE:
			hello_data->demo_event.age = evdata->age;
			pr_info("%s: line %d: age = %d\n", __func__, __LINE__, hello_data->demo_event.age);
			break;

		case HELLO_EVENT_PRINT_NUM:
			hello_data->demo_event.num = evdata->num;
			pr_info("%s: line %d: num = %d\n", __func__, __LINE__, hello_data->demo_event.num);
			break;
		}
	}

	return 0;
}

/*
 * 定义一个platform_device全局变量，
 * 然后在helloworld_init函数中调用platform_device_register向内核注册
 * 注册成功后，通过adb，你会看到/sys/devices/platform/helloworld，这个设备的名称是由下方
 * 变量helloworld_platform_device.name来决定。
 */
#ifdef REGISTER_PLATFORM_DEVICE_BUT_NOT_USE_DEVICETREE
static struct platform_device helloworld_platform_device = {
	.name = HELLOWORLD_PLATFORM_DEVICE_NAME,
	.id = -1, //.id = -1，则设备路径是/sys/devices/platform/helloworld；而如果id = 0，则此设备路径是/sys/devices/platform/helloworld.0。
};
#else
/*
 * helloworld_of_match_table变量，用成员compatible字符串匹配dts的，
 * 当dts匹配成功，那么就会调用platform_driver的probe成员，开始执行初始化流程。例如：
 * helloworld:helloworld {
 *     compatible = "litao,helloworld";
 *     status = "okay";
 * };
 *
 * 通过adb，你会看到/sys/devices/platform/helloworld，helloworld这个设备的名称是由dts中的helloworld:helloworld来决定，
 * 而不是compatible的字符串。
 */
static struct of_device_id helloworld_of_match_table[] = {
	{ .compatible = HELLOWORLD_DT_COMPATIBLE_NAME },
	{},
};
#endif

static int helloworld_platform_probe(struct platform_device *pdev)
{
	struct proc_dir_entry *proc_entry; /* 用于在/proc/目录下，创建proc fs设备 */
	static struct helloworld_data *data = NULL; /* struct helloworld_data 是本demo的主要驱动数据结构 */

	int chrdev_major;
	int chrdev_minor;

	dev_t devno = 0;

	int ret;

	pr_info("%s: line %d\n", __func__, __LINE__);

	data = kzalloc(sizeof(struct helloworld_data), GFP_KERNEL);
	if (data == NULL) {
		pr_err("%s: line %d: Couldn't alloc memory!\n", __func__, __LINE__);
		return -EFAULT;
	} else {
		pr_info("%s: line %d: Alloc memory success!\n", __func__, __LINE__);
	}

	/* 用于在/proc/目录下，创建proc fs设备。与proc_create不同的是，proc_create_data多了一个参数，传递驱动数据结构的指针 */
	proc_entry = proc_create_data(HELLOWORLD_PROC_FS_NAME, 0660, NULL, &helloworld_ops, data);
	if (proc_entry == NULL) {
		pr_err("%s: line %d: Couldn't create proc entry!\n", __func__, __LINE__);
	} else {
		pr_info("%s: line %d: Create proc entry success!\n", __func__, __LINE__);
	}

	/* class_create在/sys/class/目录下，创建目录  ，目录的名字就是字符串宏HELLOWORLD_CLASS_NAME */
	data->hello_class = class_create(THIS_MODULE, HELLOWORLD_CLASS_NAME);
	if (IS_ERR(data->hello_class)) {
		pr_err("%s: line %d: Couldn't alloc memory!\n", __func__, __LINE__);
		return -EFAULT;
	} else {
		pr_info("%s: line %d: Alloc memory success!\n", __func__, __LINE__);
	}

	/* 由内核分配字符设备号 */
	ret = alloc_chrdev_region(&devno, 0, 1, HELLOWORLD_CHRDEV_NAME);
	if(ret < 0) {
	    pr_err("Failed to alloc chrdev!\n");
	}

	/* 根据字符设备号devno，计算出major和minor */
	chrdev_major = MAJOR(devno);
	chrdev_minor = MINOR(devno);

	pr_info("%s: line %d: devno = %d, chrdev_major = %d, chrdev_minor = %d\n", __func__, __LINE__,
			(int)devno, chrdev_major, chrdev_minor);

	/* 初始化字符设备 */
	cdev_init(&data->chrdev, &helloworld_ops);
	data->chrdev.owner = THIS_MODULE;

	/* 为内核新增字符设备，在/sys/dev/char/目录下以设备号的方式呈现，同时，在/dev目录下也会创建 */
	ret = cdev_add(&data->chrdev, devno, 1);
	if (ret) {
		pr_err("%s: line %d: Couldn't add cdev!\n", __func__, __LINE__);
	}

	/* 在/dev/目录下创建名为HELLOWORLD_MISC_DEV_NAME的设备文件 */
	ret = misc_register(&helloworld_misc_device);
	if (ret) {
		pr_err("helloworld_misc_device register failed\n");
		return -EFAULT;
	} else {
		pr_info("%s: line %d: Register misc device success!\n", __func__, __LINE__);
	}

	/* 在/sys/class/helloworld/目录下创建helloworld设备和目录 */
	data->hello_device = device_create(data->hello_class, NULL, devno, NULL, HELLOWORLD_DEVICE_NAME);
	if (IS_ERR(data->hello_device)) {
		pr_err("%s: line %d: Couldn't create device!\n", __func__, __LINE__);
		return -EFAULT;
	} else {
		pr_info("%s: line %d: Create device success!\n", __func__, __LINE__);
	}

	dev_set_drvdata(data->hello_device, data);

	/* 在/sys/class/helloworld/helloworld/目录下创建hello_kitty属性，也就是文件节点 */
	ret = device_create_file(data->hello_device, &dev_attr_hello_kitty);
	if (ret < 0) {
	    pr_err("Failed to create attribute hello_kitty!\n");
	} else {
		pr_info("%s: line %d: Device create hello_kitty file success!\n", __func__, __LINE__);
	}

	/* 初始化链表, 用于链表操作实践 */
	INIT_LIST_HEAD(&data->hello_list);

	/* 在/sys/class/helloworld/helloworld/目录下创建hello_list属性，也就是文件节点 */
	ret = device_create_file(data->hello_device, &dev_attr_hello_list);
	if (ret < 0) {
	    pr_err("Failed to create attribute hello_list!\n");
	} else {
		pr_info("%s: line %d: Device create hello_list file success!\n", __func__, __LINE__);
	}

	/* 在/sys/class/helloworld/helloworld/目录下创建hello_mutex_lock属性，也就是文件节点 */
	ret = device_create_file(data->hello_device, &dev_attr_hello_mutex_lock);
	if (ret < 0) {
	    pr_err("Failed to create attribute hello_mutex_lock!\n");
	} else {
		pr_info("%s: line %d: Device create hello_mutex_lock file success!\n", __func__, __LINE__);
	}

	/* 在/sys/class/helloworld/helloworld/目录下创建hello_schedule_work属性，也就是文件节点 */
	ret = device_create_file(data->hello_device, &dev_attr_hello_schedule_work);
	if (ret < 0) {
	    pr_err("Failed to create attribute hello_schedule_work!\n");
	} else {
		pr_info("%s: line %d: Device create hello_schedule_work file success!\n", __func__, __LINE__);
	}

	/* 在/sys/class/helloworld/helloworld/目录下创建hello_schedule_delayed_work属性，也就是文件节点 */
	ret = device_create_file(data->hello_device, &dev_attr_hello_schedule_delayed_work);
	if (ret < 0) {
	    pr_err("Failed to create attribute hello_schedule_delayed_work!\n");
	} else {
		pr_info("%s: line %d: Device create hello_schedule_delayed_work file success!\n", __func__, __LINE__);
	}

	/* 在/sys/class/helloworld/helloworld/目录下创建hello_queue_work属性，也就是文件节点 */
	ret = device_create_file(data->hello_device, &dev_attr_hello_queue_work);
	if (ret < 0) {
	    pr_err("Failed to create attribute hello_queue_work!\n");
	} else {
		pr_info("%s: line %d: Device create hello_queue_work file success!\n", __func__, __LINE__);
	}

	/* 在/sys/class/helloworld/helloworld/目录下创建hello_queue_delayed_work属性，也就是文件节点 */
	ret = device_create_file(data->hello_device, &dev_attr_hello_queue_delayed_work);
	if (ret < 0) {
	    pr_err("Failed to create attribute hello_queue_delayed_work!\n");
	} else {
		pr_info("%s: line %d: Device create hello_queue_delayed_work file success!\n", __func__, __LINE__);
	}

	/* 在/sys/class/helloworld/helloworld/目录下创建hello_hrtimer属性，也就是文件节点 */
	ret = device_create_file(data->hello_device, &dev_attr_hello_hrtimer);
	if (ret < 0) {
	    pr_err("Failed to create attribute hello_hrtimer!\n");
	} else {
		pr_info("%s: line %d: Device create hello_hrtimer file success!\n", __func__, __LINE__);
	}

	/* 在/sys/class/helloworld/helloworld/目录下创建hello_hrtimer属性，也就是文件节点 */
	ret = device_create_file(data->hello_device, &dev_attr_hello_hrtimer);
	if (ret < 0) {
	    pr_err("Failed to create attribute hello_hrtimer!\n");
	} else {
		pr_info("%s: line %d: Device create hello_hrtimer file success!\n", __func__, __LINE__);
	}

	/* 在/sys/class/helloworld/helloworld/目录下创建hello_notifier_demo_event属性，也就是文件节点 */
	ret = device_create_file(data->hello_device, &dev_attr_hello_notifier_demo_event);
	if (ret < 0) {
	    pr_err("Failed to create attribute hello_notifier_demo_event!\n");
	} else {
		pr_info("%s: line %d: Device create hello_notifier_demo_event file success!\n", __func__, __LINE__);
	}

	/* 下面是在/sys/bus/platform/helloworld/目录下创建hello_pl属性 */
	if (pdev == NULL) {
		pr_err("%s: line %d: NULL pointer error!\n", __func__, __LINE__);
	} else {
		dev_set_drvdata(&pdev->dev, data);

		ret = device_create_file(&pdev->dev, &dev_attr_hello_pl);
		if(ret < 0) {
		    pr_err("Failed to create attribute hello_pl!\n");
		} else {
			pr_info("%s: line %d: Device create hello_pl file for device of platform device success!\n", __func__, __LINE__);
		}
	}

	/* 互斥锁初始化 */
	mutex_init(&data->hello_mutex_lock);

	/* hello_work抽象任务初始化，具体的实例实现是hello_work_func函数 */
	INIT_WORK(&data->hello_work, hello_work_func);

	/* hello_delayed_work延迟抽象任务初始化，具体的实例实现是hello_delayed_work_func函数 */
	INIT_DELAYED_WORK(&data->hello_delayed_work, hello_delayed_work_func);

	/* 创建私有的内核工作队列hello_wq，标记为"helloworld workqueue" */
	data->hello_wq = create_singlethread_workqueue("helloworld workqueue");

	/*
	 * 初始化高精度定时器，用hello_hrtimer_func函数地址将hello_hrtimer的function函数指针成员实例化。
	 */
	hrtimer_init(&data->hello_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	data->hello_hrtimer.function = hello_hrtimer_func;

	/* 
	 * 注册fb亮灭屏事件，实际是将hello_fb_notifier添加到fb的链表中，
	 * 使用hello_fb_notifier_callback函数地址将hello_fb_notifier的notifier_call函数指针实例化。
	 * 
	 * 注意理解概念：hello_fb_notifier对于fb来说，只是一个接收广播者，而fb是这个广播的通知者。
	 * hello_fb_notifier是被动的，也就是客户端client。
	 */
	data->hello_fb_notifier.notifier_call = hello_fb_notifier_callback;
	ret = fb_register_client(&data->hello_fb_notifier);
	if (ret)
		pr_err("Failed to register fb client!\n");

	data->hello_demo_notifier.notifier_call = hello_demo_notifier_callback;
	ret = hello_register_client(&data->hello_demo_notifier);
	if (ret)
		pr_err("Failed to register hello client!\n");

	return 0;
}

/* 热插拔设备内核模块移除时调用，但我们做机的，一般都是将模块编译进内核。后续的android版本可能要求驱动编译为ko，就会用到。 */
static int helloworld_platform_remove(struct platform_device *pdev)
{
	pr_info("%s: line %d\n", __func__, __LINE__);

	return 0;
}

/* kernel挂起(也就是待机)流程中，由kernel发起调用，需要将此函数赋值给platform_drver的suspend函数指针 */
static int helloworld_platform_suspend(struct platform_device *pdev, pm_message_t state)
{
	pr_info("%s: line %d\n", __func__, __LINE__);

	return 0;
}

/* kernel唤醒流程中，由kernel发起调用，需要将此函数赋值给platform_drver的resume函数指针 */
static int helloworld_platform_resume(struct platform_device *pdev)
{
	pr_info("%s: line %d\n", __func__, __LINE__);

	return 0;
}

/*
 * 注册一个platform_driver，这些函数指针什么时候才可以被调用呢？
 *
 * 第一种条件：驱动调用platform_device_register Linux API向内核注册一个platform device，
 * 然后调用platform_driver_register Linux API向内核注册一个platform driver。
 * 最后内核检查platform_driver.driver.name与platform_deivce.name的一致，匹配成功后调用platfor driver的probe函数。
 *
 * 第二种条件：当dts的compitible与of_match_table的字符串匹配时。这种匹配，不依赖platform deivce，不注册都可以。
 * 所以，也和platform driver的driver.name没有关系。
 *
 * 两种条件任意一个满足时，内核就会开始调用probe函数指针。
 */
static struct platform_driver helloworld_platform_driver = {
	.probe = helloworld_platform_probe,
	.remove = helloworld_platform_remove,
	.suspend = helloworld_platform_suspend,
	.resume = helloworld_platform_resume,
	.driver = {
		.name = HELLOWORLD_PLATFORM_DEVICE_NAME,
		.owner = THIS_MODULE,
#ifndef REGISTER_PLATFORM_DEVICE_BUT_NOT_USE_DEVICETREE
		.of_match_table = helloworld_of_match_table,
#endif
	},
};

/* 内核模块初始化，此驱动在开机时执行的第一个函数 */
static int helloworld_init(void)
{
	int ret;

	pr_info("%s: line %d\n", __func__, __LINE__);

#ifdef REGISTER_PLATFORM_DEVICE_BUT_NOT_USE_DEVICETREE
	ret = platform_device_register(&helloworld_platform_device);
	if (ret) {
		pr_err("%s: line %d: Couldn't register platform device!\n", __func__, __LINE__);
	} else {
		pr_info("%s: line %d: Rigister platform device success!\n", __func__, __LINE__);
	}
#endif

	ret = platform_driver_register(&helloworld_platform_driver);
	if (ret) {
		pr_err("%s: line %d: Couldn't register platform driver!\n", __func__, __LINE__);
	} else {
		pr_info("%s: line %d: Rigister platform driver success!\n", __func__, __LINE__);
	}

	return 0;
}

static void helloworld_exit(void)
{
	pr_info("%s: line %d\n", __func__, __LINE__);

	return;
}

/* 常规操作，套路 */
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Helloworld driver");

/* late_initcall的作用于module_init()相同，用于内核模块初始化 */
late_initcall(helloworld_init);
module_exit(helloworld_exit);
