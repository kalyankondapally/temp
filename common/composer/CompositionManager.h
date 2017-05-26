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

#ifndef COMMON_HWC_COMPOSITION_MANAGER_H
#define COMMON_HWC_COMPOSITION_MANAGER_H

#include <vector>
#include <mutex>
#include <map>
#include <bitset>

#include "AbstractComposer.h"
#include "AbstractComposition.h"
#include "AbstractBufferManager.h"
#include "BufferQueue.h"
#include "HwcList.h"
#include "SurfaceFlingerComposer.h"
#include "platformdefines.h"

#ifdef uncomment
#include <ui/GraphicBuffer.h>
#endif
#include "singleton.h"

//namespace intel {
//namespace ufo {
//namespace hwc {
namespace hwcomposer {

class Hwc;

// This class manages an array of possible compositions. The array is used for two things, firstly an assessment of the cost
// of a particular composition and secondly to actually store the results of a composition.
// The usage model is that you can request a composition, which will set up internal state relevant to that composition.
// You can then request a cost evaluation of that composition (a number of different costs are supported) if you need it
// Every frame after, you must call updateComposition to keep the composition alive and to verify whether the handles
// changed (which will invalidate any existing composition results).
// If you decide to use that composition, you can call performComposition to make it actually happen. Multiple calls to
// performComposition will not result in multiple compositions unless the handles have changed.
class CompositionManager : public Singleton<CompositionManager>, public AbstractBufferManager::Tracker
{
public:

    CompositionManager();
    ~CompositionManager();

    // One-time initialise during on first frame.
    void firstFrameInit( void );

    void add(AbstractComposer* pComposer)
    {
        mpComposers.push_back(pComposer);
    }
#ifdef uncommenthwc1
    void onPrepareBegin(size_t numDisplays, hwc_display_contents_1_t** displays, nsecs_t timestamp);
#endif
    void onPrepareEnd();
    void onAccept(const Content::Display& display, uint32_t d);
#ifdef uncommenthwc1
    void onSetBegin(size_t numDisplays, hwc_display_contents_1_t** ppDisplayContents);
#endif
    void onEndOfFrame( uint32_t hwcFrameIndex );

    // Request a composition of the source layer stack to the requested resolution.
    // Once a composition has been requested for a frame then the src layers must remain available (must not be deleted or modified).
    AbstractComposition* requestComposition(const Content::LayerStack& src, uint32_t width, uint32_t height, uint32_t format, ECompressionType compression, AbstractComposer::Cost type = AbstractComposer::Power);

    // Compositions may be removed automatically if they have not been used for a while.
    // To prevent this, use onLock/onUnlock.
    // Returns new lock count.
    uint32_t lockComposition( AbstractComposition* pComposition );
    uint32_t unlockComposition( AbstractComposition* pComposition );

    // Last resort composition. This HAS to succeed.
    AbstractComposition* fallbackToSurfaceFlinger(uint32_t display)
    {
        return mSurfaceFlingerComposer.handleAllLayers(display);
    }

    // Request to perform a composition now into the specified target layer.
    // This effectively bundles up the whole composition process from request, through to completion of the composition.
    // The src layers do not have to remain available.
    bool performComposition(const Content::LayerStack& src, const Layer& target);


    // TODO: Evaluations
    // Evaluate the cost of a composition. This is a once off call, it will sum up the cost of all the requested
    // compositions since the last evaluation or reset
    //uint32_t performEvaluation(Layer& target, AbstractComposer::Cost cost, uint32_t *pCompositionCost);

    // Reset the evaluation state, note that performEvaluation does this automatically
    //void resetEvaluation();

    // Implements AbstractBufferManager::Tracker
    virtual void notifyBufferAlloc( HWCNativeHandle handle );
    virtual void notifyBufferFree( HWCNativeHandle handle );

    // Dump a little info about all the composer
    HWCString dump() const;

private:
    friend class Singleton<CompositionManager>;

    class Composition;
    friend Composition;

    // Internal function to search for the best composition engine for a particular composition
    void chooseBestCompositionEngine(Composition& c, AbstractComposer::Cost type);

    // Expire buffers - drain the mStaleBufferHandles list.
    void expireBuffers( void );

    // These accessors used from composition class
    nsecs_t                         getTimestamp() const        { return mTimestamp;   }
    BufferQueue&                    getBufferQueue()            { return mBufferQueue; }

    // Invalidate any compositions containing this buffer handle
    void                            invalidate(HWCNativeHandle handle);

private:
    HwcList<Composition>            mCompositions;              // List of currently active compositions
    std::vector<AbstractComposer*>  mpComposers;

    SurfaceFlingerComposer          mSurfaceFlingerComposer;    // Composer that manages surfaceflinger compositions
    BufferQueue                     mBufferQueue;               // Currently allocated Composition buffers

    std::vector<HWCNativeHandle>    mStaleBufferHandles;        // List of buffer handles that we know have been freed.
    std::mutex                      mStaleBufferMutex;          // Mutex for thread safe access to the stale buffer handle list.

    std::vector<HWCNativeHandle>    mCurrentHandles[cMaxSupportedPhysicalDisplays];// List of buffer handles that we know have been freed.
    std::map<HWCNativeHandle, std::bitset<cMaxSupportedPhysicalDisplays>> mCurrentHandleUsage;

    pid_t                           mPrimaryTid;                // Primary thread.
    nsecs_t                         mTimestamp;                 // Time of the most recent composition
};

};
//}; // namespace hwc
//}; // namespace ufo
//}; // namespace intel

#endif // COMMON_HWC_COMPOSITION_MANAGER_H
