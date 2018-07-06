/******************************************************************
 * Visor Data Logger
 * Accelerometer and Magnetometer Example
 * Developed by Sevun Scientific, Inc.
 * http://sevunscientific.com
 * *****************************************************************
 *
 *    _____/\\\\\\\\\\\_______/\\\\\\\\\\\____/\\\\\\\\\\\_
 *     ___/\\\/////////\\\___/\\\/////////\\\_\/////\\\///__
 *      __\//\\\______\///___\//\\\______\///______\/\\\_____
 *       ___\////\\\___________\////\\\_____________\/\\\_____
 *        ______\////\\\___________\////\\\__________\/\\\_____
 *         _________\////\\\___________\////\\\_______\/\\\_____
 *          __/\\\______\//\\\___/\\\______\//\\\______\/\\\_____
 *           _\///\\\\\\\\\\\/___\///\\\\\\\\\\\/____/\\\\\\\\\\\_
 *            ___\///////////_______\///////////_____\///////////__
 *
 * *****************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include "ag/fxos8700cq_linaro.h"

// Define FXOS8700CQ I2C address, determined by PCB layout with pins SA0=1, SA1=0
#define AG_SLAVE_ADDR       0x1D

//*****************************************************************************
// I2C Functions
//*****************************************************************************

// see fxas8700cq.c for accelerometer and magnetometer functions


int main()
{
    //*****************************************************************************
    // Main Code
    //*****************************************************************************

    I2CAGReceive(0x1d, 0x0d, 0, 0); 
    

  //
  // open file handle to bus
  //
  int file;
  int adapter_nr = 1; /* probably dynamically determined */
  char filename[20];

  snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
  file = open(filename, O_RDWR);
  if (file < 0) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
    printf("open failed.\n");
    return 1;
  }

  //
  // set address of device
  //
  int addr = AG_SLAVE_ADDR; /* The I2C address */
  if (ioctl(file, I2C_SLAVE, addr) < 0) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
    printf("address failed.\n");
    return 1;
  }

  //
  // try to read
  //
  __u8 reg = 0x0d; /* Device register to access */
  __u8 res;
  char buf[10];
  /* Using SMBus commands */
  res = i2c_smbus_read_byte_data(file, reg);
  if (res < 0) {
    /* ERROR HANDLING: i2c transaction failed */
    printf("read failed.\n");
    return 1;
  } else {
    /* res contains the read word */
    printf("WHO_AM_I = 0x%x\n", res);
  }

  //
  // try to read/write/read
  //
  // read 1
  reg = 0x11; /* Device register to access */
  /* Using SMBus commands */
  res = i2c_smbus_read_byte_data(file, reg);
  if (res < 0) {
    /* ERROR HANDLING: i2c transaction failed */
    printf("read1 failed.\n");
    return 1;
  } else {
    /* res contains the read word */
    printf("PL_CFG = 0x%x\n", res);
  }
  
  // write 1
  __u8 val = 0xC0;
  reg = 0x11; /* Device register to access */
  res = i2c_smbus_write_byte_data (file, reg, val);
  if (res < 0) {
    /* ERROR HANDLING: i2c transaction failed */
    printf("write1 failed.\n");
    return 1;
  } else {
    //all good
  }
  
  // read 2
  reg = 0x11; /* Device register to access */
  /* Using SMBus commands */
  res = i2c_smbus_read_byte_data(file, reg);
  if (res < 0) {
    /* ERROR HANDLING: i2c transaction failed */
    printf("read2 failed.\n");
    return 1;
  } else {
    /* res contains the read word */
    printf("PL_CFG = 0x%x\n", res);
  }

  //
  // close file and exit
  //

  // printf() displays the string inside quotation
  printf("Hello, World!\n");
  close(file);
  return 0;
}
