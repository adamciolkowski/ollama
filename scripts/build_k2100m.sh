#!/bin/sh

set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
IMAGE=${IMAGE:-ollama-k2100m-build}
OUTPUT=${OUTPUT:-"$ROOT/dist/ollama-linux-amd64-k2100m"}

docker build \
    --platform linux/amd64 \
    --target artifact \
    --file "$ROOT/Dockerfile.k2100m" \
    --tag "$IMAGE" \
    "$ROOT"

container=$(docker create "$IMAGE")
trap 'docker rm -f "$container" >/dev/null 2>&1 || true' EXIT INT TERM

mkdir -p "$(dirname -- "$OUTPUT")"
docker cp "$container:/ollama" "$OUTPUT"
chmod +x "$OUTPUT"

echo "Built $OUTPUT"
