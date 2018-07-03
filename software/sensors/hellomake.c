#include <stdio.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>

// be sure to 'sudo apt-get install libi2c-dev' 
// *after* i2c-tools or whatever got you the command line interface.
// this gets you an updated i2c-dev.h

int main()
{

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
  int addr = 0x1d; /* The I2C address */
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