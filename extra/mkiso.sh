#!/bin/bash
#
# grub-firmware-qemu

IMAGE_FILE="$1"
BASE_DIR=$(dirname "$0")

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
	
	rm "$IMAGE_FILE"
fi

TMPDIR="/tmp/$$/"

mkdir -p $TMPDIR/boot/grub

# Generate the grub.cfg
echo -n "  \` Generating config files ... "
cat >  $TMPDIR/boot/grub/grub.cfg <<EOF
set timeout=0

menuentry 'stage3' {
	insmod fat
	insmod multiboot
	set root='(cd)'
	multiboot /stage3 Stage 3
}
EOF
echo "ok"

echo
echo "Copying files ... "
shift 1
for file in $@
do
    echo -n "  \` $file ... "
    cp -R $file $TMPDIR/
    echo
done

echo -n "  \` Generating core image ... "
grub-mkrescue --modules core,boot,multiboot -o $IMAGE_FILE $TMPDIR > /dev/null 2> /dev/null

echo "ok"
rm -rf $TMPDIR
