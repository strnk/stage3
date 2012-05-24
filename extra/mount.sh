#!/bin/bash
#
# grub-firmware-qemu

IMAGE_FILE="$1"
BASE_DIR=$(dirname "$0")
DEVICE=device

_FS_MOUNTED=0
_LOOP_FS_ON=0

# Check for argument count
if [[ $# -lt 2 ]]
then
	echo "usage: $0 imagename [files]"
	exit 1
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


echo -n "Mounting image on block device ... "
LOOP_FS="$(sudo losetup -v -f "$IMAGE_FILE" -o 1024000 | awk '{ print $NF }')"
checkError $LOOP_FS
_LOOP_FS_ON=1
echo

echo "Installing GRUB :"
echo -n "  \` Mounting filesystem ... "
mkdir -p $DEVICE
sudo mount -o user,umask=000 $LOOP_FS $DEVICE
checkError
_FS_MOUNTED=1

echo "Copying files ... "
shift 1
for file in $@
do
    echo -n "  \` $file ... "
    cp -Rf $file $DEVICE/
    checkError
done

sleep 1

echo -n "Unmounting filesystem ... "
sudo umount "$LOOP_FS"
checkError

rm -rf $DEVICE

echo -n "Unmounting image ... "
sudo losetup -d "$LOOP_FS"
checkError
