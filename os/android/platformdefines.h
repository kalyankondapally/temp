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

#ifndef OS_ANDROID_PLATFORMDEFINES_H_
#define OS_ANDROID_PLATFORMDEFINES_H_

#ifndef LOG_TAG
#define LOG_TAG "iahwcomposer"
#endif

#ifndef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#endif

#include <utils/Trace.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>

#include <ui/GraphicBuffer.h>

#include <sw_sync.h>
#include <sync/sync.h>
#include <utils/String8.h>
#include <utils/Timers.h>

#include "hwcdefs_internal.h"
#include "drmutils_android.h"

#ifdef _cplusplus
extern "C" {
#endif

struct gralloc_handle {
  buffer_handle_t handle_ = NULL;
  android::sp<android::GraphicBuffer> buffer_ = NULL;
};

typedef struct gralloc_handle* HWCNativeHandle;
typedef android::String8 HWCString;
typedef android::status_t err_status_t;

#define VTRACE(fmt, ...) ALOGV("%s: " fmt, __func__, ##__VA_ARGS__)
#define DTRACE(fmt, ...) ALOGD("%s: " fmt, __func__, ##__VA_ARGS__)
#define ITRACE(fmt, ...) ALOGI(fmt, ##__VA_ARGS__)
#define WTRACE(fmt, ...) ALOGW("%s: " fmt, __func__, ##__VA_ARGS__)
#define ETRACE(fmt, ...) ALOGE("%s: " fmt, __func__, ##__VA_ARGS__)
#define ETRACEIF(fmt, ...) ALOGE_IF("%s: " fmt, __func__, ##__VA_ARGS__)
#define ITRACEIF(fmt, ...) ALOGI_IF(fmt, ##__VA_ARGS__)
#define DTRACEIF(fmt, ...) ALOGD_IF("%s: " fmt, __func__, ##__VA_ARGS__)
#define VTRACEIF(fmt, ...) ALOGV_IF("%s: " fmt, __func__, ##__VA_ARGS__)
#define WTRACEIF(fmt, ...) ALOGW_IF("%s: " fmt, __func__, ##__VA_ARGS__)
#define HWCASSERT(fmt, ...) \
  ALOG_ASSERT(stderr, "%s: \n" fmt, __func__, ##__VA_ARGS__)
#define STRACE() ATRACE_CALL()

// This ScopedTrace function compiles away properly when disabled. Android's one
// doesnt, it
// leaves strings and atrace calls in the code.
class HwcScopedTrace {
 public:
  inline HwcScopedTrace(bool bEnable, const char* name) : mbEnable(bEnable) {
    if (mbEnable)
      atrace_begin(ATRACE_TAG_GRAPHICS, name);
  }

  inline ~HwcScopedTrace() {
    if (mbEnable)
      atrace_end(ATRACE_TAG_GRAPHICS);
  }

 private:
  bool mbEnable;
};

// Conditional variants of the macros in utils/Trace.h
#define ATRACE_CALL_IF(enable) HwcScopedTrace ___tracer(enable, __FUNCTION__)
#define ATRACE_NAME_IF(enable, name) HwcScopedTrace ___tracer(enable, name)
#define ATRACE_INT_IF(enable, name, value) \
  do {                                     \
    if (enable) {                          \
      ATRACE_INT(name, value);             \
    }                                      \
  } while (0)
#define ATRACE_EVENT_IF(enable, name) \
  do {                                \
    ATRACE_INT_IF(enable, name, 1);   \
    ATRACE_INT_IF(enable, name, 0);   \
  } while (0)
// _cplusplus
#ifdef _cplusplus
}
#endif

#endif  // OS_ANDROID_PLATFORMDEFINES_H_
