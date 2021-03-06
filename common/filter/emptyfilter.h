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

#ifndef INTEL_COMMON_HWC_EMPTYFILTER_H
#define INTEL_COMMON_HWC_EMPTYFILTER_H

#include "AbstractBufferManager.h"
#include "abstractfilter.h"
#include "platformdefines.h"
#include <hwcbuffer.h>

#include<list>

namespace hwcomposer {

class EmptyFilter : public AbstractFilter
{
public:
    EmptyFilter();
    virtual ~EmptyFilter();

    const char* getName() const { return "EmptyFilter"; }
    const Content& onApply(const Content& ref);
    HWCString dump();

protected:
    HWCNativeHandle getBlankBuffer(uint32_t width, uint32_t height);

    void ageBlankBuffers();

    AbstractBufferManager& mBM;
    // Private reference to hold modified state
    Content mReference;

    // Helper struct to contain per display state
    struct DisplayState
    {
        DisplayState() : mbWasModified(false) {}
        bool mbWasModified;
        Layer mBlankLayer;
    };
    DisplayState mDisplayState[cMaxSupportedSFDisplays];

    // Helper struct to contain per buffer state
    struct BufferState
    {
        BufferState() : mFramesSinceLastUsed(0) {}

	std::shared_ptr<HWCNativeHandlesp> mpBuffer;
	HwcBuffer buffer_data;
        uint32_t mFramesSinceLastUsed;
    };
    std::list<BufferState> mBufferList;
};

};
//}; // namespace hwc
//}; // namespace ufo
//}; // namespace intel

#endif // INTEL_COMMON_HWC_EMPTYFILTER_H
