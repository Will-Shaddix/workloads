FROM ubuntu:22.04

ARG KREL=5.15.0-25-generic


# Install necessary packages for kernel build
# RUN apt update && apt install -y \
#     build-essential \
#     libncurses-dev \
#     bison \
#     flex \
#     libssl-dev \
#     libelf-dev \
#     bc \
#     wget \
#     git \
#     kmod \
#     apt-src \
#     vim \
#     curl \
#     file

RUN apt update && apt install -y \
    build-essential \
    libncurses-dev \
    bison \
    flex \
    libssl-dev \
    libelf-dev \
    libdw-dev \
    bc \
    wget \
    git \
    linux-headers-${KREL} \
    kmod \
    vim \
    curl \
    file \
    rsync \
    xz-utils \
    pkg-config \
    python3 \
    perl \
    dwarves \
    gcc-aarch64-linux-gnu \
    binutils-aarch64-linux-gnu \
    libc6-dev-arm64-cross \
    ndctl \
    daxctl \
    e2fsprogs \
    xfsprogs \
    util-linux



ENV CROSS_COMPILE=aarch64-linux-gnu-


RUN sed -Ei 's/^# deb-src /deb-src /' /etc/apt/sources.list
RUN apt update
RUN mkdir /workspace
RUN cd /workspace && apt source linux-image-unsigned-5.15.0-25-generic

RUN cd /workspace/linux-5.15.0 && \
    cd scripts && \
    chmod +x pahole-version.sh pahole-flags.sh && \
    cd .. && \
    # make defconfig && \
    make ARCH=arm64 defconfig && \
    # Enable PMEM/DAX/FS-DAX related options (built-in or modules) on top of defconfig.
    # NOTE: These options only take effect if the corresponding drivers/filesystems are present in this kernel tree.
    ./scripts/config --disable CONFIG_SECURITY_SELINUX && \
    ./scripts/config --disable CONFIG_SECURITY_SELINUX_BOOTPARAM && \
    ./scripts/config --disable CONFIG_SECURITY_SELINUX_DEVELOP && \
    ./scripts/config --disable CONFIG_SECURITY_SELINUX_AVC_STATS && \

    # --- make vmlinux ET_EXEC instead of ET_DYN (PIE) ---
    ./scripts/config --disable CONFIG_ARM64_KERNEL_PIE || true; \
    ./scripts/config --disable CONFIG_RANDOMIZE_BASE   || true; \
    # Sometimes PIE depends on relocatable; if you want to be aggressive:
    ./scripts/config --disable CONFIG_RELOCATABLE      || true; \

    ./scripts/config --enable CONFIG_DAX && \
    ./scripts/config --module CONFIG_DEV_DAX && \
    ./scripts/config --enable CONFIG_LIBNVDIMM && \
    ./scripts/config --enable CONFIG_BLK_DEV_PMEM && \
    ./scripts/config --enable CONFIG_NVDIMM_DAX && \
    ./scripts/config --enable CONFIG_NVDIMM_PFN && \
    ./scripts/config --enable CONFIG_NVDIMM_BTT && \
    ./scripts/config --enable CONFIG_FS_DAX && \
    ./scripts/config --enable CONFIG_FS_DAX_PMD && \
    ./scripts/config --enable CONFIG_DAX_PMEM && \
    ./scripts/config --enable CONFIG_DAX_DRIVER && \
    ./scripts/config --module CONFIG_VIRTIO_PMEM && \
    # Filesystem support (FS-DAX uses your FS driver too)
    ./scripts/config --enable CONFIG_EXT4_FS && \
    ./scripts/config --enable CONFIG_EXT4_FS_POSIX_ACL && \
    ./scripts/config --enable CONFIG_XFS_FS && \
    ./scripts/config --enable CONFIG_XFS_POSIX_ACL && \
    # Some FS-specific DAX toggles exist on certain versions; enable when available.
    ./scripts/config --enable CONFIG_EXT4_FS_DAX && \
    ./scripts/config --enable CONFIG_XFS_DAX && \
    # Memory hotplug / ZONE_DEVICE (for pmem/devdax memory-map support)
    ./scripts/config --enable CONFIG_MEMORY_HOTPLUG && \
    ./scripts/config --enable CONFIG_MEMORY_HOTREMOVE && \
    ./scripts/config --enable CONFIG_ZONE_DEVICE && \
    # (Often needed with ZONE_DEVICE/pmem as "memory")
    ./scripts/config --enable CONFIG_ARCH_HAS_PMEM_API || true; \
    ./scripts/config --enable CONFIG_DEV_PAGEMAP_OPS   || true; \
    ./scripts/config --enable CONFIG_MIGRATION         || true; \
    # Resolve dependencies/new symbols with defaults.
    make ARCH=arm64 olddefconfig && \
    make ARCH=arm64 -j 8 && \
    make INSTALL_MOD_PATH=/workspace/output modules_install




RUN git clone https://github.com/nkrim/gem5.git --depth=1 --filter=blob:none --no-checkout --sparse --single-branch --branch=gem5-bridge && \
    cd gem5 && \
    git sparse-checkout add util/m5 && \
    git sparse-checkout add util/gem5_bridge && \
    git sparse-checkout add include && \
    git checkout

# Prepare kernel tree for external module builds
RUN cd /workspace/linux-5.15.0 && \
    make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} modules_prepare

# Build + install gem5_bridge against the ARM64 kernel tree you built
RUN KREL_BUILT="$(make -sC /workspace/linux-5.15.0 ARCH=arm64 kernelrelease)" && \
    cd gem5/util/gem5_bridge && \
    make -C /workspace/linux-5.15.0 ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} M=$PWD modules && \
    make -C /workspace/linux-5.15.0 ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} M=$PWD \
        INSTALL_MOD_PATH=/workspace/output INSTALL_MOD_DIR=extra/gem5 modules_install && \
    depmod -b /workspace/output "${KREL_BUILT}" && \
    rm -rf /workspace/output/lib/modules/"${KREL_BUILT}"/build \
           /workspace/output/lib/modules/"${KREL_BUILT}"/source

# # Build + install module for the running headers KREL into /workspace/output
# RUN mkdir -p /workspace/output && \
#     cd gem5/util/gem5_bridge && \
#     make ARCH=arm64 -C /lib/modules/${KREL}/build M=$PWD modules && \
#     make ARCH=arm64-C /lib/modules/${KREL}/build M=$PWD \
#         INSTALL_MOD_PATH=/workspace/output INSTALL_MOD_DIR=gem5 modules_install && \
#     depmod -b /workspace/output ${KREL} && \
#     rm -rf /workspace/output/lib/modules/${KREL}/build \
#            /workspace/output/lib/modules/${KREL}/source

# RUN cd /workspace/output/lib/modules/5.15.193 && \
#     rm -rf build source


# RUN cd gem5/util/gem5_bridge && \
#     make KMAKEDIR=/workspace/linux-5.15.0 INSTALL_MOD_PATH=/workspace/output build install

RUN cd /workspace/output/lib/modules/5.15.193 && \
    rm -rf build source

CMD ["/bin/bash"]
