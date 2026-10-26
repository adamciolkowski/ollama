# Development

- Install cmake or (optionally, required tools for GPUs)
- run `go generate ./...`
- run `go build .`

Install required tools:

- cmake version 3.24 or higher
- go version 1.20 or higher
- gcc version 11.4.0 or higher

```bash
brew install go cmake gcc
```

Optionally enable debugging and more verbose logging:

```bash
export CGO_CFLAGS="-g"
```

Get the required libraries and build the native LLM code:

```bash
go generate ./...
```

Then build ollama:

```bash
go build .
```

Now you can run `ollama`:

```bash
./ollama
```

## Building on Linux with GPU support


### Linux/Windows CUDA (NVIDIA)
*Your operating system distribution may already have packages for NVIDIA CUDA. Distro packages are often preferable, but instructions are distro-specific. Please consult distro-specific docs for dependencies if available!*

Install `cmake` and `golang` as well as [NVIDIA CUDA](https://developer.nvidia.com/cuda-downloads) development and runtime packages.
Then generate dependencies:
```
go generate ./...
```
Then build the binary:
```
go build .
```

### Linux CUDA for Quadro K2100M

The Quadro K2100M is a Kepler compute-capability 3.0 GPU. CUDA 10.2 is the
last toolkit that can compile for it, and NVIDIA's R470 driver branch is the
last Linux driver family that supports it. Newer CUDA toolkits cannot be made
compatible by only relaxing Ollama's device check.

This branch is based on Ollama v0.1.18, the final release before Ollama added
an explicit minimum compute-capability check. Build the pinned CUDA 10.2,
GCC 8, and `sm_30` configuration with:

```bash
./scripts/build_k2100m.sh
```

The resulting binary is `dist/ollama-linux-amd64-k2100m`. Copy it to the
K2100M Linux host and run it normally:

```bash
./ollama-linux-amd64-k2100m serve
```

The host needs a working NVIDIA driver, but it does not need the CUDA toolkit
used to build Ollama. Verify that the log reports `cuda` rather than the CPU
fallback. The K2100M usually has 2 GiB of VRAM, so use small quantized models
and expect only partial layer offload.

To run Ollama in Docker instead, install Docker Engine and NVIDIA Container
Toolkit on the Linux host, then verify that the GPU is visible in a container:

```bash
docker run --rm --gpus all nvcr.io/nvidia/cuda:10.2-base-ubuntu18.04 nvidia-smi
```

Build the runtime image from this repository. The Dockerfile clones
Ollama `v0.1.18` and `llama.cpp` from GitHub, then overlays the Kepler
CUDA 10.2 patches from this tree:

```bash
docker build --platform linux/amd64 \
  --target runtime \
  -f Dockerfile.k2100m \
  -t ollama-k2100m .
```

Run it with GPU access and persistent model storage:

```bash
docker run -d \
  --name ollama-k2100m \
  --gpus all \
  --restart unless-stopped \
  -p 11434:11434 \
  -v ollama-k2100m:/root/.ollama \
  ollama-k2100m
```

CUDA 10.2, GCC, CMake, and Go are used only in the image's build stage. The
host requires the R470 NVIDIA driver and NVIDIA Container Toolkit, but no CUDA
Toolkit installation. For example, start with a small model:

```bash
docker exec -it ollama-k2100m ollama run tinyllama
```

### Linux ROCm (AMD)
*Your operating system distribution may already have packages for AMD ROCm and CLBlast. Distro packages are often preferable, but instructions are distro-specific. Please consult distro-specific docs for dependencies if available!*

Install [CLBlast](https://github.com/CNugteren/CLBlast/blob/master/doc/installation.md) and [ROCm](https://rocm.docs.amd.com/en/latest/deploy/linux/quick_start.html) developement packages first, as well as `cmake` and `golang`.
Adjust the paths below (correct for Arch) as appropriate for your distributions install locations and generate dependencies:
```
CLBlast_DIR=/usr/lib/cmake/CLBlast ROCM_PATH=/opt/rocm go generate ./...
```
Then build the binary:
```
go build .
```

ROCm requires elevated privileges to access the GPU at runtime.  On most distros you can add your user account to the `render` group, or run as root.

## Containerized Build

If you have Docker available, you can build linux binaries with `./scripts/build_linux.sh` which has the CUDA and ROCm dependencies included.