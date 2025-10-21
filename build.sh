#!/bin/bash
set -e

EDK2_CONTAINER="ghcr.io/tianocore/containers/fedora-39-build:latest"
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_DIR="${PROJECT_DIR}/app"

#colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

#check if Docker is installed
if ! command -v docker &> /dev/null; then
    printf "${RED}Error: Docker is not installed${NC}\n"
    printf "${RED}Please install Docker${NC}\n"
    exit 1
fi

#check if app directory exists
if [ ! -d "${APP_DIR}" ]; then
    printf "${RED}Error: app/ directory not found${NC}\n"
    exit 1
fi

#get edk2 image
printf "${YELLOW}Pulling edk2 build container...${NC}\n"
docker pull ${EDK2_CONTAINER}

printf "${GREEN}Container downloaded successfully!${NC}\n"

#run the container build script
chmod +x "${PROJECT_DIR}/docker/container_build.sh"

#run edk2 container
printf "${YELLOW}Running build in container...${NC}\n"
docker run --rm \
    -v "${PROJECT_DIR}:/work" \
    -w /work \
    -t \
    -e "TERM=xterm-256color" \
    -u $(id -u):$(id -g) \
    ${EDK2_CONTAINER} \
    /bin/bash /work/docker/container_build.sh

#check if we succeeded
if [ $? -eq 0 ]; then
    printf "${GREEN}Build successful!${NC}\n"
    printf "Built files are in: ${PROJECT_DIR}/Build/\n"
else
    printf "${RED}Build failed!${NC}\n"
    exit 1
fi

printf "${GREEN}Done!${NC}\n"
