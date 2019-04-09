..  Copyright 2019-present

# buildroot
Buildroot files for Xilinx KC705 linux and fmcjesdadc1 project board

## Steps
### 1


How to build Rootfs file for Microblaze Linux
=====================================

1. `Install Buildroot Core <http://docs.platformio.org/page/core.html>`_
2. `Install Buildroot  fmcjesdadc1 components`

.. code-block:: bash

    # Make Change directory to
    > mkdir BRdir
    > cd BRdir
    > git clone https://github.com/buildroot/buildroot
    > git clone https://github.com/bernardocarvalho/buildroot-kc705-fmcjesdadc1
    > cd buildroot/configs/
    > cp ../../buildroot-kc705-fmcjesdadc1/board/ipfn-ist/fmcjesdadc1/configs/kc705_fmcjesdadc1_defconfig ./
    > cd ../board/
    > ln -s ../../buildroot-kc705-fmcjesdadc1/board/ipfn-ist
    > cd ..
    > make kc705_fmcjesdadc1_defconfig
    # configure other choices 
    > make menuconfig
    > make

How to build AD Linux file for Microblaze Linux
=====================================

1. `Compile HDL (master branch) with Vivado 2018.2  <https://github.com/analogdevicesinc/hdl>`_
2. `Build Linux `

.. code-block:: bash
 
    # Make Change directory to
    > git clone https://github.com/analogdevicesinc/linux
    > git checkout 2018__R1
    # point to uBlaze Toolchain, e.g.
    > source /opt/Xilinx/SDK/2018.2/settings64.sh
    > export ARCH=microblaze
    > export CROSS_COMPILE=microblazeel-xilinx-linux-gnu-
    > make adi_mb_defconfig
    > make simpleImage.kc705_fmcjesdadc1

