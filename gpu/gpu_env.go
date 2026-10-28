//go:build linux || windows

package gpu

import (
	"os"
	"strconv"
)

const defaultK2100MVramBytes = 2 * 1024 * 1024 * 1024

func vramFromEnv() (free, total uint64) {
	free = defaultK2100MVramBytes
	total = defaultK2100MVramBytes
	raw := os.Getenv("OLLAMA_GPU_MEMORY")
	if raw == "" {
		return free, total
	}
	v, err := strconv.ParseUint(raw, 10, 64)
	if err != nil || v == 0 {
		return free, total
	}
	return v, v
}

func shouldUseCudaWithoutNVML() bool {
	switch os.Getenv("OLLAMA_GPU") {
	case "cuda":
		return true
	case "off", "0", "false":
		return false
	}
	_, err := os.Stat("/dev/nvidia0")
	return err == nil
}

func cudaInfoWithoutNVML() (GpuInfo, bool) {
	if !shouldUseCudaWithoutNVML() {
		return GpuInfo{}, false
	}
	free, total := vramFromEnv()
	return GpuInfo{
		memInfo: memInfo{
			FreeMemory:  free,
			TotalMemory: total,
		},
		Library: "cuda",
	}, true
}
