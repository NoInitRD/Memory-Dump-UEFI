## Contributing

Thanks for your interest in contributing!

## Making Changes

To make changes to the UEFI application:

1. Edit the main source file: `app/Application.c`

2. Build the application by running: `./build.sh`

3. Simulate using `./sim.sh` or flash it to a storage device and boot off it to test.

The `build.sh` script uses an **EDK2 container from TianoCore** to handle the build process. The `sim.sh` script requires **QEMU** to be installed.

## Submitting Changes

- Just open a **Pull Request (PR)** with your changes.
- I’ll take a look—if everything looks good, I’ll merge it.
- If there’s something that needs improvement, I’ll either explain why or offer suggestions to help get it there.