#!/bin/bash


#check for necessary dependencies
dependencies=("dd" "mkfs.vfat" "mount" "umount")

for dep in "${dependencies[@]}"; do
    if ! command -v "$dep" &> /dev/null; then
        echo "Error: $dep is not installed. Please install it and try again."
        exit 1
    fi
done


IMAGE_NAME="bootable_image.iso"
MOUNT_DIR="/mnt/efi_image"
EFI_DIR="$MOUNT_DIR/EFI/BOOT"
BOOT_FILE="boot.efi"
VOLUME_LABEL="BOOT"
FAT_SIZE_MB=7000


#create a temporary mount directory for the EFI image
sudo mkdir -p "$MOUNT_DIR"

#create an empty FAT32 image file
echo "Creating a FAT32 image..."
sudo dd if=/dev/zero of="$IMAGE_NAME" bs=1MB count=$FAT_SIZE_MB status=progress

#format the image as FAT32
echo "Formatting the image as FAT32..."
sudo mkfs.vfat -F 32 "$IMAGE_NAME"

#mount the image
echo "Mounting the image..."
sudo mount -o loop "$IMAGE_NAME" "$MOUNT_DIR"

#create the necessary EFI directories
sudo mkdir -p "$EFI_DIR"

#copy boot.efi into the EFI boot directory
echo "Copying boot.efi to $EFI_DIR..."
sudo cp "$BOOT_FILE" "$EFI_DIR/BOOTx64.efi"

#unmount the image
echo "Unmounting the image..."
sudo umount "$MOUNT_DIR"

#clean up
sudo rm -rf "$MOUNT_DIR"

echo "Finished"
