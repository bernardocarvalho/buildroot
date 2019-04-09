# buildroot
Buildroot files for Xilinx KC705 linux and fmcjesdadc1 project board

## Steps
### 1

..  Copyright 2019-present

How to build Rootfs file for Microblaze Linux
=====================================

1. `Install PlatformIO Core <http://docs.platformio.org/page/core.html>`_
2. Download `development platform with examples <https://github.com/platformio/platform-ststm32/archive/develop.zip>`_

.. code-block:: bash

    # Change directory to example
    > cd platform-ststm32/examples/stm32cube-hal-blink
git clone https://github.com/buildroot/buildroot
git clone https://github.com/bernardocarvalho/buildroot-kc705-fmcjesdadc1
cd buildroot/configs/
cp ../../buildroot-kc705-fmcjesdadc1/board/ipfn-ist/fmcjesdadc1/configs/kc705_fmcjesdadc1_defconfig ./
cd ../board/
ln -s ../../buildroot-kc705-fmcjesdadc1/board/ipfn-ist

make kc705_fmcjesdadc1_defconfig
make menuconfig

