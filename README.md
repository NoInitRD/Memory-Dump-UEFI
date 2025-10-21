# Introduction
A UEFI application for dumping the contents of RAM. Intended to be flashed onto a USB connected device and booted live. It comes with a UEFI shell to facilitate navigation and use.

**Please do not use this program for illegal or nefarious purposes.**

# Information
I originally made this utility so I could circumvent BitLocker on Windows 11, but it can be used for any reason (although it is particularly useful for forensics). I recommend flashing it to a NVME device connected via USB for the best performance. 

# Instructions
I included the compiled application so all you have to do to use it is run:

```sudo chmod +x flashImage.sh && sudo ./flashImage.sh /dev/sdX```

Once the image is flashed to a USB device, you need to boot from it and run "app.efi" from the UEFI shell. The application will be located on one of the connected drives (fs0, fs1, fs2...). To learn more about how to use the UEFI shell, [click here.](https://letmegooglethat.com/?q=how+to+use+UEFI+shell+)

# Development/Contributing

See [`CONTRIBUTING.md`](CONTRIBUTING.md).

# Tips
For each moment of time a computer is unpowered the contents of the RAM will degrade rapidly. To mitigate this, I like to short the reset pins on the devices motherboard. This will cause the device to abruptly reboot without interrupting power delivery, which keeps the contents of RAM fresh. These pins can be seen marked with "RESET" in the below image. Their location can vary based on what motherboard you are using. 

![image](https://github.com/user-attachments/assets/feea8481-e67c-47e5-ac1f-fecd5e2d124f)

Another tactic is to physically cool the RAM as much as possible, which can help slow the degradation. 
