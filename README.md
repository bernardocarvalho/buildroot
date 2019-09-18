..  Copyright 2019-present 

# buildroot
Buildroot files for Xilinx KC705 linux and fmcjesdadc1 project board

## Steps



How to build Linux / Rootfs file for Microblaze Linux
=====================================

1. `Clone ADI Buildroot Fork` [Buildroot] (https://github.com/analogdevicesinc/buildroot)
2. `Clone Buildroot special fmcjesdadc1 components`

```bash
    # Make/Change directory to
    > mkdir BRdir
    > cd BRdir
    > git clone https://github.com/analogdevicesinc/buildroot
    > git clone https://github.com/bernardocarvalho/buildroot-kc705-fmcjesdadc1
    > cd buildroot/board
    > ln -s ../../buildroot-kc705-fmcjesdadc1/board/ipfn-ist
    > cd ../configs
    > ln -s ../board/ipfn-ist/fmcjesdadc1/configs/kc705_fmcjesdadc1_defconfig ./
    > cd ..
    # Point to uBlaze Toolchain, e.g.
    > export PATH="/mnt/sda5/Xilinx/SDK/2018.2/gnu/microblaze/linux_toolchain/lin64_le/bin:$PATH"
    > make kc705_fmcjesdadc1_defconfig
    # configure other choices 
    # (change linux branch to 2018_R1 (linux 4.9) 
    > make menuconfig
    > make
```

Not Needed ! How to build AD Linux Kernel for Microblaze (Linux buildroot 4.9.0-g7cf8748) 
=====================================

1. `Compile HDL (master branch) with Vivado 2018.2  <https://github.com/analogdevicesinc/hdl>`_
2. `Build Linux)
.. code-block:: bash
 
```bash
    # Make Change directory to
    > git clone https://github.com/analogdevicesinc/linux
    > git checkout 2018_R1
    # point to uBlaze Toolchain, e.g.
    > source /opt/Xilinx/SDK/2018.2/settings64.sh
    > export ARCH=microblaze
    > export CROSS_COMPILE=microblazeel-xilinx-linux-gnu-
    > make adi_mb_defconfig
    > make simpleImage.kc705_fmcjesdadc1
```
