# libiio - Library for interfacing industrial I/O (IIO) devices

BUILDROOT_HOME=/home/bernardo/Broot/buildroot
CC=$(BUILDROOT_HOME)/output/host/usr/bin/microblazeel-linux-gcc
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

