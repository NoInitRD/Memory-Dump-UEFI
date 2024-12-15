#!/bin/bash


usage() {
    echo "Usage: $0 <device>"
    echo "Example: $0 /dev/sdX"
    exit 1
}

if [ $# -ne 1 ]; then
    usage
fi

#device to write the image to (e.g., /dev/sdX)
DEVICE=$1

#check if the device exists
if [ ! -b "$DEVICE" ]; then
    echo "Error: $DEVICE does not exist. Please check the device path and try again."
    exit 1
fi

#confirm with the user before proceeding
echo "WARNING: This will overwrite the entire device $DEVICE."
echo "Are you sure you want to continue? (y/n)"
read -r CONFIRM

if [[ ! "$CONFIRM" =~ ^[Yy]$ ]]; then
    echo "Operation aborted."
    exit 0
fi


#unmount any existing partitions on the device
echo "Unmounting any existing partitions on $DEVICE..."
sudo umount ${DEVICE}* 2>/dev/null

#remove existing partitions and partition table
echo "Removing all partitions and partition table on $DEVICE..."
sudo parted "$DEVICE" --script mklabel gpt

#partition the device as FAT32 (taking up the entire device)
echo "Partitioning the device $DEVICE as FAT32..."

#create a new partition table and a single partition using parted
sudo parted "$DEVICE" --script mkpart primary fat32 0% 100%

#ensure the partition is recognized
sleep 2
PARTITION="${DEVICE}1"

#format the new partition as FAT32
echo "Formatting $PARTITION as FAT32..."
sudo mkfs.fat -F 32 "$PARTITION"

#mount the new partition
echo "Mounting $PARTITION..."
MOUNT_DIR="/mnt/efi_image"
sudo mkdir -p "$MOUNT_DIR"
sudo mount "$PARTITION" "$MOUNT_DIR"

#copy the EFI files into the partition
EFI_DIR="$MOUNT_DIR/EFI/BOOT"
BOOT_FILE="boot.efi"
echo "Copying $BOOT_FILE to $EFI_DIR..."

#create the necessary directories and copy the EFI boot file
sudo mkdir -p "$EFI_DIR"
sudo cp "dependencies/shellx64.efi" "$EFI_DIR/BOOTx64.efi"
sudo cp "$BOOT_FILE" "$MOUNT_DIR/app.efi"

#unmount the partition after copying the files
echo "Unmounting $PARTITION..."
sudo umount "$MOUNT_DIR"

#safely eject the device
echo "Ejecting the device..."
sudo eject "$DEVICE"

echo "Finished."
