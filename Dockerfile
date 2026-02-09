FROM --platform=linux/amd64 ubuntu:24.04

RUN apt-get update && apt-get install -y \
  build-essential gcc-multilib \
  nasm \
  grub-pc-bin grub-common \
  xorriso mtools \
  qemu-system-x86 \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /work
