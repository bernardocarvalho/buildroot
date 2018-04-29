# https://elinux.org/images/2/2a/Using-buildroot-real-project.pdf
TARGETDIR=$1
B_DIR=`dirname $0`

# mount mount point
mkdir -p $TARGETDIR/mnt/nfs

grep -q "^10.136.241.211" $TARGETDIR/etc/fstab || \
    echo "10.136.241.211:/opt/share /mnt/nfs nfs port=2049,nolock,proto=tcp 0 0" \
    >> $TARGETDIR/etc/fstab

#    echo "10.136.241.211:/opt/share /mnt/nfs nfs hard,intr,rsize=8192,wsize=8192 0 0" \
#
# Copy the rootfs additions
#cp -a $BOARDDIR/rootfs-additions/* $TARGETDIR/
cp -a $B_DIR/rootfs-additions/* $TARGETDIR/
