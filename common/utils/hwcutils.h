/*
// Copyright (c) 2017 Intel Corporation
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

#ifndef COMMON_UTILS_HWCUTILS_H_
#define COMMON_UTILS_HWCUTILS_H_

#include <hwcdefs.h>
#include "hwcdefs_internal.h"

namespace hwcomposer {

inline HWCString dumpDisplayType(EDisplayType eDT) {
#define DISPLAYTYPE_TO_STRING(A) \
  case eDT##A:                   \
    return HWCString(#A);
  switch (eDT) {
    DISPLAYTYPE_TO_STRING(Panel);
    DISPLAYTYPE_TO_STRING(External);
    DISPLAYTYPE_TO_STRING(Virtual);
    DISPLAYTYPE_TO_STRING(Widi);
    DISPLAYTYPE_TO_STRING(Fake);
    DISPLAYTYPE_TO_STRING(Unspecified);
  }
#undef DISPLAYTYPE_TO_STRING
  return HWCString("<?>");
};

// Helper functions.
/**
 */
template <typename T>
static inline T min(T a, T b) {
  return a < b ? a : b;
}

/**
 */
template <typename T>
static inline T max(T a, T b) {
  return a > b ? a : b;
}

// Alignment template. Align must be a power of 2.
template <typename T>
inline T alignTo(T value, T align) {
  HWCASSERT(align > 1 && !(align & (align - 1)));
  return (value + (align - 1)) & ~(align - 1);
}

// Call poll() on fd.
//  - timeout: time in miliseconds to stay blocked before returning if fd
//  is not ready.
void HWCPoll(int fd, int timeout);

}  // namespace hwcomposer

#endif  // COMMON_UTILS_HWCUTILS_H_
