#!/bin/bash

IMAGE_NAME="bootable_image.iso"
MOUNT_DIR="/mnt/efi_image"
EFI_DIR="$MOUNT_DIR/EFI/BOOT"
BOOT_FILE="dependencies/shellx64.efi"
APP_FILE="Build/Application.efi"
VOLUME_LABEL="BOOT"
FAT_SIZE_MB=7000

#check for necessary dependencies
dependencies=("dd" "mkfs.vfat" "mount" "umount")

for dep in "${dependencies[@]}"; do
    if ! command -v "$dep" &> /dev/null; then
        printf "Error: $dep is not installed. Please install it and try again.\n"
        exit 1
    fi
done

#create a temporary mount directory for the EFI image
sudo mkdir -p "$MOUNT_DIR"

#create an empty FAT32 image file
printf "Creating a FAT32 image...\n"
sudo dd if=/dev/zero of="$IMAGE_NAME" bs=1MB count=$FAT_SIZE_MB status=progress

#format the image as FAT32
printf "Formatting the image as FAT32...\n"
sudo mkfs.vfat -F 32 "$IMAGE_NAME"

#mount the image
printf "Mounting the image...\n"
sudo mount -o loop "$IMAGE_NAME" "$MOUNT_DIR"

#create the necessary EFI directories
sudo mkdir -p "$EFI_DIR"

#copy "Application.efi" into the EFI boot directory
printf "Copying Application.efi to $EFI_DIR...\n"
sudo cp "$BOOT_FILE" "$EFI_DIR/BOOTx64.efi"
sudo cp "$APP_FILE" "$MOUNT_DIR/app.efi"

#unmount the image
printf "Unmounting the image...\n"
sudo umount "$MOUNT_DIR"

#clean up
sudo rm -rf "$MOUNT_DIR"

printf "Finished\n"
