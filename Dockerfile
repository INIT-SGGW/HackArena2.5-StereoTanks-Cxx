# Stage 1: Build the C++ Application
FROM debian:bullseye-slim AS builder

# Set environment variables for non-interactive installs and vcpkg setup
ENV DEBIAN_FRONTEND=noninteractive
ENV VCPKG_ROOT=/vcpkg
ENV CMAKE_ARGS="-DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

# Install necessary build packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    unzip \
    zip \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Clone and Set Up vcpkg for dependency management
RUN git clone https://github.com/microsoft/vcpkg.git $VCPKG_ROOT && cd $VCPKG_ROOT && ./bootstrap-vcpkg.sh

# Set the working directory
WORKDIR /usr/src/app

# Copy vcpkg.json for caching
COPY vcpkg.json ./vcpkg.json

# Install dependencies via vcpkg
RUN $VCPKG_ROOT/vcpkg install --triplet x64-linux

# Copy the application source code and build the application
COPY . .
RUN cmake . $CMAKE_ARGS -DCMAKE_EXE_LINKER_FLAGS="-static -pthread" && make

# Stage 2: Minimal Runtime Image
FROM gcr.io/distroless/base-debian10

# Copy the statically linked binary from the build stage
COPY --from=builder /usr/src/app/HackArena2.5-StereoTanks-Cxx /app/HackArena2.5-StereoTanks-Cxx
COPY --from=builder /usr/src/app/data /app/data

# Set non-root user (if applicable)
USER 1000

# Set the entry point to run the application
ENTRYPOINT ["/app/HackArena2.5-StereoTanks-Cxx"]
