# This docker base image is used to build Cascade.
# It contains all the necessary dependencies to build and develop Cascade.

FROM ubuntu:18.04 AS cascade-builder

ARG MAKE_PARALLELISM=4

RUN apt-get update && apt-get -y install \
    build-essential \
    cmake \
    clang-8 \
    clang-format-8 \
    flex \
    bison \
    libncurses5-dev \
    libbenchmark-dev \
    libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

# gtest binaries are not build by default, so build them and install
RUN cd /usr/src/gtest && \
    mkdir build && \
    cmake .. && make -j ${MAKE_PARALLELISM} && make install && cd / && rm -rf /usr/src/gtest/build
