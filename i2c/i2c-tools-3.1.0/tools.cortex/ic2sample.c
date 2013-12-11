#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{

  char filename[20]="/dev/i2c";
  int file; 
  int res;
  
  file = open(filename,O_RDWR); //open the file. (appropriate nod in initramfs required)
  if(file<0){
    fprintf(stderr, "Error: Could not open file \n");
    return 0;
  }

  if(ioctl(file,I2C_SLAVE,0x3E) < 0){
    fprintf(stderr, "Error : ioctl I2C_SLAVE failed\n ");
    return 0;
  }

  printf("Done \n");
  res=i2c_smbus_read_byte_data(file,0x00);
  if(res<0){
    fprintf(stderr, "Error : i2c_smbus_read_byte_data() failed\n ");
    return 0;
  }else{
    printf("revision nr = %x\n", res);
  }


  return 0;
  

}
