#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{

  int file1; 
  int file2; 
  char res[10];
  int new_res;
  int padc = 0;
  int tadc = 0;
  float temp = 0.0;
  float rh = 0.0;

  int ievent = 0;
  int nevent = 100;
  
  while(ievent<nevent){

    file1 = open("/sys/class/i2c-adapter/i2c-0/0-0040/temperature",O_RDONLY); //open the file. (appropriate nod in initramfs required)
    if(file1<0){
      fprintf(stderr, "Error: Could not open file\n");
      return 0;
    }
    
    file2 = open("/sys/class/i2c-adapter/i2c-0/0-0040/humidity",O_RDONLY); //open the file. (appropriate nod in initramfs required)
    if(file2<0){
      fprintf(stderr, "Error: Could not open file \n");
      return 0;
    }


    sleep(5);
    read(file1, res, 5);
    padc = atoi(res);
    temp = -46.85 + 175.72*padc/65536;
    
    printf("%s --> Padc = 0x%x, temperature = %f (deg)\n", res, padc, temp);    
    
    
    read(file2, &res, 5);
    padc = atoi(res);
    rh = -6 + 125.0*padc/65536;
    printf("Padc = 0x%x, relative humidity = %f [Percent]\n", padc, rh);    

    close(file1);
    close(file2);



    ievent ++;
  }
  return 0;
}