#!/usr/bin/env python3

import os
import sys
import subprocess
import shutil
from pathlib import Path

class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    NC = '\033[0m'

def print_status(message, color=Colors.BLUE):
    print(f"{color}[*] {message}{Colors.NC}")

def print_error(message):
    print(f"{Colors.RED}[ERROR] {message}{Colors.NC}", file=sys.stderr)

def print_success(message):
    print(f"{Colors.GREEN}[SUCCESS] {message}{Colors.NC}")

def run_command(cmd, cwd=None, env=None):
    print_status(f"Running: {cmd}")
    try:
        result = subprocess.run(
            cmd,
            shell=True,
            cwd=cwd,
            env=env,
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        if result.stdout:
            print(result.stdout)
        return True
    except subprocess.CalledProcessError as e:
        print_error(f"Command failed: {cmd}")
        if e.stdout:
            print(e.stdout)
        if e.stderr:
            print(e.stderr)
        return False

def find_edk2_root():
    edk2_root = Path("/work/edk2")
    
    if edk2_root.exists() and (edk2_root / "edksetup.sh").exists():
        return edk2_root
    
    return None

def setup_build_environment(edk2_root):
    print_status("Setting up EDK2 build environment...")
    
    env = os.environ.copy()
    env['WORKSPACE'] = str(edk2_root)
    env['PACKAGES_PATH'] = str(edk2_root)
    env['EDK_TOOLS_PATH'] = str(edk2_root / "BaseTools")
    
    setup_script = f"""
    cd {edk2_root}
    source edksetup.sh BaseTools > /dev/null 2>&1
    env
    """
    
    try:
        result = subprocess.run(
            ["bash", "-c", setup_script],
            capture_output=True,
            text=True,
            check=True
        )
        
        for line in result.stdout.split('\n'):
            if '=' in line:
                key, _, value = line.partition('=')
                env[key] = value
        
        return env
    except subprocess.CalledProcessError as e:
        print_error("Failed to setup EDK2 environment")
        print(e.stderr)
        return None

def build_basetools(edk2_root):
    basetools_dir = edk2_root / "BaseTools"
    genfw = basetools_dir / "Source" / "C" / "bin" / "GenFw"
    
    if genfw.exists():
        print_status("BaseTools already built")
        return True
    
    print_status("Building EDK2 BaseTools...")
    return run_command("make -C BaseTools", cwd=edk2_root)

def copy_application(edk2_root):
    print_status("Copying application to EDK2 workspace...")
    
    app_source = Path.cwd()
    app_dest = edk2_root / "MemoryDumpPkg"
    app_dest.mkdir(exist_ok=True)
    
    for file in ["Application.c", "Application.inf", "MemoryDumpPkg.dsc"]:
        src = app_source / file
        if src.exists():
            shutil.copy2(src, app_dest / file)
            print_status(f"Copied {file}")
        else:
            print_error(f"Source file not found: {file}")
            return False
    
    return True

def build_application(edk2_root, env, build_type="RELEASE"):
    print_status(f"Building application ({build_type})...")
    
    build_cmd = f"build -p MemoryDumpPkg/MemoryDumpPkg.dsc -a X64 -t GCC5 -b {build_type}"
    
    if not run_command(build_cmd, cwd=edk2_root, env=env):
        return False
    
    print_success("Build completed successfully!")
    return True

def copy_output(edk2_root):
    print_status("Copying output files...")
    
    build_dir = edk2_root / "Build" / "MemoryDump"
    output_dir = Path.cwd().parent / "Build"
    output_dir.mkdir(exist_ok=True)
    
    efi_files = list(build_dir.rglob("*.efi"))
    
    if not efi_files:
        print_error("No .efi files found in build output")
        return False
    
    for efi_file in efi_files:
        dest = output_dir / efi_file.name
        shutil.copy2(efi_file, dest)
        print_success(f"Copied {efi_file.name} to {output_dir}")
    
    return True

def main():
    print_status("Memory-Dump-UEFI Build Script")
    print_status("=" * 50)
    
    #find EDK2 root
    edk2_root = find_edk2_root()
    if not edk2_root:
        print_error("Could not find EDK2 root directory")
        print_error("Please ensure EDK2 is cloned and accessible")
        sys.exit(1)
    
    print_status(f"Found EDK2 at: {edk2_root}")
    
    #build BaseTools
    if not build_basetools(edk2_root):
        print_error("Failed to build BaseTools")
        sys.exit(1)
    
    #setup environment
    env = setup_build_environment(edk2_root)
    if not env:
        print_error("Failed to setup build environment")
        sys.exit(1)
    
    #copy application
    if not copy_application(edk2_root):
        print_error("Failed to copy application files")
        sys.exit(1)
    
    #build application
    build_type = sys.argv[1] if len(sys.argv) > 1 else "RELEASE"
    if build_type not in ["DEBUG", "RELEASE"]:
        print_error(f"Invalid build type: {build_type}")
        sys.exit(1)
    
    if not build_application(edk2_root, env, build_type):
        print_error("Build failed")
        sys.exit(1)
    
    #copy output
    if not copy_output(edk2_root):
        print_error("Failed to copy output files")
        sys.exit(1)
    
    print_success("=" * 50)
    print_success("Build completed successfully!")

if __name__ == "__main__":
    main()
