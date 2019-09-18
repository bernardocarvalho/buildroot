# Cloned from board/analogdevicesinc/common/post-build.sh
#!/bin/sh

BOARD_DIR="$(dirname $0)"

install -D -m 0755 ${BOARD_DIR}/S40network ${TARGET_DIR}/etc/init.d/

sed -i '/hostname/a ::sysinit:/bin/mount -t debugfs none /sys/kernel/debug/'\
	${TARGET_DIR}/etc/inittab

rm -rf ${TARGET_DIR}/etc/dropbear
mkdir -p ${TARGET_DIR}/etc/dropbear
${HOST_DIR}/bin/dropbearkey -t rsa -f ${TARGET_DIR}/etc/dropbear/dropbear_rsa_host_key
${HOST_DIR}/bin/dropbearkey -t dss -f ${TARGET_DIR}/etc/dropbear/dropbear_dss_host_key
${HOST_DIR}/bin/dropbearkey -t ecdsa -f ${TARGET_DIR}/etc/dropbear/dropbear_ecdsa_host_key
# ***************
# ipfn section
# https://elinux.org/images/2/2a/Using-buildroot-real-project.pdf
TARGETDIR=$1

# create 'mount point'
mkdir -p ${TARGETDIR}/mnt/nfs

grep -q "^10.136.241.205" ${TARGETDIR}/etc/fstab || \
    echo "10.136.241.205:/opt/share /mnt/nfs nfs port=2049,nolock,proto=tcp 0 0" \
    >> ${TARGETDIR}/etc/fstab

#    echo "10.136.241.205:/opt/share /mnt/nfs nfs hard,intr,rsize=8192,wsize=8192 0 0" \
#
# Copy the rootfs additions
#cp -a $BOARDDIR/rootfs-additions/* $TARGETDIR/
cp -a ${BOARD_DIR}/rootfs-additions/* ${TARGETDIR}/
#touch $TARGETDIR/var/log/lastlog
