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
  float a0 = 0.0;
  float b1 = 0.0;
  float b2 = 0.0;
  float c12 = 0.0;
  float pcomp=0.0;
  float pressure = 0.0;
  
  file = open(filename,O_RDWR); //open the file. (appropriate nod in initramfs required)
  if(file<0){
    fprintf(stderr, "Error: Could not open file \n");
    return 0;
  }

  if(ioctl(file,I2C_SLAVE,0x60) < 0){
    fprintf(stderr, "Error : ioctl I2C_SLAVE failed\n ");
    return 0;
  }
  printf("ioctl done \n");
  res = i2c_smbus_write_byte_data(file, 0x12, 0x1);
  sleep(1);
  printf("Done \n");

  ///// read Padc
  res=i2c_smbus_read_word_data(file,0x00);
  if(res<0){
    fprintf(stderr, "Error : i2c_smbus_read_byte_data() failed\n ");
    return 0;
  }else{
    padc = (((res>>8) + (res<<8)&0x00ffff)) / 64;
    printf("Padc = 0x%x, %d\n", res, padc);
    
  }

  ///// read Tadc
  res=i2c_smbus_read_word_data(file,0x02);
  if(res<0){
    fprintf(stderr, "Error : i2c_smbus_read_byte_data() failed\n ");
    return 0;
  }else{
    tadc = (((res>>8) + (res<<8)&0x00ffff)) / 64;
    printf("Tadc = 0x%x, %d\n", res, tadc);
    
  }

  //// correction a0
  res=i2c_smbus_read_word_data(file,0x04);
  if(res<0){
    fprintf(stderr, "Error : i2c_smbus_read_byte_data() failed\n ");
    return 0;
  }else{
    new_res = (((res>>8) + (res<<8)&0x00ffff));
    /////////////
    int sign = (new_res & 0x8000 ) >> 15;
    a0 = (new_res & 0x7ff8 ) >> 3;
    a0 += ( ((new_res & 0x4) >>2) / 2.0 + 
	    ((new_res & 0x2) >>1) / 4.0 + 
	    ((new_res & 0x1)) / 8.0 );
    a0 = (1-2*sign)*a0;
    printf("a0 = 0x%x, %f\n", res, a0);
  }   


  //// correction b1
  res=i2c_smbus_read_word_data(file,0x06);
  if(res<0){
    fprintf(stderr, "Error : i2c_smbus_read_byte_data() failed\n ");
    return 0;
  }else{
    new_res = (((res>>8) + (res<<8)&0x00ffff));
    /////////////
    int sign = (new_res & 0x8000 ) >> 15;
    new_res = ~new_res;
    new_res += 1;
    b1 = (new_res & 0x6000 ) >> 13;
    b1 += ( ((new_res & 0x1000) >>12) / 2.0 + 
	    ((new_res & 0x0800) >>11) / 4.0 + 
	    ((new_res & 0x0400) >>10) / 8.0 + 
	    ((new_res & 0x0200) >> 9) / 16.0 + 
	    ((new_res & 0x0100) >> 8) / 32.0 + 
	    ((new_res & 0x0080) >> 7) / 64.0 + 
	    ((new_res & 0x0040) >> 6) / 128.0 + 
	    ((new_res & 0x0020) >> 5) / 256.0 + 
	    ((new_res & 0x0010) >> 4) / 512.0 + 
	    ((new_res & 0x0008) >> 3) / 1024.0 + 
	    ((new_res & 0x0004) >> 2) / 2048.0 + 
	    ((new_res & 0x0002) >> 1) / 4096.0 + 
	    ((new_res & 0x0001)  ) / 8192.0); 
    b1 = (1-2*sign)*b1;
    printf("b1 = 0x%x, %f\n", res, b1);
  }

  //// correction b2
  res=i2c_smbus_read_word_data(file,0x08);
  if(res<0){
    fprintf(stderr, "Error : i2c_smbus_read_byte_data() failed\n ");
    return 0;
  }else{
    new_res = (((res>>8) + (res<<8)&0x00ffff));
    /////////////
    int sign = (new_res & 0x8000 ) >> 15;
    new_res = ~new_res;
    new_res += 1;
    b2 = (new_res & 0x4000 ) >> 14;
    b2 += (
	   ((new_res & 0x2000) >>13) / 2.0 + 
	   ((new_res & 0x1000) >>12) / 4.0 + 
	   ((new_res & 0x0800) >>11) / 8.0 + 
	   ((new_res & 0x0400) >>10) / 16.0 + 
	   ((new_res & 0x0200) >> 9) / 32.0 + 
	   ((new_res & 0x0100) >> 8) / 64.0 + 
	   ((new_res & 0x0080) >> 7) / 128.0 + 
	   ((new_res & 0x0040) >> 6) / 256.0 + 
	   ((new_res & 0x0020) >> 5) / 512.0 + 
	   ((new_res & 0x0010) >> 4) / 1024.0 + 
	   ((new_res & 0x0008) >> 3) / 2048.0 + 
	   ((new_res & 0x0004) >> 2) / 4096.0 + 
	   ((new_res & 0x0002) >> 1) / 8192.0 + 
	   ((new_res & 0x0001)  ) / 16384.0); 
    b2 = (1-2*sign)*b2;
    printf("b2 = 0x%x, %f\n", res, b2);
  }

  //// correction c12
  res=i2c_smbus_read_word_data(file,0x0a);
  if(res<0){
    fprintf(stderr, "Error : i2c_smbus_read_byte_data() failed\n ");
    return 0;
  }else{
    new_res = (((res>>8) + (res<<8)&0x00ffff));
    /////////////
    int sign = (new_res & 0x8000 ) >> 15;
    new_res = new_res >> 2;
    /////////////
    c12 = (
	   ((new_res & 0x1000) >>12) / 1024.0 + 
	   ((new_res & 0x0800) >>11) / 2048.0 + 
	   ((new_res & 0x0400) >>10) / 4096.0 + 
	   ((new_res & 0x0200) >> 9) / 8192.0 + 
	   ((new_res & 0x0100) >> 8) / 16384.0 + 
	   ((new_res & 0x0080) >> 7) / 32768.0 + 
	   ((new_res & 0x0040) >> 6) / 65536.0 +  
	   ((new_res & 0x0020) >> 5) / 131072.0 + 
	   ((new_res & 0x0010) >> 4) / 262144.0 +  
	   ((new_res & 0x0008) >> 3) / 524288.0 + 
	   ((new_res & 0x0004) >> 2) / 1048576.0 + 
	   ((new_res & 0x0002) >> 1) / 2097152.0 + 
	   ((new_res & 0x0001)  ) /  4194304);
    c12 = (1-2*sign)*c12;
    printf("c12 = 0x%x, %f\n", res, c12);
  }


  
  pcomp = a0 + (b1+c12*tadc)*padc+b2*tadc;
  pressure = pcomp*(115.0-50.0)/1023.0+50;

  printf(" Pressure = %f, %f (kPa)\n", pcomp, pressure);

  return 0;
  

}
