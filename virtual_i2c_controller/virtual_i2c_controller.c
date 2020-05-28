#include "virtual_i2c_controller.h"




static void virtual_i2c_adapter_dev_release(struct device *dev)
{

}

static struct platform_device gvirtual_i2c_master_platform_device = {
    .name = "virtual_i2c_adapter_dev",
    .id = 1,
    .dev =
    {
        .release = virtual_i2c_adapter_dev_release,
    }
};


static int virtual_i2c_master_xfer (struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
    int ret = 0;
    int i = 0;
    virtual_i2c_bus_t *virtual_i2c_bus_ptr = container_of(adap, virtual_i2c_bus_t, adapter);
    virtual_i2c_dev_info_t *vir_i2c_dev_infop = NULL;
    uint8_t reg_addr = 0;
    
    for(i = 0; i < num; i++)
    {
        /*reg:8bits value:16bits*/
        if(msgs[i].len < 3)
            return -EINVAL;
        list_for_each_entry(vir_i2c_dev_infop, &(virtual_i2c_bus_ptr->virtual_dev_info.list), node)
        {
            if(vir_i2c_dev_infop->addr == msgs[i].addr)
            {
                if(msgs[i].flags &I2C_M_RD)
                {
                    /*read...*/
                    reg_addr = msgs[i].buf[0];
                    if(reg_addr >= VIRTUAL_I2C_DEV_REGS_NUM)
                        return -EINVAL;
                    msgs[i].buf[1] = vir_i2c_dev_infop->regs[reg_addr]>>8;
                    msgs[i].buf[2] = vir_i2c_dev_infop->regs[reg_addr]&0x00FF;
                }
                else
                {
                    /*write...*/
                    reg_addr = msgs[i].buf[0];
                    if(reg_addr >= VIRTUAL_I2C_DEV_REGS_NUM)
                        return -EINVAL;
                    vir_i2c_dev_infop->regs[reg_addr] = msgs[i].buf[1]<<8 |msgs[i].buf[2];
                }
            }
        }

    }

    return ret;
}

static u32 virtual_i2c_functionality (struct i2c_adapter * adap)
{

    return I2C_FUNC_I2C;

}

static struct i2c_algorithm virtual_i2c_algo = 
{
    .master_xfer = virtual_i2c_master_xfer,
    .functionality = virtual_i2c_functionality
};


static struct i2c_board_info virtual_i2c_board_0 = {
    .type = "virtual_i2c_dev0",
    .addr = 0x30
};

static int virtual_i2c_adapter_platform_probe(struct platform_device *platform_dev)
{
    struct i2c_client * i2c_dev;
    virtual_i2c_bus_t *virtual_i2c_bus_ptr = NULL;


    virtual_i2c_bus_ptr = devm_kzalloc(&platform_dev->dev, sizeof(virtual_i2c_bus_t), GFP_KERNEL);
    if (!virtual_i2c_bus_ptr) 
    {
        printk("%s(): unable to alloc i2c adapter \n", __func__);
        return -EINVAL;
    }

    virtual_i2c_bus_ptr->adapter.owner = THIS_MODULE;
    memcpy(virtual_i2c_bus_ptr->adapter.name, "virtual_i2c_adapter", sizeof(virtual_i2c_bus_ptr->adapter.name));
    virtual_i2c_bus_ptr->adapter.algo = &virtual_i2c_algo;
    virtual_i2c_bus_ptr->adapter.class = I2C_CLASS_HWMON;
    virtual_i2c_bus_ptr->adapter.dev.parent = &platform_dev->dev;
#ifdef CONFIG_OF
    virtual_i2c_bus_ptr->adapter.dev.of_node = of_node_get(platform_dev->dev.of_node);
#endif


    INIT_LIST_HEAD(&virtual_i2c_bus_ptr->virtual_dev_info.list);
    
    mutex_init(&virtual_i2c_bus_ptr->virtual_dev_info.virtual_i2c_mutex);
    if (i2c_add_adapter(&virtual_i2c_bus_ptr->adapter)) 
    {
        return -EINVAL;
    }

    platform_set_drvdata(platform_dev, virtual_i2c_bus_ptr);

    if ((i2c_dev = i2c_new_device(&virtual_i2c_bus_ptr->adapter, &virtual_i2c_board_0)) == NULL)
    {
        printk("%s:%d add i2c device failed\n", __FUNCTION__, __LINE__);

        return -EINVAL;
    }
    else
    {
        virtual_i2c_dev_info_t *virtual_devp = devm_kzalloc(&platform_dev->dev, sizeof(virtual_i2c_dev_info_t), GFP_KERNEL);
        if(NULL == virtual_devp)
        {
            i2c_unregister_device(i2c_dev);
            return -EINVAL;
        }
        INIT_LIST_HEAD(&virtual_devp->node);
        virtual_devp->addr = i2c_dev->addr;
        list_add_tail(&(virtual_devp->node), &(virtual_i2c_bus_ptr->virtual_dev_info.list));
    }



    printk("%s:%d\n", __FUNCTION__, __LINE__);
    return 0;
} 

static int virtual_i2c_adapter_platform_remove(struct platform_device *platform_dev)
{
    virtual_i2c_bus_t *virtual_i2c_bus_ptr = platform_get_drvdata(platform_dev);


    printk("%s:%d\n", __FUNCTION__, __LINE__);

    i2c_del_adapter(&virtual_i2c_bus_ptr->adapter);
    printk("%s:%d\n", __FUNCTION__, __LINE__);
    return 0;
}


static struct platform_driver virtual_i2c_adapter_platform_driver = {
    .driver = {
        .name = "virtual_i2c_adapter_dev",
        .owner = THIS_MODULE,
    },
    .probe = virtual_i2c_adapter_platform_probe,
    .remove = virtual_i2c_adapter_platform_remove,
};


static int __init virtual_i2c_adapter_init(void)
{
    int ret = 0;

    ret = platform_device_register(&gvirtual_i2c_master_platform_device);

    if(ret == 0)
    {
        ret = platform_driver_register(&virtual_i2c_adapter_platform_driver);
    }

    return ret;
}

static void __exit virtual_i2c_adapter_exit(void)
{
    printk("%s:%d\n", __FUNCTION__, __LINE__);
    platform_device_unregister(&gvirtual_i2c_master_platform_device);
    printk("%s:%d\n", __FUNCTION__, __LINE__);

    platform_driver_unregister(&virtual_i2c_adapter_platform_driver);
}



module_init(virtual_i2c_adapter_init);
module_exit(virtual_i2c_adapter_exit);
MODULE_DESCRIPTION("Virtual I2c Adapter Platform Device Drivers");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("jerry_chg");
