#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
//#include <linux/i2c.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{

  char filename[20]="/dev/i2c-0";
  int file; 
  int res;
  int new_res;
  int padc = 0;
  int tadc = 0;
  float temp = 0.0;
  float rh = 0.0;

  file = open(filename,O_RDWR); //open the file. (appropriate nod in initramfs required)
  if(file<0){
    fprintf(stderr, "Error: Could not open file \n");
    return 0;
  }

  if(ioctl(file,I2C_SLAVE,0x40) < 0){
    fprintf(stderr, "Error : ioctl I2C_SLAVE failed\n ");
    return 0;
  }
  printf("ioctl done \n");
  res = i2c_smbus_write_byte_data(file, 0x40, 0x3e);
  sleep(1);
  printf("Done \n");

  ///// read Padc
  res=i2c_smbus_read_word_data(file,0x03);
  if(res<0){
    fprintf(stderr, "Error : i2c_smbus_read_byte_data() failed\n ");
    return 0;
  }else{
    padc = (((res>>8) + (res<<8)&0x00ffff));
    temp = -46.85 + 175.72*padc/65536;
    printf("Padc = 0x%x, temperature = %f (deg)\n", padc, temp);    
  }
  //// read R
  res=i2c_smbus_read_word_data(file,0x05);
  if(res<0){
    fprintf(stderr, "Error : i2c_smbus_read_byte_data() failed\n ");
    return 0;
  }else{
    padc = (((res>>8) + (res<<8)&0x00ffff));
    rh = -6 + 125.0*padc/65536;
    printf("Padc = 0x%x, relative humidity = %f [Percent]\n", padc, rh);    
  }


  return 0;
}
