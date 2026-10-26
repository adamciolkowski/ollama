#!/bin/sh
# Start the K2100M Ollama runtime image (build with Dockerfile.k2100m first).
#
# The image name must be the last argument to docker run. The volume needs -v:
#   -v ollama-k2100m:/root/.ollama
# Without -v, Docker passes the volume string to /bin/ollama as an unknown command.

set -eu

IMAGE=${IMAGE:-ollama-k2100m}
NAME=${NAME:-ollama-k2100m}
VOLUME=${VOLUME:-ollama-k2100m}

docker rm -f "$NAME" >/dev/null 2>&1 || true

docker run -d \
  --name "$NAME" \
  --gpus all \
  --restart unless-stopped \
  -p 11434:11434 \
  -v "${VOLUME}:/root/.ollama" \
  "$IMAGE"

echo "Started $NAME (image $IMAGE). Logs: docker logs -f $NAME"
echo "Pull/run: docker exec -it $NAME ollama pull tinyllama"
echo "          docker exec -it $NAME ollama run tinyllama"
