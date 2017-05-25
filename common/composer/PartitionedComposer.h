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

#ifndef COMMON_HWC_PARTITIONEDCOMPOSER_H
#define COMMON_HWC_PARTITIONEDCOMPOSER_H

#include "base.h"
#include "AbstractComposer.h"
#include "option.h"
#ifdef uncomment
#include <ui/Region.h>
#endif

#include <memory>

class HwcContext;

//namespace intel {
//namespace ufo {
//namespace hwc {
namespace hwcomposer {

class PartitionedComposer : NonCopyable, public AbstractComposer
{
public:
    class CellComposer
    {
    public:
        CellComposer() {}
        virtual ~CellComposer() {}

        virtual bool beginFrame(const Content::LayerStack& source, const Layer& target) = 0;
#ifdef uncomment
        virtual bool drawLayerSet(uint32_t numIndices, const uint32_t* pIndices, const Region& region) = 0;
#endif
        virtual bool endFrame() = 0;

        virtual bool isLayerSupportedAsInput(const Layer& layer) = 0;
        virtual bool isLayerSupportedAsOutput(const Layer& layer) = 0;

        virtual bool canBlankUnsupportedInputLayers() = 0;
    };

    PartitionedComposer(std::shared_ptr<CellComposer> renderer);
    virtual ~PartitionedComposer();

    virtual const char* getName() const;
    virtual float onEvaluate(const Content::LayerStack& source, const Layer& target, AbstractComposer::CompositionState** ppState, Cost type = Power);
    virtual void onCompose(const Content::LayerStack& source, const Layer& target, AbstractComposer::CompositionState* pState);
    virtual ResourceHandle onAcquire(const Content::LayerStack& source, const Layer& target);
    virtual void onRelease(ResourceHandle hResource);
private:
    std::shared_ptr<CellComposer> mpRenderer;
    Option mOptionPartitionVideo; // Allow video to video compositions.
};

};
//} // namespace hwc
//} // namespace ufo
//} // namespace intel

#endif // COMMON_HWC_PARTITIONEDCOMPOSER_H
