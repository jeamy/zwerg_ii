FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Build toolchain (Ubuntu 22.04 has GCC 11 with GLIBCXX_3.4.30)
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        build-essential \
        cmake \
        git \
        zip \
        pkg-config \
        patchelf \
        libgl1-mesa-dev \
        libdbus-1-dev \
        libfreetype6-dev \
        libfontconfig1-dev \
        libxkbcommon-dev \
        libvulkan-dev \
        python3 \
        python3-pip \
        python3-dev \
        protobuf-compiler \
        libprotobuf-dev \
    && rm -rf /var/lib/apt/lists/*

# Qt6 (official prebuilt binaries via aqtinstall)
# Rationale: focal repositories / okirby qt6-backports do not provide Qt6 Multimedia/WebSockets dev packages.
ARG QT_VERSION=6.5.3
ARG QT_TARGET_DIR=/opt/Qt

RUN python3 -m pip install --no-cache-dir aqtinstall==3.1.7 \
    && python3 -m aqt install-qt linux desktop "$QT_VERSION" gcc_64 -O "$QT_TARGET_DIR" --modules qtmultimedia qtwebsockets \
    && rm -rf /root/.cache/pip

# Make Qt discoverable for CMake in subsequent layers / container sessions.
ENV CMAKE_PREFIX_PATH=${QT_TARGET_DIR}/${QT_VERSION}/gcc_64
ENV PATH=${QT_TARGET_DIR}/${QT_VERSION}/gcc_64/bin:$PATH

WORKDIR /work
