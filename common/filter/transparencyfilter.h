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

#ifndef INTEL_COMMON_HWC_TRANSPARENCYFILTER_H
#define INTEL_COMMON_HWC_TRANSPARENCYFILTER_H

#include <hwcdefs.h>

#include "abstractfilter.h"
#ifdef uncomment
#include "abstractbuffermanager.h"
#endif

#include <memory>


//namespace intel {
//namespace ufo {
//namespace hwc {
namespace hwcomposer {

#define MAX_DETECT_LAYERS 4

class TransparencyFilter : public AbstractFilter
{
public:
    TransparencyFilter();
    virtual ~TransparencyFilter();

    const char* getName() const { return "TransparencyFilter"; }
    const Content& onApply(const Content& ref);
    HWCString dump();

private:
    class DetectionThread;
    class DetectionItem
    {
        friend TransparencyFilter;
    public:
        DetectionItem();
        virtual ~DetectionItem();
        void reset();
        void updateRepeatCounts(const Layer& ly);
        void initiateDetection(const Layer& layer, HwcRect<float> videoRect);
        void filterLayers(Content& ref);
        void garbageCollect(void);
        HWCString dump();
    private:
#ifdef uncomment
        AbstractBufferManager&  mBM;
        buffer_handle_t         mCurrentHandle;     // Handle of the currently repeating frame
        hwc_rect                mBlackMask;
#endif
        uint32_t                mRepeatCount;
        bool                    mbEnabled;
#ifdef uncomment
       std::sp<GraphicBuffer>       mpLinearBuffer;
#endif
        uint32_t                mFramesBeforeCheck;
#ifdef uncomment
        std::sp<DetectionThread>     mpDetectionThread;
#endif
        bool                    mbFirstEnabledFrame;
        bool                    mbFirstDisabledFrame;
    };

    void skipFilter(void);

    DetectionItem       mDetection[MAX_DETECT_LAYERS];
    uint32_t            mDetectionNum;
    Content             mReference;
};

};
//}; // namespace hwc
//}; // namespace ufo
//}; // namespace intel

#endif // INTEL_COMMON_HWC_TRANSPARENCYFILTER_H
