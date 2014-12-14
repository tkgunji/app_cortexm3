/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 *
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

struct spi_ioc_transfer xfer[2];
char buf[32];
char buf2[32];

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))


#define READ_ARRAY_OPCODE   0x03
#define DEVICE_ID_READ      0x9F


#define WRITE_ENABLE_CMD    0x06
#define WRITE_DISABLE_CMD   0x4
#define PROGRAM_PAGE_CMD    0x02
#define WRITE_STATUS1_OPCODE    0x01
#define CHIP_ERASE_OPCODE   0x60
#define ERASE_4K_BLOCK_OPCODE   0x21
#define ERASE_32K_BLOCK_OPCODE  0x52
#define ERASE_64K_BLOCK_OPCODE  0xD8
#define READ_STATUS         0x05

#define DMA_TRANSFER_SIZE    32u
#define READY_BIT_MASK      0x01

#define UNPROTECT_SECTOR_OPCODE     0x39

#define DONT_CARE       0

#define NB_BYTES_PER_PAGE   256


static void pabort(const char *s)
{
  perror(s);
  abort();
}

static const char *device = "/dev/spidev0.0";
static uint8_t mode = 3;
static uint8_t bits = 8;
static uint32_t speed = 10000000;
static uint16_t delay;

void print_status_reg(int fd)
{
  int status;
  buf[0] = READ_STATUS;

  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 1;
  
  xfer[1].rx_buf = (unsigned long) buf2;
  xfer[1].len = 1; /* Length of Data to read */
  
  status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
    
  if (status < 0)
    {
      perror("SPI_IOC_MESSAGE");
      return;
    }

  printf("status-reg 1 %d %02x \n", status, buf2[0]);
}

void wait_ready(int fd)
{
  int status;
  uint8_t ready_bit;
  buf[0] = READ_STATUS;

  do {
    xfer[0].tx_buf = (unsigned long)buf;
    xfer[0].len = 1;

    xfer[1].rx_buf = (unsigned long) buf2;
    xfer[1].len = 1; /* Length of Data to read */

    status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
    
    if (status < 0)
      {
	perror("SPI_IOC_MESSAGE");
	return;
      }
    ready_bit = buf2[0];
    ready_bit = ready_bit & READY_BIT_MASK;
  } while(ready_bit == 1);
//printf("*** %d %02x \n", status, buf2[0]);
//  printf("*** %d %02x \n", status, buf2[1]);

}

void spi_write(int fd, int addr,  unsigned char value[], int nbytes)
{

  int status; 
  int index=0;


  //memset(buf, 0, sizeof buf);


  //write enable 
  buf[0] = WRITE_ENABLE_CMD; //0x06;
  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 1;
  status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
  if (status < 0)
  {
    perror("SPI_IOC_MESSAGE");
    return;
  }


  //write data 
  buf[0] = PROGRAM_PAGE_CMD; //0x02;
  buf[1] = ((addr >> 16) & 0xFF);
  buf[2] = ((addr >> 8) & 0xFF);;
  buf[3] = (addr & 0xFF);

  for(index=0; index<nbytes; index++){
    buf[4+index] = value[index];
  }
  //buf[4]  = value;


  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = nbytes+4; /* Length of  command to write*/

  status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
  if (status < 0)
    {
      perror("SPI_IOC_MESSAGE");
      return;
    }

  for(index=0; index<nbytes; index++){
    printf("%d : value[0x%x] = set: %02x \n", status, index, value[index]);
  }  
  
  //printf("ret: %02x %02x %02x %02x\n", buf2[0], buf2[1], buf2[2], buf2[3]);
 
}


void spi_erase_4k(int fd, int addr)
{

  int status; 
  int index=0;

  //write enable 
  wait_ready(fd);
  buf[0] = WRITE_ENABLE_CMD; //0x06;
  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 1;
  status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
  if (status < 0)
  {
    perror("SPI_IOC_MESSAGE");
    return;
  }

  wait_ready(fd);
  //erase data 
  buf[0] = ERASE_4K_BLOCK_OPCODE; //0x20;
  buf[1] = ((addr >> 16) & 0xFF);
  buf[2] = ((addr >> 8) & 0xFF);;
  buf[3] = (addr & 0xFF);

  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 4; /* Length of  command to write*/
  
  status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
  if (status < 0)
    {
      perror("SPI_IOC_MESSAGE");
      return;
    }
}

//////////
// Read n bytes from the 2 bytes add1 add2 address
//////////
 
char * spi_read(int fd, int addr, int nbytes)
{
  int status; 
  int index=0;
  //memset(buf, 0, sizeof buf);
  buf[0] = READ_ARRAY_OPCODE; //0x03;
  buf[1] = ((addr >> 16) & 0xFF);
  buf[2] = ((addr >> 8) & 0xFF);
  buf[3] = (addr & 0xFF);

  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 4; /* Length of  command to write*/

  xfer[1].rx_buf = (unsigned long) buf2;
  xfer[1].len = nbytes; /* Length of Data to read */

  status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
  if (status < 0)
    {
      perror("SPI_IOC_MESSAGE");
      return;
    }
  //printf("env: %02x %02x %02x\n", buf[0], buf[1], buf[2]);
  for(index=0; index<nbytes; index++){
    printf("%d : buf2[%d] = ret: %02x \n", status, index, buf2[index]);
  }

  return buf2;
}
 

void print_usage(const char *prog)
{
  printf("Usage: %s [-DsbdlHOLC3]\n", prog);
  puts("  -D --device   device to use (default /dev/spidev1.1)\n"
            "  -s --speed    max speed (Hz)\n"
            "  -d --delay    delay (usec)\n"
            "  -b --bpw      bits per word \n"
            "  -l --loop     loopback\n"
            "  -H --cpha     clock phase\n"
            "  -O --cpol     clock polarity\n"
            "  -L --lsb      least significant bit first\n"
            "  -C --cs-high  chip select active high\n"
       "  -3 --3wire    SI/SO signals shared\n");
  exit(1);
}

void parse_opts(int argc, char *argv[])
{
  while (1) {
    static const struct option lopts[] = {
      { "device",  1, 0, 'D' },
      { "speed",   1, 0, 's' },
      { "delay",   1, 0, 'd' },
      { "bpw",     1, 0, 'b' },
      { "loop",    0, 0, 'l' },
      { "cpha",    0, 0, 'H' },
      { "cpol",    0, 0, 'O' },
      { "lsb",     0, 0, 'L' },
      { "cs-high", 0, 0, 'C' },
      { "3wire",   0, 0, '3' },
      { NULL, 0, 0, 0 },
    };
    int c;

    c = getopt_long(argc, argv, "D:s:d:b:lHOLC3", lopts, NULL);

    if (c == -1)
      break;

    switch (c) {
    case 'D':
      device = optarg;
      break;
    case 's':
      speed = atoi(optarg);
      break;
    case 'd':
      delay = atoi(optarg);
      break;
    case 'b':
      bits = atoi(optarg);
      break;
    case 'l':
      mode |= SPI_LOOP;
      break;
    case 'H':
      mode |= SPI_CPHA;
      break;
    case 'O':
      mode |= SPI_CPOL;
      break;
    case 'L':
      mode |= SPI_LSB_FIRST;
      break;
    case 'C':
      mode |= SPI_CS_HIGH;
      break;
    case '3':
      mode |= SPI_3WIRE;
      break;
    default:
      print_usage(argv[0]);
      break;
    }
  }
}

int main(int argc, char *argv[])
{
  int ret = 0;
  int fd;
  int addr = 0x0;
  char *buffer;
  int iadd=0;

  int nbytes = 4;
  unsigned char data[] = {
    0x4e, 0x41, 0x42, 0x43, 0x44, 0x45,
    0x46, 0x4e, 0x44, 0x44, 0x44, 0x95,
    0x4F, 0x4e, 0x4F, 0x4F, 0x5F, 0x6F,
    0x4F, 0x4e, 0x4F, 0x4F, 0x5F, 0x6F,
    0x4F, 0x4e, 0x4F, 0x4F, 0x5F, 0x6F,
    0xDE, 0xAD, 0xBE, 0xEF, 0xBA, 0xAD,
    0xF0, 0x0D,
  };

  parse_opts(argc, argv);

  fd = open(device, O_RDWR);
  if (fd < 0)
    pabort("can't open device");

  /*
   * spi mode
   */
  ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
  if (ret == -1)
    pabort("can't set spi mode");


  ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
  if (ret == -1)
    pabort("can't get spi mode");

  /*
   * bits per word
   */
  ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
  if (ret == -1)
    pabort("can't set bits per word");

  ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
  if (ret == -1)
    pabort("can't get bits per word");

  /*
   * max speed hz
   */
  ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
  if (ret == -1)
    pabort("can't set max speed hz");

  ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
  if (ret == -1)
    pabort("can't get max speed hz");

  printf("spi mode: %d\n", mode);
  printf("bits per word: %d\n", bits);
  printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);



  //spi_erase_4k(fd, 0x410000); ///this address should be base-address of the Flash block
  //buffer=(char *)spi_read(fd, addr, nbytes);  
  //spi_write(fd, addr, data, nbytes);  
  buffer=(char *)spi_read(fd, addr, nbytes);  

  close(fd);

  return ret;
}


