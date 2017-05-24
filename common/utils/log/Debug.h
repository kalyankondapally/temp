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

#ifndef COMMON_UTILS_LOG_DEBUGH
#define COMMON_UTILS_LOG_DEBUGH

namespace hwcomposer {

// Utility function - returns human-readable string from a DRM format number.
const char* getDRMFormatString( int32_t drmFormat );

#ifdef uncomment_hwc1
extern String8 printLayer(hwc_layer_1_t& layer);
extern void dumpDisplayContents(const char *pIdentifier, hwc_display_contents_1_t* pDisp, uint32_t frameIndex);
extern void dumpDisplaysContents(const char *pIdentifier, size_t numDisplays, hwc_display_contents_1_t** displays, uint32_t frameIndex);
#endif

}; // namespace hwcomposer

#endif // COMMON_UTILS_LOG_DEBUGH
