
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

#ifndef OS_LINUX_DRMUTILS_H_
#define OS_LINUX_DRMUTILS_H_

#include <stdint.h>
#include <drm_fourcc.h>
#include "format.h"

inline int32_t convertPlatformFormatToDrmFormat(uint32_t format, bool bDiscardAlpha = false)
{
   return format;
}

inline uint32_t convertDrmFormatToPlatformFormat(int32_t format)
{
    return format;
}

#endif
