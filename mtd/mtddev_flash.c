#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>

int main()
{
  mtd_info_t mtd_info;           // the MTD structure
  erase_info_t ei;               // the erase block structure
  int i;
  int offset = 0x10;


  unsigned char data[20] = { 0xDE, 0xAD, 0xBE, 0xEF,  // our data to write
			     0xDE, 0xAD, 0xBE, 0xEF,
			     0xDE, 0xAD, 0xBE, 0xEF,
			     0xDE, 0xAD, 0xBE, 0xEF,
			     0xDE, 0xAD, 0xBE, 0xEF};
  unsigned char read_buf[20] = {0x00};                // empty array for reading

  int fd = open("/dev/mtd2", O_RDWR); // open the mtd device for reading and 
  // writing. Note you want mtd0 not mtdblock0
  // also you probably need to open permissions
  // to the dev (sudo chmod 777 /dev/mtd0)

  ioctl(fd, MEMGETINFO, &mtd_info);   // get the device info

  // dump it for a sanity check, should match what's in /proc/mtd
  printf("MTD Type: %x\nMTD total size: %x bytes\nMTD erase size: %x bytes\n",
         mtd_info.type, mtd_info.size, mtd_info.erasesize);


  ei.length = mtd_info.erasesize;   //set the erase block size
  for(ei.start = 0; ei.start < mtd_info.size; ei.start += ei.length)
    {
      ioctl(fd, MEMUNLOCK, &ei);
      // printf("Eraseing Block %#x\n", ei.start); // show the blocks erasing
      // warning, this prints a lot!
      ioctl(fd, MEMERASE, &ei);
    }    


  lseek(fd, 0, SEEK_SET);               // go to the first block
  read(fd, read_buf, sizeof(read_buf)); // read 20 bytes

  // sanity check, should be all 0xFF if erase worked
  for(i = 0; i<20; i++)
    printf("buf[%d] = 0x%02x\n", i, (unsigned int)read_buf[i]);

  lseek(fd, offset, SEEK_SET);        // go back to first block's start
  write(fd, data, sizeof(data)); // write our message

  lseek(fd, offset, SEEK_SET);              // go back to first block's start
  read(fd, read_buf, sizeof(read_buf));// read the data

  // sanity check, now you see the message we wrote!    
  for(i = 0; i<20; i++)
    printf("buf[%d] = 0x%02x\n", i, (unsigned int)read_buf[i]);


  close(fd);
  return 0;
}
