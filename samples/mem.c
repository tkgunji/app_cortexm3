#include <stdio.h>
#include <sys/sysinfo.h>

int main(void)
{
  struct sysinfo info;

  sysinfo(&info);

  printf("mem_unit:%d\n", info.mem_unit); 
  printf("totalram:%ld\n", info.totalram);
  printf("freeram:%ld\n", info.freeram);
  printf("bufferram:%ld\n", info.bufferram);
  printf("totalswap:%ld\n", info.totalswap);
  printf("freeswap:%ld\n", info.freeswap);

  return 0;
}
