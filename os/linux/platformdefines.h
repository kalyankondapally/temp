/*
// Copyright (c) 2016 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#ifndef OS_LINUX_PLATFORMDEFINES_H_
#define OS_LINUX_PLATFORMDEFINES_H_

#include <stdio.h>
#include <stddef.h>
#include <gbm.h>
#include <assert.h>

#include <cstring>
#include <algorithm>
#include <cstddef>

#include <libsync.h>

#include "string8.h"
#include "hwcdefs_internal.h"

struct gbm_handle {
#ifdef USE_MINIGBM
  struct gbm_import_fd_planar_data import_data;
#else
  struct gbm_import_fd_data import_data;
#endif
  struct gbm_bo* handle = NULL;
  uint32_t total_planes = 0;
};

typedef struct gbm_handle *HWCNativeHandle;
typedef struct gbm_handle HWCNativeHandlesp;
typedef hwcomposer::String8 HWCString;
typedef int64_t nsecs_t;       // nano-seconds

typedef int32_t err_status_t;

namespace hwcomposer {
int property_get(const char *key, char *value, const char *default_value);

int property_set(const char *key, const char *value);

int property_list(void (*propfn)(const char *key, const char *value,
                                 void *cookie),
		  void *cookie);
}

#define PROPERTY_VALUE_MAX 92

#ifdef _cplusplus
extern "C" {
#endif

#define VTRACE(fmt, ...) fprintf(stderr, "%s: \n" fmt, __func__, ##__VA_ARGS__)
#define DTRACE(fmt, ...) fprintf(stderr, "%s: \n" fmt, __func__, ##__VA_ARGS__)
#define ITRACE(fmt, ...) fprintf(stderr, "\n" fmt, ##__VA_ARGS__)
#define WTRACE(fmt, ...) fprintf(stderr, "%s: \n" fmt, __func__, ##__VA_ARGS__)
#define ETRACE(fmt, ...) fprintf(stderr, "%s: \n" fmt, __func__, ##__VA_ARGS__)
#define ETRACEIF(fmt, ...) ((void)0)
#define ITRACEIF(fmt, ...) ((void)0)
#define DTRACEIF(fmt, ...) ((void)0)
#define VTRACEIF(fmt, ...) ((void)0)
#define WTRACEIF(fmt, ...) ((void)0)
#define HWCASSERT(fmt, ...) ((void)0)
#define STRACE() ((void)0)

// TODO: Provide proper support.
#define ATRACE_CALL_IF(enable) ((void)0)
#define ATRACE_NAME_IF(enable, name) ((void)0)
#define ATRACE_INT_IF(enable, name, value) ((void)0)
#define ATRACE_EVENT_IF(enable, name) ((void)0)

#ifndef LOG_ALWAYS_FATAL_IF
#define LOG_ALWAYS_FATAL_IF(cond, ...)                                  \
  ((CONDITION(cond)) ? ((void)HWCASSERT(#cond, LOG_TAG, ##__VA_ARGS__)) \
                     : (void)0)
#endif

// _cplusplus
#ifdef _cplusplus
}
#endif

#endif  // OS_LINUX_PLATFORMDEFINES_H_
