#include "virtual_i2cdev.h"

#define DEV_NAME "virtual_i2cdev"

static struct class *class_ptr;

static int virtual_i2c_dev_ctrl_open(struct inode *inode, struct file *filp)
{
    struct cdev *cdev = inode->i_cdev;
    virtual_dev_t *devp = container_of(cdev, struct __virtual_dev_s, chr_dev);

    filp->private_data = devp;

    return 0;
}

static int virtual_i2c_dev_ctrl_close(struct inode *inode, struct file *file)
{
    file->private_data  = NULL;
    return 0;
}



static int virtual_i2c_dev_set_value(virtual_dev_t *devp, virtual_i2c_dev_data_t *infop)
{
    struct i2c_msg msg;
    uint8_t buff[4];

    if((devp == NULL) || (infop == NULL))
    {
        return -EINVAL;
    }

    /*write operation*/
    memset(&msg, 0, sizeof(msg));
    msg.addr = devp->client->addr;
    msg.buf = buff;

    buff[0] = infop->reg&0x00FF;

    msg.len = 3;
    buff[1] = infop->value>>8;
    buff[2] = infop->value&0x00FF;

    return  i2c_transfer(devp->client->adapter, &msg, 1);
}

static int virtual_i2c_dev_get_value(virtual_dev_t *devp, virtual_i2c_dev_data_t *infop)
{
    struct i2c_msg msg;
    uint8_t buff[4];
    int result = 0;

    if((devp == NULL) || (infop == NULL))
    {
        return -EINVAL;
    }

    /*write operation*/
    memset(&msg, 0, sizeof(msg));
    msg.addr = devp->client->addr;
    msg.buf = buff;
    msg.flags = I2C_M_RD;
    buff[0] = infop->reg&0x00FF;

    msg.len = 3;


    result = i2c_transfer(devp->client->adapter, &msg, 1);
    if(result == 0)
    {
        infop->value = buff[1]<<8|buff[2];
        printk("buff[1]=0x%x buff[2]=0x%x\n", buff[1], buff[2]);
    }

    return result;
}

/*
board ioctl function: in this function, we can add any sub sytem ioctl case .
*/
static long virtual_i2c_dev_ctrl_ioctl(struct file *filep, unsigned int cmd, unsigned long args)
{
    int ret = 0;
    virtual_dev_t *devp = filep->private_data;
    virtual_i2c_dev_data_t common_cmd;

    memset(&common_cmd, 0, sizeof(virtual_i2c_dev_data_t));
    switch(cmd)
    {
        case VIRTUAL_I2C_READ_VALUE:
            if(copy_from_user(&common_cmd, (void __user *) args, sizeof(virtual_i2c_dev_data_t)))
            {
                printk("%s:%d\n", __FUNCTION__, __LINE__);
                return -EFAULT;
            }
            ret = virtual_i2c_dev_get_value(devp, &common_cmd);
            if(ret != 0)
                return ret;

            if(copy_to_user((void __user *) args, &common_cmd, sizeof(virtual_i2c_dev_data_t)))
            {
                printk("%s:%d\n", __FUNCTION__, __LINE__);
                return -EFAULT;
            }
            break;
        case VIRTUAL_I2C_WRITE_VALUE:
            if(copy_from_user(&common_cmd, (void __user *) args, sizeof(virtual_i2c_dev_data_t)))
            {
                printk("%s:%d\n", __FUNCTION__, __LINE__);
                return -EFAULT;
            }
            ret = virtual_i2c_dev_set_value(devp, &common_cmd);
            break;
        default:
            return -EINVAL;
    }

    return ret;
}

static struct  file_operations virtual_i2c_dev_ops =
{
    .owner = THIS_MODULE,
    .open = virtual_i2c_dev_ctrl_open,
    .release = virtual_i2c_dev_ctrl_close,
    .unlocked_ioctl = virtual_i2c_dev_ctrl_ioctl
};




static int virtual_i2c_dev_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    
    int result = -ENODEV;
    virtual_dev_t  *devp;

    printk("%s:%d\n", __FUNCTION__, __LINE__);

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
        return -EIO;

    
    devp = kzalloc(sizeof(virtual_dev_t),GFP_KERNEL);
    if(devp == NULL)
    {
        printk(KERN_ERR"%s:Init:malloc failed\n", __FUNCTION__);
        return -ENOSPC;
    }
    memcpy(devp->name, DEV_NAME, sizeof(DEV_NAME));

    
    result = alloc_chrdev_region(&(devp->dev_num),0,1,devp->name);
    if(result != 0)
    {
        printk(KERN_ERR"%s: can not alloc dev_num\n", __FUNCTION__);
        goto out_err_mem;
    }

    cdev_init(&devp->chr_dev, &virtual_i2c_dev_ops);
    cdev_add(&devp->chr_dev,devp->dev_num, 1);
    class_ptr = class_create(THIS_MODULE, "virtual_i2c_dev");
    if(IS_ERR(class_ptr))
    {
        printk(KERN_ERR "%s:class_create() failed for dev_ctrl_class\n", __FUNCTION__);
        goto out_err_class;
    }

    devp->base_dev = device_create(class_ptr, NULL, devp->dev_num, devp, devp->name);
    if(devp->base_dev == NULL)
    {
        printk(KERN_ERR"%s:device_create failed \n", __FUNCTION__);
        goto out_err_device;
    }
    devp->client = client;
    i2c_set_clientdata(client, devp);

    printk("virtual i2c dev addr=0x%x\n",  client->addr);

    return 0;

out_err_device:
    class_destroy(class_ptr);
    class_ptr = NULL;
out_err_class:
    unregister_chrdev_region(devp->dev_num,1);
out_err_mem:
    kfree(devp);
    devp = NULL;
    return result;


}

static int virtual_i2c_dev_remove(struct i2c_client *client)
{
    virtual_dev_t *devp = i2c_get_clientdata(client);

    if(devp == NULL)
        return -EINVAL;

    device_del(devp->base_dev);
    cdev_del(&devp->chr_dev);
    unregister_chrdev_region(devp->dev_num,1);
    devp->base_dev = NULL;
    kfree(devp);
    devp = NULL;
    
    class_destroy(class_ptr);
    class_ptr = NULL;

    return 0;
}

static const struct of_device_id virtual_i2c_dev_of_match[] = {
    {
        .compatible = "virtual,virtual_i2c_dev0",
    },    
    {},
};

MODULE_DEVICE_TABLE(of, virtual_i2c_dev_of_match);

static const struct i2c_device_id virtual_i2c_dev_id[] = {
    {"virtual_i2c_dev0", 0},
    {},
};

static struct i2c_driver virtual_i2c_dev_driver = {
    .driver = {
        .name	= "virtual_i2c_driver",
        .of_match_table = of_match_ptr(virtual_i2c_dev_of_match),
    },
    .probe      = virtual_i2c_dev_probe,
    .remove     = virtual_i2c_dev_remove,
    .id_table   = virtual_i2c_dev_id,
};

module_i2c_driver(virtual_i2c_dev_driver);

MODULE_DESCRIPTION("virtual i2c Device Drivers"); 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("jerry_chg");
