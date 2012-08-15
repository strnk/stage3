#!/bin/bash
#
# grub-firmware-qemu

SECTOR_SIZE=1024
SECTOR_COUNT=20480
IMAGE_FILE="$1"
BASE_DIR=$(dirname "$0")


_FS_MOUNTED=0
_LOOP_FS_ON=0
_LOOP_DEVICE_ON=0

# Check for argument count
if [[ $# -lt 1 ]]
then
	echo "usage: $0 imagename"
	exit 1
fi

# Check if the output image already exists
if [[ -e "$IMAGE_FILE" ]]
then
	read -p "Output file already exists. Overwrite ? (y/n) "
	
	if [ $REPLY != "y" ]
	then
	    exit 0
	fi
fi

## Error checking function
# Does all the necessary steps in case of error
checkError() {
    # No error - good !
    if [ $? -eq 0 ]
    then
        if [ $# -lt 1 ]
        then
            echo "ok"
        else
            echo -n "$1"
        fi
        return
    fi
    
    # Print a failure message
    echo "/!\ Failure : check above for errors."
    
    if [ $_FS_MOUNTED -eq 1 ]
    then
        echo "Unmounting filesystem ... "
        sudo umount "$LOOP_FS"
        if [[ $? -ne 0 ]]
        then
            echo "unable to unmount!"
        else
            echo "ok"
        fi
    fi
    
    if [ $_LOOP_DEVICE_ON -eq 1 ]
    then
        echo "Unmounting image ... "
        sudo losetup -d "$LOOP_DEVICE"
        if [[ $? -ne 0 ]]
        then
            echo "unable to unloop!"
        else
            echo "ok"
        fi
    fi
    
    if [ $_LOOP_FS_ON -eq 1 ]
    then
        echo "Unmounting image (2) ... "
        sudo losetup -d "$LOOP_DEVICE"
        if [[ $? -ne 0 ]]
        then
            echo "unable to unloop!"
        else
            echo "ok"
        fi
    fi 
    
    exit 1  
}

# Zero-fill the image file
echo -n "Initializing $IMAGE_FILE ... "
bximage -hd -mode=flat -size=$(($SECTOR_COUNT/1024)) -q "$IMAGE_FILE"
#dd if=/dev/zero of="$IMAGE_FILE" bs=$SECTOR_SIZE count=$SECTOR_COUNT > /dev/null 2>/dev/null
checkError "ok : $(du -h --apparent-size $IMAGE_FILE | cut -f1)"
echo

echo -n "Mounting image on block device ... "
LOOP_FS="$(sudo losetup -v -f "$IMAGE_FILE" -o 1024000 | awk '{ print $NF }')"
checkError $LOOP_FS
_LOOP_FS_ON=1

LOOP_DEVICE="$(sudo losetup -v -f "$IMAGE_FILE" | awk '{ print $NF }')"
checkError "${LOOP_DEVICE}"
_LOOP_DEVICE_ON=1
echo

echo -n "Initializing partition table ... "
sudo parted "$LOOP_DEVICE" mklabel msdos
checkError

echo -n "Creating primary partition ... "
sudo parted "$LOOP_DEVICE" -a none mkpart primary fat32 1024000B 100%
checkError

echo -n "Initializing filesystem ... "
sudo mkfs.msdos $LOOP_FS >/dev/null
checkError

DEVICE=device
GRUB=${DEVICE}/grub

echo "Installing GRUB :"
echo -n "  \` Mounting filesystem ... "
mkdir -p $DEVICE
sudo mount -o user,umask=000 $LOOP_FS $DEVICE
checkError
_FS_MOUNTED=1

echo -n "  \` Creating GRUB sub-directory ... "
mkdir -p $GRUB
checkError


echo -n "  \` Copying grub files ... "
sudo cp /usr/lib/grub/i386-pc/{fat,multiboot,boot}.* $GRUB
checkError


echo -n "  \` Generating config files ... "
cat > ${GRUB}/device.map <<EOF
(hd0)   $LOOP_DEVICE
EOF
checkError "device.map "

# Generate the grub.cfg
cat > ${GRUB}/grub.cfg <<EOF
set timeout=0

menuentry 'stage3' {
	insmod fat
	insmod multiboot
	set root='(hd0, 1)'
	multiboot /stage3 Stage 3
}
EOF
checkError "grub.cfg"
echo


echo -n "  \` Generating core image ... "
sudo grub-mkimage -O i386-pc --output="${GRUB}/core.img" -c extra/grub_config --prefix="(hd0,<   1)/grub" fat biosdisk part_msdos configfile normal multiboot
checkError

echo "Copying files ... "
shift 1
for file in $@
do
    echo -n "  \` $file ... "
    cp -R $file $DEVICE/
    checkError
done

echo -n "  \` Installing GRUB ... "
sudo grub-setup --device-map="${GRUB}/device.map" --directory="${GRUB}" --root-device='(hd0,1)' '(hd0)'
checkError

## Launch a shell for debugging purpose
cd device
bash
cd ..
sleep 1

echo -n "Unmounting filesystem ... "
sudo umount "$LOOP_FS"
checkError

rm -rf $DEVICE

echo -n "Unmounting image ... "
sudo losetup -d "$LOOP_DEVICE"
sudo losetup -d "$LOOP_FS"
checkError
