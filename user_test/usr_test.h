#ifndef USR_TEST_H_
#define USR_TEST_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#define DEV_NAME "/dev/virtual_i2cdev"


typedef struct __virtual_i2c_dev_data_t
{
    uint8_t reg;
    uint16_t value;
}virtual_i2c_dev_data_t;

#define DEVICE_MAGIC 'b'
#define VIRTUAL_I2C_READ_VALUE            _IOR(DEVICE_MAGIC, 0x01, virtual_i2c_dev_data_t)      /*read*/
#define VIRTUAL_I2C_WRITE_VALUE            _IOW(DEVICE_MAGIC, 0x01, virtual_i2c_dev_data_t)      /*write*/


#endif
