#ifndef __APPLE__  // TODO - maybe consider nvidia support on intel macs?

#include "gpu_info_cuda.h"

#include <string.h>

#ifndef _WIN32
const char *cuda_lib_paths[] = {
    // NVIDIA Container Toolkit bind-mounts driver libs here.
    "/usr/local/nvidia/lib64/libnvidia-ml.so.1",
    "/usr/local/nvidia/lib/libnvidia-ml.so.1",
    "libnvidia-ml.so.1",
    "libnvidia-ml.so",
    "/usr/local/cuda/lib64/libnvidia-ml.so.1",
    "/usr/local/cuda/lib64/libnvidia-ml.so",
    "/usr/lib/x86_64-linux-gnu/libnvidia-ml.so.1",
    "/usr/lib/x86_64-linux-gnu/nvidia/current/libnvidia-ml.so",
    "/usr/lib/wsl/lib/libnvidia-ml.so.1",
    NULL,
};
#else
const char *cuda_lib_paths[] = {
    "nvml.dll",
    "",
    NULL,
};
#endif

#define CUDA_LOOKUP_SIZE 5

void cuda_init(cuda_init_resp_t *resp) {
  nvmlReturn_t ret;
  resp->err = NULL;
  const int buflen = 256;
  char buf[buflen + 1];
  int i;

  struct lookup {
    char *s;
    void **p;
  } l[CUDA_LOOKUP_SIZE] = {
      {"nvmlInit_v2", (void *)&resp->ch.initFn},
      {"nvmlShutdown", (void *)&resp->ch.shutdownFn},
      {"nvmlDeviceGetHandleByIndex", (void *)&resp->ch.getHandle},
      {"nvmlDeviceGetMemoryInfo", (void *)&resp->ch.getMemInfo},
      {"nvmlDeviceGetCount_v2", (void *)&resp->ch.getCount},
  };

  for (i = 0; cuda_lib_paths[i] != NULL && resp->ch.handle == NULL; i++) {
    resp->ch.handle = LOAD_LIBRARY(cuda_lib_paths[i], RTLD_LAZY);
    if (!resp->ch.handle) {
      (void)LOAD_ERR();
    }
  }
  if (!resp->ch.handle) {
    char *msg = LOAD_ERR();
    const char *last = cuda_lib_paths[i - 1];
    snprintf(buf, buflen,
             "Unable to load NVML (libnvidia-ml) after trying %d paths; last=%s: %s",
             i, last ? last : "none", msg ? msg : "unknown error");
    free(msg);
    resp->err = strdup(buf);
    return;
  }

  for (i = 0; i < CUDA_LOOKUP_SIZE; i++) {
    *l[i].p = LOAD_SYMBOL(resp->ch.handle, l[i].s);
  }
  if (!resp->ch.initFn) {
    resp->ch.initFn = LOAD_SYMBOL(resp->ch.handle, "nvmlInit");
  }
  if (!resp->ch.getCount) {
    resp->ch.getCount = LOAD_SYMBOL(resp->ch.handle, "nvmlDeviceGetCount");
  }
  if (!resp->ch.initFn || !resp->ch.shutdownFn || !resp->ch.getHandle ||
      !resp->ch.getMemInfo || !resp->ch.getCount) {
    UNLOAD_LIBRARY(resp->ch.handle);
    resp->ch.handle = NULL;
    resp->err = strdup("NVML loaded but required symbols are missing");
    return;
  }

  ret = (*resp->ch.initFn)();
  if (ret != NVML_SUCCESS) {
    snprintf(buf, buflen, "nvml vram init failure: %d", ret);
    resp->err = strdup(buf);
  }

  return;
}

void cuda_check_vram(cuda_handle_t h, mem_info_t *resp) {
  resp->err = NULL;
  nvmlDevice_t device;
  nvmlMemory_t memInfo = {0};
  nvmlReturn_t ret;
  const int buflen = 256;
  char buf[buflen + 1];
  int i;

  if (h.handle == NULL) {
    resp->err = strdup("nvml handle sn't initialized");
    return;
  }

  unsigned int devices;
  ret = (*h.getCount)(&devices);
  if (ret != NVML_SUCCESS) {
    snprintf(buf, buflen, "unable to get device count: %d", ret);
    resp->err = strdup(buf);
    return;
  }

  resp->total = 0;
  resp->free = 0;

  for (i = 0; i < devices; i++) {
    ret = (*h.getHandle)(i, &device);
    if (ret != NVML_SUCCESS) {
      snprintf(buf, buflen, "unable to get device handle %d: %d", i, ret);
      resp->err = strdup(buf);
      return;
    }

    ret = (*h.getMemInfo)(device, &memInfo);
    if (ret != NVML_SUCCESS) {
      snprintf(buf, buflen, "device memory info lookup failure %d: %d", i, ret);
      resp->err = strdup(buf);
      return;
    }

    resp->total += memInfo.total;
    resp->free += memInfo.free;
  }
}
#endif  // __APPLE__