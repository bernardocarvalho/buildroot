################################################################################
#
## AD9250_IIO_DAQ
#
#################################################################################

AD9250_IIO_DAQ_VERSION = 1.0
AD9250_IIO_DAQ_DEPENDENCIES = \ 
	libiio
AD9250_IIO_DAQ_SITE = ./package/ad9250-iio-daq/src
AD9250_IIO_DAQ_SITE_METHOD = local

define AD9250_IIO_DAQ_BUILD_CMDS
    $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
	endef

define AD9250_IIO_DAQ_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/ad9250-iio-daq $(TARGET_DIR)/usr/bin
	endef

$(eval $(generic-package))

