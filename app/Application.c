#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <Guid/FileInfo.h>
#include <string.h>

#define CHUNK_SIZE 0x8000000     //128 MB per chunk
#define MAX_FILE_SIZE 0xA8000000 //4GB per file (fat32 restriction)

//wait for user input before continuing
VOID WaitForKeyInput()
{
    EFI_INPUT_KEY Key;
    Print(L"Press any key to exit...\n");
    while (TRUE)
    {
        if (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == EFI_SUCCESS)
            break;
    }
}

//finds the working directory of the current application
EFI_STATUS GetWorkingDirectory(EFI_HANDLE ImageHandle, EFI_FILE_PROTOCOL **RootDir)
{
    EFI_LOADED_IMAGE *LoadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
    EFI_STATUS Status;

    //get the loaded image protocol
    Status = gBS->OpenProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&LoadedImage, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(Status))
    {
        Print(L"Failed to get loaded image protocol: %r\n", Status);
        return Status;
    }

    //get the file system protocol
    Status = gBS->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID **)&Volume);
    if (EFI_ERROR(Status))
    {
        Print(L"Failed to get file system protocol: %r\n", Status);
        return Status;
    }

    //open the root directory
    Status = Volume->OpenVolume(Volume, RootDir);
    if (EFI_ERROR(Status))
    {
        Print(L"Failed to open root directory: %r\n", Status);
        return Status;
    }

    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;
    EFI_FILE_PROTOCOL *RootDir;
    EFI_FILE_PROTOCOL *currentFile = NULL;
    CHAR16 fileName[30]; //buffer for the file name (dump1.bin, dump2.bin, etc.)

    //variables for the memory map
    EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
    UINTN MemoryMapSize = 0;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;

    EFI_MEMORY_DESCRIPTOR *desc;
    unsigned char *batch_buffer = NULL; //buffer to hold memory chunks

    UINTN totalMemoryWritten = 0; //total memory written across all files
    UINTN processedMemory = 0;    //memory processed for the current file

    //reset watchdog timer to avoid system reset after 5 minutes
    SystemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    //get the root directory using GetWorkingDirectory
    Status = GetWorkingDirectory(ImageHandle, &RootDir);
    if (EFI_ERROR(Status))
    {
        Print(L"Failed to get working directory: %r\n", Status);
        WaitForKeyInput();
        return Status;
    }

    //get memory map
    Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (Status == EFI_BUFFER_TOO_SMALL)
    {
        MemoryMapSize += 0x1000; //add 4 KB padding
        MemoryMap = AllocatePool(MemoryMapSize);
        if (MemoryMap == NULL)
        {
            Print(L"Failed to allocate memory for MemoryMap.\n");
            WaitForKeyInput();
            return EFI_OUT_OF_RESOURCES;
        }

        Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    }

    if (EFI_ERROR(Status))
    {
        Print(L"Failed to get memory map: %r\n", Status);
        WaitForKeyInput();
        return Status;
    }

    //allocate memory for batch buffer
    batch_buffer = AllocatePool(CHUNK_SIZE);
    if (batch_buffer == NULL)
    {
        Print(L"Failed to allocate batch buffer.\n");
        WaitForKeyInput();
        return EFI_OUT_OF_RESOURCES;
    }

    //prepare the file name for the first dump file
    UINTN dumpFileIndex = 1;

    //use UnicodeSPrint to format the file name (e.g., "dump1.bin")
    UnicodeSPrint(fileName, sizeof(fileName), L"dump%u.bin", dumpFileIndex);

    //open the first dump file
    Status = RootDir->Open(RootDir, &currentFile, fileName, EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
    if (EFI_ERROR(Status))
    {
        Print(L"Failed to create dump file %s: %r\n", fileName, Status);
        WaitForKeyInput();
        return Status;
    }

    Print(L"Generating new bin file: %s\n", fileName); //log message for new bin file creation

    //calculate the total memory size to dump
    UINTN totalMemorySize = 0;
    for (UINTN i = 0; i < MemoryMapSize / DescriptorSize; i++)
    {
        desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)MemoryMap + i * DescriptorSize);
        if (desc->Type == EfiConventionalMemory)
            totalMemorySize += (UINTN)desc->NumberOfPages * EFI_PAGE_SIZE;
    }

    UINTN chunk_size_in_bytes;
    long region_offset;

    //process memory regions and write chunks to file
    for (UINTN i = 0; i < MemoryMapSize / DescriptorSize; i++)
    {
        desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)MemoryMap + i * DescriptorSize);
        if (desc->Type == EfiConventionalMemory)
        {
            long region_size = (long)desc->NumberOfPages * EFI_PAGE_SIZE;
            if (region_size > 0)
            {
                region_offset = 0;
                while (region_offset < region_size)
                {
                    //determine the size of the chunk to copy
                    chunk_size_in_bytes = (region_size - region_offset < CHUNK_SIZE) ? (region_size - region_offset) : CHUNK_SIZE;

                    //copy memory to batch buffer
                    gBS->CopyMem(batch_buffer, (void *)(desc->PhysicalStart + region_offset), chunk_size_in_bytes);

                    //write the chunk to the file
                    EFI_STATUS writeStatus = currentFile->Write(currentFile, &chunk_size_in_bytes, batch_buffer);
                    if (EFI_ERROR(writeStatus))
                    {
                        Print(L"Failed to write memory chunk to file: %r\n", writeStatus);
                        WaitForKeyInput();
                        return writeStatus;
                    }

                    //update total memory written across all files
                    totalMemoryWritten += chunk_size_in_bytes;

                    //update memory processed for the current file
                    processedMemory += chunk_size_in_bytes;

                    //calculate overall progress
                    UINTN overallProgress = (totalMemoryWritten * 100) / totalMemorySize;

                    //print overall progress
                    Print(L"\rOverall Progress: %u%%", overallProgress);

                    //if we've written 4GB for the current file, close the current file and create a new one (fat32 restriction)
                    if (processedMemory >= MAX_FILE_SIZE)
                    {
                        currentFile->Close(currentFile);

                        //prepare the file name for the next dump
                        dumpFileIndex++;
                        UnicodeSPrint(fileName, sizeof(fileName), L"dump%u.bin", dumpFileIndex);

                        Print(L"\nGenerating new bin file: %s\n", fileName);

                        //open the next dump file
                        Status = RootDir->Open(RootDir, &currentFile, fileName, EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
                        if (EFI_ERROR(Status))
                        {
                            Print(L"Failed to create dump file %s: %r\n", fileName, Status);
                            WaitForKeyInput();
                            return Status;
                        }

                        //reset processed memory count for the new file
                        processedMemory = 0;
                    }

                    region_offset += chunk_size_in_bytes;
                }
            }
        }
    }

    //close the file and exit
    if (currentFile)
        currentFile->Close(currentFile);

    Print(L"\nSuccessfully dumped memory to dump files.\n");

    WaitForKeyInput(); //hang for user input before exiting
    return EFI_SUCCESS;
}
