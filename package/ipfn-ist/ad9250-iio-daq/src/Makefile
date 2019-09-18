#Makefile fro Cross compile Mibcroblaze  apps with Buiuld root
# libiio - Library for interfacing industrial I/O (IIO) devices

# Needs to be absolute path
#BUILDROOT_HOME=/home/bernardo/FPGA/AD-GIT/buildroot
BUILDROOT_HOME=/home/bernardo/tmp/BRdir/buildroot
#CC=$(BUILDROOT_HOME)/output/host/usr/bin/microblazeel-linux-gcc
CC=$(BUILDROOT_HOME)/output/host/usr/bin/microblazeel-xilinx-linux-gnu-cc
CFLAGS=--sysroot=$(BUILDROOT_HOME)/output/staging
INCLUDES=
LFLAGS=

TARGET := ad9250-iio-daq

CFLAGS += -Wall
LDFLAGS += -liio
LDFLAGS += -lgpiod

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(TARGET).o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(TARGET:%=%.o)
