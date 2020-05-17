# virt_i2c

#### 介绍
虚拟i2c控制器驱动实例，可通过该实例进行linux下i2c子系统下i2c driver的学习

#### 软件架构
软件架构说明
在之前分析mdio子系统时，我们提到系统实现了fixed-mii_bus，从而为mac2mac提供虚拟mii_bus和虚拟phy_device，实现对虚拟phy_device的模拟。受此启发，我打算实现一个虚拟的i2c adapter，并基于该i2c adapter挂载一个虚拟的i2c device，并基于虚拟i2c adapter、i2c device搭建一个虚拟的i2c controller即i2c设备，以便为需要学习linux i2c驱动模型的童鞋可以方便的练习i2c device driver。
	本篇文章的目的如下:
	1. 当你没有开发板，又想学习i2c驱动模型，并想进行linux i2c device driver的练习时；
	2. 当你完成了i2c device driver，在硬件开发板出来之前进行i2c device driver的验证时。

	若对linux i2c驱动模型不熟悉时，可参考我之前写的文章：
	《https://www.toutiao.com/i6755852803836477955/》

一、 virtual i2c adapter的实现分析
	我们知道linux系统针对i2c controller抽象了数据结构i2c_adapter，因此我们在进行虚拟i2c 
controller的实现，也就是实现一个i2c_adapter类型的变量，并将其注册至i2c子系统中。

数据结构定义及关联
	因我们需要模拟的是一个虚拟的i2c controller，以及多个依附于虚拟i2c controller的i2c 
device，所以我们定义而定数据结构包含i2c controller、i2c device两部分的内容。
	我们定义了数据结构体virtual_i2c_bus_t，该结构体中包含了i2c_adapter类型的成员和virtual_i2c_adapter_info_t类型的成员，其中：
	1. i2c_adapter类型的成员中，我们需要实现i2c_adapter->algo，因我们抽象的是i2c ，因此我们需要实现master_xfer接口和functionality接口；
而virtual_i2c_adapter_info_t结构体则主要包含了一个双向链表头list，和一个互斥量。list链表用于将所有虚拟i2c设备连接在一起，虚拟i2c设备对应的数据结构为virtual_i2c_dev_info_t，该数据结构包括node节点、virtual i2c device的地址addr，virtual i2c device对应的寄存器等
