#!/bin/bash

APP_FILE="Build/Application.efi"
MOUNT_DIR="/mnt/efi_image"

usage() {
    printf "Usage: $0 <device>\n"
    printf "Example: $0 /dev/sdX\n"
    exit 1
}

if [ $# -ne 1 ]; then
    usage
fi

#device to write the image to (e.g., /dev/sdX)
DEVICE=$1

#check if the device exists
if [ ! -b "$DEVICE" ]; then
    printf "Error: $DEVICE does not exist. Please check the device path and try again.\n"
    exit 1
fi

#confirm with the user before proceeding
printf "WARNING: This will overwrite the entire device $DEVICE.\n"
printf "Are you sure you want to continue? (y/n)\n"
read -r CONFIRM

if [[ ! "$CONFIRM" =~ ^[Yy]$ ]]; then
    printf "Operation aborted.\n"
    exit 0
fi

#unmount any existing partitions on the device
printf "Unmounting any existing partitions on $DEVICE...\n"
sudo umount ${DEVICE}* 2>/dev/null

#remove existing partitions and partition table
printf "Removing all partitions and partition table on $DEVICE...\n"
sudo parted "$DEVICE" --script mklabel gpt

#partition the device as FAT32 (taking up the entire device)
printf "Partitioning the device $DEVICE as FAT32...\n"

#create a new partition table and a single partition using parted
sudo parted "$DEVICE" --script mkpart primary fat32 0% 100%

#ensure the partition is recognized
sleep 2
PARTITION="${DEVICE}1"

#format the new partition as FAT32
printf "Formatting $PARTITION as FAT32...\n"
sudo mkfs.fat -F 32 "$PARTITION"

#mount the new partition
printf "Mounting $PARTITION...\n"
sudo mkdir -p "$MOUNT_DIR"
sudo mount "$PARTITION" "$MOUNT_DIR"

#copy the EFI files into the partition
EFI_DIR="$MOUNT_DIR/EFI/BOOT"
printf "Copying $APP_FILE to $EFI_DIR...\n"

#create the necessary directories and copy the EFI boot file
sudo mkdir -p "$EFI_DIR"
sudo cp "dependencies/shellx64.efi" "$EFI_DIR/BOOTx64.efi"
sudo cp "$APP_FILE" "$MOUNT_DIR/app.efi"

#unmount the partition after copying the files
printf "Unmounting $PARTITION...\n"
sudo umount "$MOUNT_DIR"

#safely eject the device
printf "Ejecting the device...\n"
sudo eject "$DEVICE"

printf "Finished.\n"
