FROM ghcr.io/tianocore/containers/ubuntu-22-build:a0dd931

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y swig && \
    rm -rf /var/lib/apt/lists/*

RUN python3 -m pip install --upgrade pip setuptools wheel

ENV EDK2_REPO_URL="https://github.com/tianocore/edk2.git"
ENV EDK2_REPO_HASH="d189de3b0a2f44f4c9b87ed120be16569ea19b51"
ENV EDK2_PATH="/edk2"

RUN git clone --depth 1 --single-branch "${EDK2_REPO_URL}" "${EDK2_PATH}" && \
    python3 -m pip install --no-cache-dir -r "${EDK2_PATH}/pip-requirements.txt" && \
    stuart_setup TOOL_CHAIN_TAG=GCC && \
    stuart_update TOOL_CHAIN_TAG=GCC
