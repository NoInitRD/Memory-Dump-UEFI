#!/bin/bash
set -e

#colors
YELLOW='\033[1;33m'
NC='\033[0m'

printf "${YELLOW}Inside container...${NC}\n"
cd /work

#clone edk2 if not present
if [ ! -d "edk2" ]; then
    printf "${YELLOW}Cloning edk2 repository...${NC}\n"
    git clone --depth 1 https://github.com/tianocore/edk2.git
    cd edk2
    git submodule update --init --recursive
    cd ..
fi

cd edk2

#setup build environment
printf "${YELLOW}Setting up edk2 build environment...${NC}\n"
export WORKSPACE=$(pwd)
export PACKAGES_PATH=${WORKSPACE}
source edksetup.sh

#build BaseTools if not already built
if [ ! -f "BaseTools/Source/C/bin/GenFw" ]; then
    printf "${YELLOW}Building edk2 BaseTools...${NC}\n"
    make -C BaseTools
fi

#copy application to edk2 tree
printf "${YELLOW}Copying application to edk2...${NC}\n"
APP_TARGET_DIR="${WORKSPACE}/Application"
mkdir -p ${APP_TARGET_DIR}
cp /work/app/* ${APP_TARGET_DIR}/

#build the application
printf "${YELLOW}Building UEFI application...${NC}\n"
cd ${APP_TARGET_DIR}

#run platform build
python3 /work/app/PlatformBuild.py

#copy output back to host
printf "${YELLOW}Copying build output...${NC}\n"
mkdir -p /work/Build
find ${WORKSPACE}/Build -name "*.efi" -exec cp {} /work/Build/ \;

printf "${YELLOW}Build Complete${NC}\n"
ls -lh /work/Build/
