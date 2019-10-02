/*
 * iio - AD9250 IIO FMC ADC streaming
 * https://github.com/analogdevicesinc/libiio/blob/master/examples/ad9361-iiostream.c
 * Copyright (C) 2014 IABG mbH
 * Author: Michael Feilen <feilen_at_iabg.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * Max aquisition continuous time is ~ 20 ms, 5 blocks
 *
 **/

#include <stdbool.h>
//#include <stdint.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
//#include <sys/stat.h>
#include <unistd.h>

#include <gpiod.h>
#include <iio.h>

/* helper macros */
#define MHZ(x) ((long long)(x * 1000000.0 + .5))
/*#define GHZ(x) ((long long)(x*1000000000.0 + .5))*/

//#define GPIO_NUM_O_LINES 14
//#define GPIO_LINE_OFFSET 18
#define N_CHAN 2
#define N_BLOCKS                                                               \
  2 //  Number of RX buffers to save, 32ms, Max 16.5 good acquisition
#define GPIO_CHIP_NAME "/dev/gpiochip0"
#define GPIO_CONSUMER "gpiod-consumer"
#define GPIO_MAX_LINES 64
#define TRIG_EN_OFF 36
#define TRIG_REG_ADD_OFF 11
#define TRIG_REG_WRT_OFF 13
#define TRIG_REG_VAL_OFF 40

#define GPIO_BASE_ADDRESS 0x40000000
#define GPIO_0_DATA_OFFSET 0
#define GPIO_DIRECTION_OFFSET 4
#define GPIO_1_DATA_OFFSET 8

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
/*
   line   9:      unnamed       unused  output  active-high Trigger active High
   line  10:      unnamed       unused   input  active-high
   line  11:      unnamed       unused  output  active-high Address Line
   line  12:      unnamed       unused  output  active-high Address Line
   line  13:      unnamed       unused  output  active-high
   line  32:      unnamed "sysref-enable" output active-high [used]

   line  36:      unnamed  Trigger enable bit
   Lines 40-55 Trigger Value
   */
#define ASSERT(expr)                                                           \
  {                                                                            \
    if (!(expr)) {                                                             \
      (void)fprintf(stderr, "assertion failed (%s:%d)\n", __FILE__, __LINE__); \
      (void)abort();                                                           \
    }                                                                          \
  }

int memfd;
void *mapped_base, *mapped_dev_base;

static bool stop;
/* cleanup and exit */

static void handle_sig(int sig) {
  printf("Waiting for process to finish...\n");
  stop = true;
}

/* Since linux 4.8 the GPIO sysfs interface is deprecated. User space should use
   the character device instead. This library encapsulates the ioctl calls and
   data structures behind a straightforward API.
   */

int set_multiple_gpio(unsigned int offset, unsigned int width, int value) {
  int rv;
  unsigned int gpio_offsets[32];
  int gpio_values[32];
  if ((offset + width) > GPIO_MAX_LINES)
    return -1;
  for (int i = 0; i < width; i++) {
    gpio_offsets[i] = offset + i;
    gpio_values[i] = ((value >> i) & 0x1);
  }
  rv = gpiod_ctxless_set_value_multiple(GPIO_CHIP_NAME, gpio_offsets,
                                        gpio_values, width, false,
                                        GPIO_CONSUMER, NULL, NULL);
  return rv;
}
int get_multiple_gpio(unsigned int offset, unsigned int width, int *value) {
  int rv;
  int data = 0;
  unsigned int gpio_offsets[32];
  int gpio_values[32];
  if ((offset + width) > GPIO_MAX_LINES)
    return -1;
  for (int i = 0; i < width; i++)
    gpio_offsets[i] = offset + i;

  rv = gpiod_ctxless_get_value_multiple(
      GPIO_CHIP_NAME, gpio_offsets, gpio_values, width, false, GPIO_CONSUMER);
  for (int i = 0; i < width; i++)
    data |= ((gpio_values[i]) & 0x1) << i;
  *value = data;
  return rv;
}
int write_trigger_reg(unsigned int reg, int value) {
  int rv;
  rv = set_multiple_gpio(TRIG_REG_ADD_OFF, 2, reg);
  rv = set_multiple_gpio(TRIG_REG_VAL_OFF, 16, value); // Lines 40-55
  rv = gpiod_ctxless_set_value(GPIO_CHIP_NAME, TRIG_REG_WRT_OFF, 1, false,
                               GPIO_CONSUMER, NULL, NULL);
  rv = gpiod_ctxless_set_value(GPIO_CHIP_NAME, TRIG_REG_WRT_OFF, 0, false,
                               GPIO_CONSUMER, NULL, NULL);
  rv = gpiod_ctxless_set_value(GPIO_CHIP_NAME, 12, 1, false, GPIO_CONSUMER,
                               NULL, NULL);
  return rv;
}

int mmap_gpio_mem() {
  off_t dev_base = GPIO_BASE_ADDRESS;
  memfd = open("/dev/mem", O_RDWR | O_SYNC);
  if (memfd == -1) {
    printf("Can't open /dev/mem.\n");
    return -1;
  }
  printf("/dev/mem opened.\n");
  // Map one page of memory into user space such that the device is in that
  // page, but it may not be at the start of the page

  mapped_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd,
                     dev_base & ~MAP_MASK);
  if (mapped_base == (void *)-1) {
    printf("Can't map the memory to user space.\n");
    return -1;
  }
  printf("GPIO Memory mapped at address %p.\n", mapped_base);
  // get the address of the device in user space which will be an offset from
  // the base that was mapped as memory is mapped at the start of a page

  mapped_dev_base = mapped_base + (dev_base & MAP_MASK);
  return 0;
}
void mmap_gpio_write32(unsigned int uval, unsigned int reg_offset) {
  *((unsigned int *)(mapped_dev_base + reg_offset)) = uval;
}

/* Main Board configuration and streaming */
int main(int argc, char **argv) {

  /*int16_t *pval16;*/

  int rv;
  int trigger_value = 4000; // 0x0025;
  int delay_val = 0;
  unsigned long ulval;

  // Listen to ctrl+c and ASSERT
  signal(SIGINT, handle_sig);

  mmap_gpio_mem();

  // Clear Write reg enable
  rv = gpiod_ctxless_set_value(GPIO_CHIP_NAME, TRIG_REG_WRT_OFF, 0, false,
                               GPIO_CONSUMER, NULL, NULL);
  rv = write_trigger_reg(1, trigger_value);
  rv = write_trigger_reg(3, trigger_value);
  trigger_value = -6000; // 0x0025;
  rv = write_trigger_reg(2, trigger_value);
  /*sprintf(fd_name,"intData.bin");*/
  /*fd_data1 = fopen("intData34.bin", "wb");*/

  ulval = 0x11;
  mmap_gpio_write32(0x11, GPIO_1_DATA_OFFSET);
  usleep(100000);

  rv = get_multiple_gpio(TRIG_REG_VAL_OFF, 16, &delay_val); // Lines 40-55
  if (rv) {
    printf("Error get_multiple_gpio: %d\n", rv);
    return -1;
  }
  printf("* get_multiple_gpio delay_val: %d, %.3f us\n", delay_val,
         delay_val / 5.0 * 8e-3);
  // turns OFF LED, reset trigger machine
  rv = gpiod_ctxless_set_value(GPIO_CHIP_NAME, TRIG_EN_OFF, 0, false,
                               GPIO_CONSUMER, NULL, NULL);
  /*Fast reading of GPIO register*/
  ulval = *((unsigned long *)(mapped_dev_base + GPIO_0_DATA_OFFSET));
  printf("gpio 0 val 0x%lX,", ulval);
  ulval = *((unsigned long *)(mapped_dev_base + GPIO_1_DATA_OFFSET));
  printf("gpio 1 val 0x%lX \n", ulval);
  if (munmap(mapped_base, MAP_SIZE) == -1) {
    printf("Can't unmap memory from user space.\n");
    exit(0);
  }

  close(memfd);
  printf("Program Ended\n");
  return 0;
}
