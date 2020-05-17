#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/mdio.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/kdev_t.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of_device.h>


typedef struct __virtual_dev_s
{	
    char name[32];
    dev_t dev_num;	
    struct cdev chr_dev;
    struct device *base_dev;
    struct i2c_client *client;
}virtual_dev_t;

typedef struct __virtual_i2c_dev_data_t
{
    uint8_t reg;
    uint16_t value;
}virtual_i2c_dev_data_t;

#define DEVICE_MAGIC 'b'
#define VIRTUAL_I2C_READ_VALUE            _IOR(DEVICE_MAGIC, 0x01, virtual_i2c_dev_data_t)      /*read*/
#define VIRTUAL_I2C_WRITE_VALUE            _IOW(DEVICE_MAGIC, 0x01, virtual_i2c_dev_data_t)      /*write*/


