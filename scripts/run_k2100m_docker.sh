#!/bin/sh
# Start the K2100M Ollama runtime image (build with Dockerfile.k2100m first).
#
# The image name must be the last argument to docker run. The volume needs -v:
#   -v ollama-k2100m:/root/.ollama
# Without -v, Docker passes the volume string to /bin/ollama as an unknown command.

set -eu

if ! docker run --rm --gpus all nvcr.io/nvidia/cuda:10.2-base-ubuntu18.04 nvidia-smi >/dev/null 2>&1; then
  echo "GPU not visible in Docker. Install NVIDIA Container Toolkit, restart Docker," >&2
  echo "then verify: docker run --rm --gpus all nvcr.io/nvidia/cuda:10.2-base-ubuntu18.04 nvidia-smi" >&2
  exit 1
fi

IMAGE=${IMAGE:-ollama-k2100m}
NAME=${NAME:-ollama-k2100m}
VOLUME=${VOLUME:-ollama-k2100m}

# WSL2: bind-mount host NVML when the toolkit does not inject /usr/local/nvidia into plain images.
EXTRA_MOUNTS=""
if [ -f /usr/lib/wsl/lib/libnvidia-ml.so.1 ]; then
  EXTRA_MOUNTS="-v /usr/lib/wsl/lib:/usr/lib/wsl/lib:ro"
fi

docker rm -f "$NAME" >/dev/null 2>&1 || true

# shellcheck disable=SC2086
docker run -d \
  --name "$NAME" \
  --gpus all \
  --restart unless-stopped \
  -p 11434:11434 \
  -v "${VOLUME}:/root/.ollama" \
  $EXTRA_MOUNTS \
  "$IMAGE"

echo "Started $NAME (image $IMAGE). Logs: docker logs -f $NAME"
echo "Expect version 0.1.18-k2100m and 'Nvidia GPU detected' in the log."
echo "Pull/run: docker exec -it $NAME ollama pull tinyllama"
echo "          docker exec -it $NAME ollama run tinyllama"
