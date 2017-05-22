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

#ifndef HWCDEFS_INTERNAL_H_
#define HWCDEFS_INTERNAL_H_

#include <stdint.h>

#include <hwcrect.h>

#include "format.h"

namespace hwcomposer {

typedef enum _EHwcsScalingMode {
    HWCS_SCALE_CENTRE  = 0,  // Present the content centred at 1:1 source resolution.
    HWCS_SCALE_STRETCH,      // Do not preserve aspect ratio - scale to fill the display without cropping.
    HWCS_SCALE_FIT,          // Preserve aspect ratio - scale to closest edge (may be letterboxed or pillarboxed).
    HWCS_SCALE_FILL,         // Preserve aspect ratio - scale to fill the display (may crop the content).
    HWCS_SCALE_MAX_ENUM      // End of enum.
} EHwcsScalingMode;

// Display types.
enum EDisplayType {
  eDTPanel,
  eDTExternal,
  eDTVirtual,
  eDTWidi,
  eDTFake,
  eDTUnspecified,
};

#define INTEL_HWC_DEFAULT_HAL_PIXEL_FORMAT DRM_FORMAT_ABGR8888
#define INTEL_HWC_DEFAULT_REFRESH_RATE 60
#define INTEL_HWC_DEFAULT_REFRESH_PERIOD_NS \
  (1000000000 / (INTEL_HWC_DEFAULT_REFRESH_RATE))
#define INTEL_HWC_DEFAULT_BITS_PER_CHANNEL 16
#define INTEL_HWC_DEFAULT_INTERNAL_PANEL_DPI 160
#define INTEL_HWC_DEFAULT_EXTERNAL_DISPLAY_DPI 75

#define HWC_UNUSED(x) ((void)&(x))
#define UNUSED __attribute__((unused))

#if INTEL_HWC_LOGVIEWER_BUILD
const static bool sbLogViewerBuild = true;
#else
const static bool sbLogViewerBuild = false;
#endif

#if INTEL_HWC_INTERNAL_BUILD
const static bool sbInternalBuild = true;
#else
const static bool sbInternalBuild = false;
#endif

// This constant is used to indicate the maximum supported physical displays.
// This must be sufficient to cover panels, externals, virtuals, fakes and
// proxies etc.
const static unsigned int cMaxSupportedPhysicalDisplays = 8;

// This constant is used to indicate the maximum number of logical displays.
// A logical display can mux/demux between SurfaceFlinger displays and physical
// displays.
// A pool of logical displays can be created of which only some will be made
// available to SurfaceFlinger.
const static unsigned int cMaxSupportedLogicalDisplays = 8;

// This constant is used to indicate the maximum supported displays from
// SurfaceFlinger.
// TODO: Should come via a SF constant.
const static unsigned int cMaxSupportedSFDisplays = 3;

// Some generic constants
enum {
  INVALID_DISPLAY_ID = 0xFFFF,  // Display ID used to mean uninitialized or
                                // unspecified display index
};

enum {
  OK = 0,        // Everything's swell.
  NO_ERROR = 0,  // No errors.

  UNKNOWN_ERROR = 0x80000000,
  NO_MEMORY = -ENOMEM,
  INVALID_OPERATION = -ENOSYS,
  BAD_VALUE = -EINVAL,
  BAD_TYPE = 0x80000001,
  NAME_NOT_FOUND = -ENOENT,
  PERMISSION_DENIED = -EPERM,
  NO_INIT = -ENODEV,
  ALREADY_EXISTS = -EEXIST,
  DEAD_OBJECT = -EPIPE,
  FAILED_TRANSACTION = 0x80000002,
  JPARKS_BROKE_IT = -EPIPE,
#if !defined(HAVE_MS_C_RUNTIME)
  BAD_INDEX = -EOVERFLOW,
  NOT_ENOUGH_DATA = -ENODATA,
  WOULD_BLOCK = -EWOULDBLOCK,
  TIMED_OUT = -ETIMEDOUT,
  UNKNOWN_TRANSACTION = -EBADMSG,
#else
  BAD_INDEX = -E2BIG,
  NOT_ENOUGH_DATA = 0x80000003,
  WOULD_BLOCK = 0x80000004,
  TIMED_OUT = 0x80000005,
  UNKNOWN_TRANSACTION = 0x80000006,
#endif
  FDS_NOT_ALLOWED = 0x80000007,
};

#ifdef __cplusplus
#define CC_LIKELY(exp) (__builtin_expect(!!(exp), true))
#define CC_UNLIKELY(exp) (__builtin_expect(!!(exp), false))
#else
#define CC_LIKELY(exp) (__builtin_expect(!!(exp), 1))
#define CC_UNLIKELY(exp) (__builtin_expect(!!(exp), 0))
#endif

#define CONDITION(cond) (__builtin_expect((cond) != 0, 0))

}  // namespace hwcomposer
#endif  // PUBLIC_HWCDEFS_H_
