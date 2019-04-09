/*
 * iio - AD9250 IIO streaming example
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
 *
 **/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iio.h>
// Needed to cp ./build/libgpiod-v0.3/include/gpiod.h staging/usr/include/
/* Using libgpiod v0.3 
https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/tree/include/gpiod.h?h=v0.3.x

*/
#include <gpiod.h>

/* helper macros */
#define MHZ(x) ((long long)(x*1000000.0 + .5))
/*#define GHZ(x) ((long long)(x*1000000000.0 + .5))*/

#define GPIO_NUM_O_LINES 14
#define GPIO_LINE_OFFSET 18
#define N_CHAN 2
#define GPIO_CHIP_NAME "/dev/gpiochip0"
#define GPIO_CONSUMER "gpiod-consumer"

#define ASSERT(expr) { \
    if (!(expr)) { \
        (void) fprintf(stderr, "assertion failed (%s:%d)\n", __FILE__, __LINE__); \
        (void) abort(); \
    } \
}

/* RX is input, TX is output */
enum iodev { RX, TX };

/* IIO structs required for streaming */
static struct iio_context *ctx   = NULL;
static struct iio_device  *dev   = NULL;
static struct iio_channel *rx0_a = NULL;
static struct iio_channel *rx0_b = NULL;
static struct iio_channel *rx1_a = NULL;
static struct iio_channel *rx1_b = NULL;
static struct iio_buffer  *rxbuf0 = NULL;
//static struct iio_buffer  *rxbuf0_b = NULL;
/*static struct iio_buffer  *txbuf = NULL;*/

static bool stop;

/* cleanup and exit */
static void shutdown_iio()
{
    printf("* Destroying buffers\n");
    if (rxbuf0) { iio_buffer_destroy(rxbuf0); }
    /*if (txbuf) { iio_buffer_destroy(txbuf); }*/
    printf("* Disabling streaming channels\n");
    if (rx0_a) { iio_channel_disable(rx0_a); }
    if (rx0_b) { iio_channel_disable(rx0_b); }
    if (rx1_a) { iio_channel_disable(rx1_a); }
    if (rx1_b) { iio_channel_disable(rx1_b); }

    printf("* Destroying context\n");
    if (ctx) { iio_context_destroy(ctx); }
    //exit(0);
}

static void handle_sig(int sig)
{
    printf("Waiting for process to finish...\n");
    stop = true;
}

//http://www.wiki.xilinx.com/GPIO+User+Space+App
/*static void export_gpio(void ){*/
/*int exportfd;*/
/* Since linux 4.8 the GPIO sysfs interface is deprecated. User space should use
the character device instead. This library encapsulates the ioctl calls and
data structures behind a straightforward API.
*/
/*printf("GPIO test running...\n");*/
/*// The GPIO has to be exported to be able to see it*/
/*// in sysfs*/


/* simple configuration and streaming */
int main (int argc, char **argv)
{
    // Streaming devices
    //        struct iio_device *rx;

    // RX and TX sample counters
    //size_t nrx = 0;
    //	size_t ntx = 0;
    //	ssize_t nbytes_rx;//, nbytes_tx;
    char *p_dat_a, *p_end, *p_dat_b;
    char * pAdcData = NULL;
    ptrdiff_t p_inc;
    int16_t * pval16;
    //int16_t trigLevel = 100;//2000;
    unsigned int n_samples, bufSamples, savBytes;
    unsigned int savBlock;// =128*4096;

    /*char fd_name[64];*/
    FILE * fd_data;

    int rv;
    unsigned int gpio_offsets[40];
    int gpio_values[40];
    int trigger_value = 0x0025;
    for(int i=0; i < 2; i++){
        gpio_offsets[i] = 11 + i; //Lines 10-11 address lines
        gpio_values[i]= 0;
    }
    rv = gpiod_ctxless_set_value_multiple(GPIO_CHIP_NAME,gpio_offsets, 
        gpio_values, 2, false, GPIO_CONSUMER, NULL, NULL);
    if (rv) {
        printf("Error gpiod_chip_multiple %d\n", rv);
        //gpiod_chip_close(chip);
        return -1;
    }
    for(int i=0; i < 16; i++){
        gpio_offsets[i] = 40 + i; //Lines 40-55
        gpio_values[i]= ((trigger_value >> i) & 0x1);
    }
    rv = gpiod_ctxless_set_value_multiple(GPIO_CHIP_NAME,gpio_offsets, 
        gpio_values, 16, false, GPIO_CONSUMER, NULL, NULL);
    if (rv) {
        printf("Error gpiod_chip_multiple %d\n", rv);
        return -1;
    }
    rv = gpiod_ctxless_set_value(GPIO_CHIP_NAME,11, 1, false, GPIO_CONSUMER, NULL, NULL);
    usleep(50);
    rv = gpiod_ctxless_set_value(GPIO_CHIP_NAME,11, 0, false, GPIO_CONSUMER, NULL, NULL);
    rv = gpiod_ctxless_set_value(GPIO_CHIP_NAME,12, 1, false, GPIO_CONSUMER, NULL, NULL);
    usleep(50);
    rv = gpiod_ctxless_set_value(GPIO_CHIP_NAME,13, 0, false, GPIO_CONSUMER, NULL, NULL);
    if (rv) {
        printf("Error gpiod_set valchi %d\n", rv);
        return -1;
    }
    //	export_gpio();
    /*sprintf(fd_name,"intData.bin");*/
    fd_data =  fopen("intData.bin","wb");

    // Listen to ctrl+c and ASSERT
    signal(SIGINT, handle_sig);

    printf("* Acquiring IIO context\n");
    ASSERT((ctx = iio_create_local_context()) && "No context");
    ASSERT(iio_context_get_devices_count(ctx) > 0 && "No devices");
    dev =  iio_context_find_device(ctx, "axi-ad9250-hpc-0");
    ASSERT(dev && "No axi-ad9250-hpc-0 device found");
    /* finds AD9361 streaming IIO channels */
    rx0_a = iio_device_find_channel(dev, "voltage0", 0); // RX
    ASSERT(rx0_a && "No axi-ad9250-hpc-0 channel found");
    iio_channel_enable(rx0_a);
    rx0_b = iio_device_find_channel(dev, "voltage1", 0);
    ASSERT(rx0_b && "No axi-ad9250-hpc-1 channel found");
    iio_channel_enable(rx0_b);
    /*~1 ms buffers*/
    bufSamples = 256*1024;
    savBlock=N_CHAN *bufSamples; // 524288
    rxbuf0 = iio_device_create_buffer(dev, bufSamples, false);
    if (!rxbuf0) {
        perror("Could not create RX buffer");
        shutdown_iio();
    }
    savBytes=2*N_CHAN*sizeof(int16_t)* bufSamples;
    pAdcData = (char *) malloc(savBytes);
    if (!pAdcData) {
        perror("Could not create pAdcData buffer");
        shutdown_iio();
    }
//    do{
    for(int i=0; i < 32; i++){ // mas 16 ?
        iio_buffer_refill(rxbuf0);
        p_inc = iio_buffer_step(rxbuf0);
        p_end = iio_buffer_end(rxbuf0);
        p_dat_a = (char *)iio_buffer_first(rxbuf0, rx0_a);
        p_dat_b = (char *)iio_buffer_first(rxbuf0, rx0_b);
        pval16 = (int16_t *) (p_end - p_inc);
        printf("p_dat, %p, %p, End %p,  %d\n", p_dat_a, p_dat_b, p_end,  *pval16);
        if (*pval16 > 4){
            printf("trigger!\n");
            break;
        }
    //    usleep(10);
    }
  //  while(*pval16 < trigLevel);
    //memcpy(pAdcData, p_dat_a, (p_end - p_dat_a));
    for (int i=0; i<2; i++){
        memcpy(pAdcData + i*savBlock, p_dat_a + i*savBlock, savBlock);
    }
    n_samples = (p_end -p_dat_a)/ p_inc;
    printf("Inc, %d, End %p, N:%d,  SS, %d\n", p_inc, p_end, n_samples, bufSamples);
    printf("p_dat, %p, %p, End %p, N:%d, LS, %d\n", p_dat_a, p_dat_b, p_end, n_samples, *pval16);
    for (int i=0; i<64; i++){
        iio_buffer_refill(rxbuf0);
        p_inc = iio_buffer_step(rxbuf0);
        p_end = iio_buffer_end(rxbuf0);
        p_dat_a = (char *)iio_buffer_first(rxbuf0, rx0_a);
        for (int j=0; j<2; j++){
            memcpy(pAdcData + (j+2)*savBlock, p_dat_a + j*savBlock, savBlock);
        }
        //		fwrite(p_dat_a, 1, (p_end-p_dat_a), fd_data);
    }
    printf("Inc, %d, End %p, N:%d,  SS, %d\n", p_inc, p_end,(p_dat_a -p_end)/p_inc, iio_device_get_sample_size(dev));

    shutdown_iio();
    fwrite(pAdcData, 4, savBlock, fd_data); // Cannot be after shutdown() ???
    if(pAdcData) free(pAdcData);
    fclose(fd_data);
    printf("Program Ended\n");
    //close(fd_gpio);
    //gpiod_chip_close(chip);
    return 0;
}


