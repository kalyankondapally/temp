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

#ifdef uncomment
#include "Hwc.h"
#endif
#include "SurfaceFlingerComposer.h"
#include "AbstractBufferManager.h"
#include "filtermanager.h"
#include "log.h"
#include "utils.h"
#ifdef uncomment
#include <ufo/graphics.h>
#endif

//namespace intel {
//namespace ufo {
//namespace hwc {
namespace hwcomposer {

// Maximum layers the HWC handles. Any layers in excess of this need to be considered as unsupported
const static unsigned int cMaxLayers = 64;

SurfaceFlingerComposer::SurfaceFlingerComposer() :
    mTimestamp(0),
    mNumDisplays(0)
#ifdef uncomment
    mppDisplayContents(NULL)
#endif
{
    FilterManager::getInstance().add(*this, FilterPosition::SurfaceFlinger);
}

SurfaceFlingerComposer::~SurfaceFlingerComposer()
{
}

const char* SurfaceFlingerComposer::getName() const
{
    return "SurfaceFlingerComposer";
}

static bool isLayerSupported(const Layer& layer)
{
#ifdef uncomment
    // Hwc can never support skip layers
    if (layer.getFlags() & HWC_SKIP_LAYER)
    {
        DTRACEIF(COMPOSITION_DEBUG, "%s Unsupported SKIP %s", __FUNCTION__, layer.dump().string());
        return false;
    }
#endif

    // Check which layer formats the HWC will attempt to handle in some way. Note, if we allow
    // an unsupportable format through at this point, it should work, however there is a chance
    // that we will have to abort the composition and hence will end up with full stack SurfaceFlinger
    // composition.
    switch (layer.getBufferFormat())
    {
        // Generic Android formats
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_BGR888:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_ARGB8888:
	case HWC_PIXEL_FORMAT_YV12:
	case DRM_FORMAT_YUYV:

        // Intel specific formats
	case HWC_PIXEL_FORMAT_NV12_LINEAR_PACKED_INTEL:
	case HWC_PIXEL_FORMAT_NV12_X_TILED_INTEL:
	case HWC_PIXEL_FORMAT_NV12_Y_TILED_INTEL:
	case HWC_PIXEL_FORMAT_NV12_LINEAR_INTEL:
	case HWC_PIXEL_FORMAT_NV12_LINEAR_CAMERA_INTEL:
	case HWC_PIXEL_FORMAT_YUV420PackedSemiPlanar_INTEL:
	case HWC_PIXEL_FORMAT_YUV420PackedSemiPlanar_Tiled_INTEL:
	case HWC_PIXEL_FORMAT_YCbCr_422_H_INTEL:
	case HWC_PIXEL_FORMAT_GENERIC_8BIT_INTEL:
	case HWC_PIXEL_FORMAT_YCbCr_420_H_INTEL:
	case HWC_PIXEL_FORMAT_YCbCr_411_INTEL:
	case HWC_PIXEL_FORMAT_YCbCr_422_V_INTEL:
	case HWC_PIXEL_FORMAT_YCbCr_444_INTEL:
	case HWC_PIXEL_FORMAT_P010_INTEL:
            // Format is supported by at least one of our composers
            break;

        default:
            DTRACEIF(COMPOSITION_DEBUG, "%s Unsupported format %s", __FUNCTION__, layer.dump().string());
            return false;

    }

    return true;
}

// This function initialises the unsupported range of layers for a display. The SF composer can only compose
// a single sequential set of layers. Hence, if we have multiple unsupported, we have to request that SF
// composes anything between those unsupported layers
void SurfaceFlingerComposer::findUnsupportedLayerRange(uint32_t d, const Content::Display& display)
{
    const Content::LayerStack& layerstack = display.getLayerStack();
    int32_t min = -1;
    int32_t max = -1;

    for (uint32_t ly = 0; ly < layerstack.size(); ly++)
    {
        if (!isLayerSupported(layerstack.getLayer(ly)))
        {
            if (min < 0)
                min = ly;
            max = ly;
        }
    }

    if (layerstack.size() > cMaxLayers)
    {
        // If we exceed max layers, then we need to set a min of maxLayers-1 as we need to
        // store a render target in the input layer list.
        if (min < 0)
            min = cMaxLayers-1;
        max = layerstack.size();
    }

    mCompositions[d].mUnsupportedMin = min;
    mCompositions[d].mUnsupportedMax = max;
    return;
}

// AbstractFilter entrypoint. This function needs to identify which layers the HWC doesnt support and replace them in
// reference list with the composed render target.
const Content& SurfaceFlingerComposer::onApply(const Content& ref)
{
    HWCASSERT(ref.size() <= cMaxSupportedSFDisplays);

    // Run through each display
    bool bUnsupportedLayers = false;
    for (uint32_t d = 0; d < ref.size(); d++)
    {
        const Content::Display& in = ref.getDisplay(d);

        if (!in.isEnabled())
            continue;

        if (in.isGeometryChanged())
        {
            // Calculate the unsupported range. It may change on any geometry change.
            findUnsupportedLayerRange(d, in);
        }

        // Check all displays for any unsupported layers
        bUnsupportedLayers |= (mCompositions[d].mUnsupportedMax != -1);
    }

    // If all layers are supported on all displays, then just return the input ref, nothing to do.
    if (!bUnsupportedLayers)
        return ref;

    // If not, need to update our local content ref as appropriate and pass it on to the next filter
    DTRACEIF(COMPOSITION_DEBUG, "SurfaceFlingerComposer: onApply Unsupported Layers seen, generating a content ref");

    // Copy the content
    mOutRef = ref;

    for (uint32_t d = 0; d < ref.size(); d++)
    {
        const Content::Display& in = ref.getDisplay(d);

        if (!in.isEnabled())
            continue;

        // If there are any unsupported layers then remove them from the layer list.
        int32_t max = mCompositions[d].mUnsupportedMax;
        if (max < 0)
            continue;

        int32_t min = mCompositions[d].mUnsupportedMin;
        Content::Display& out = mOutRef.editDisplay(d);

        // If something has changed that requires a downstream geometry change, then trigger it now
        if (mCompositions[d].mbForceGeometryChange)
        {
            out.setGeometryChanged(true);
            mCompositions[d].mbForceGeometryChange = false;
        }

        // Remove the layers that the HWC cannot support at all
        Content::LayerStack& layerstack = out.editLayerStack();

        // Leave the layer "min" alone, we will reuse as a render target
        for (int32_t ly = min+1; ly <= max; ly++)
        {
            DTRACEIF(COMPOSITION_DEBUG, "SurfaceFlingerComposer: onApply Remove Layer %d", ly);
            DTRACEIF( layerstack.getLayer( min+1 ).isFrontBufferRendered(),
                      "SurfaceFlinger will compose front buffer rendered layer %s",
                      layerstack.getLayer( min+1 ).dump().string() );
            layerstack.removeLayer(min+1);
        }

        // Add the composed render target as a source layer
        mCompositions[d].onUpdatePending(mTimestamp);
        DTRACEIF(COMPOSITION_DEBUG, "SurfaceFlingerComposer: onApply Set Layer %d to %s", min, mCompositions[d].getRenderTarget().dump().string());
        layerstack.setLayer(min, &mCompositions[d].getRenderTarget());
        layerstack.updateLayerFlags();
    }

    return mOutRef;
}

int SurfaceFlingerComposer::findMatch(const Content::LayerStack& source, int32_t &min, int32_t &max) const
{
    // We can support this composition if:
    // 1) We have no unsupported layers on a display and the source layers match the SF input layers
    // 2) We have unsupported layers and the layerstack contains those along with adjacent matching layers

    for ( uint32_t d = 0; d < mNumDisplays; d++ )
    {
#ifdef uncomment
        hwc_display_contents_1_t* pDisplayContents = mppDisplayContents[d];
        if (pDisplayContents == NULL)
            continue;
#endif
        // Check for an existing allocation
        if (mCompositions[d].mComposeMax >= 0)
            continue;
        min = -1;
        max = -1;
        bool bMatchedUnsupported = false;

        // first layer is special. Either we have a match to this display's composition
        // or we have to search for the first matching layer
        const Layer& layer = source[0];
        if (layer.getComposition() == &mCompositions[d])
        {
            min = mCompositions[d].mUnsupportedMin;
            max = mCompositions[d].mUnsupportedMax;
            bMatchedUnsupported = true;
        }
        else
        {
#ifdef uncomment
            for (uint32_t ly = 0; ly < pDisplayContents->numHwLayers-1; ly++)
            {
                if (layer == pDisplayContents->hwLayers[ly])
                {
                    min = ly;
                    max = ly;
                    break;
                }
            }
#endif
            // If we didnt match the first layer, try next display
            if (max < 0)
                continue;
        }

        // If we get this far, we matched the first layer and have our min setup correct.
        // Figure out the max
        for (uint32_t ly = 1; ly < source.size(); ly++)
        {
            const Layer& layer = source[ly];

            // Check to see if this layer is the current display's composition
            if (layer.getComposition() == &mCompositions[d])
            {
                max = mCompositions[d].mUnsupportedMax;
                bMatchedUnsupported = true;
            }
            else
            {
#ifdef uncomment
                int maxCandidate = max+1;
                if ((maxCandidate < (int)pDisplayContents->numHwLayers-1)
                    && (layer == pDisplayContents->hwLayers[maxCandidate]))
                {
                    max = maxCandidate;
                }
                else
                {
                    max = -1;
                    break;
                }
#endif
            }
        }
        // If we failed a match, try next display
        if (max < 0)
            continue;
#ifdef uncomment
        // We now have a min/max layer
        if (mCompositions[d].mUnsupportedMax >= 0)
        {
            if (bMatchedUnsupported == false)
            {
                // If we have an unsupported composition, that wasnt included in our list, then fail.
                break;
            }
        }
#endif
        return d;
    }

    // Didn't manage to match a display.
    return -1;
}

float SurfaceFlingerComposer::onEvaluate(const Content::LayerStack& source, const Layer& target, AbstractComposer::CompositionState** ppState, Cost type)
{
    HWCASSERT(source.size() > 0);
    HWC_UNUSED(ppState);

    DTRACEIF(COMPOSITION_DEBUG, "SurfaceFlingerComposer::%s %s", __FUNCTION__, source.dump().string());

    // Check that the target is the default format
    if (target.getBufferFormat() != INTEL_HWC_DEFAULT_HAL_PIXEL_FORMAT)
    {
        DTRACEIF(COMPOSITION_DEBUG, "SurfaceFlingerComposer: Unsupported output format: %s", target.dump().string());
        return Eval_Not_Supported;
    }

    // And ensure that there are no encrypted source layers
    for (uint32_t ly = 0; ly < source.size(); ly++)
    {
        const Layer& layer = source[ly];

        if (layer.isEncrypted())
        {
            DTRACEIF(COMPOSITION_DEBUG, "SurfaceFlingerComposer: Unsupported input encrypted %d: %s", ly, layer.dump().string());
            return Eval_Not_Supported;
        }
    }

    int min = -1;
    int max = -1;
    int display = findMatch(source, min, max);
    if (display < 0)
    {
        return Eval_Not_Supported;
    }

    // We finally matched. Calculate cost
    int cost = Eval_Not_Supported;
    switch (type)
    {
        case Bandwidth:
        case Power:         // TODO: Implement, for now, default to bandwidth
        case Performance:   // TODO: Implement, for now, default to bandwidth
        case Quality:       // TODO: Implement, for now, default to bandwidth
        {
            int dstBandwidth = calculateBandwidthInKilobytes(target.getDstWidth(), target.getDstHeight(), target.getBufferFormat());
            int bandwidth = dstBandwidth; // For the glClear
            for (uint32_t ly = 0; ly < source.size(); ly++)
            {
                const Layer& layer = source[ly];
                // 1 read of source, 1 read and 1 write of destination per layer
                bandwidth += calculateBandwidthInKilobytes(layer.getSrcWidth(), layer.getSrcHeight(), layer.getBufferFormat());
                bandwidth += 2 * dstBandwidth;
            }
            cost = bandwidth * target.getFps();                                         // Times the frames per second
        }
        break;
    case Memory:
        // No additional memory needs to be allocated at this time, it makes use of memory already committed by SF
        cost = Eval_Cost_Min;
        break;
    }

    DTRACEIF(COMPOSITION_DEBUG, "SurfaceFlingerComposer: Evaluation cost(%d) = %d", type, cost);
    return cost;
}

void SurfaceFlingerComposer::onCompose(const Content::LayerStack& source, const Layer&, AbstractComposer::CompositionState* pState)
{
    ATRACE_NAME_IF(RENDER_TRACE, "SurfaceFlingerComposer");
    HWC_UNUSED(pState);
#ifdef uncomment
    // Nothing much to do on this call for this composer, the composition will have already been performed by SF
    Log::add(source, "SurfaceFlingerComposer ");
#endif
    return;
}

#ifdef uncomment
void SurfaceFlingerComposer::onPrepareBegin(size_t numDisplays, hwc_display_contents_1_t** ppDisplayContents, nsecs_t frameTime)
{
    mTimestamp = frameTime;
    mNumDisplays = numDisplays;
    mppDisplayContents = ppDisplayContents;

    for (uint32_t d = 0; d < mNumDisplays; d++)
    {
        hwc_display_contents_1_t* pDisplayContents = mppDisplayContents[d];
        if (pDisplayContents == NULL)
            continue;

        if (pDisplayContents->flags & HWC_GEOMETRY_CHANGED)
        {
            // Update the render target layer
            mCompositions[d].onUpdateAll(pDisplayContents->hwLayers[pDisplayContents->numHwLayers-1], frameTime);
        }
    }
}
#endif

void SurfaceFlingerComposer::onPrepareEnd()
{
    for (uint32_t d = 0; d < mNumDisplays; d++)
    {
        bool bSFRTRequired = false;
#ifdef uncomment
        hwc_display_contents_1_t* pDisplayContents = mppDisplayContents[d];
        if ( pDisplayContents )
        {
            int32_t composeMin = mCompositions[d].composeMin();
            int32_t composeMax = mCompositions[d].composeMax();

            // Update the composition state flags if anything has changed
            if (pDisplayContents->flags & HWC_GEOMETRY_CHANGED ||
                mCompositions[d].mLastComposedMin != composeMin ||
                mCompositions[d].mLastComposedMax != composeMax)
            {
                // By default, this composer claims everything as being handled. During onPrepareEnd(), we
                // will mark anything requiring composition as FB.
                for (int32_t ly = 0; ly < int32_t(pDisplayContents->numHwLayers-1); ly++)
                {
                    if (ly >= composeMin && ly <= composeMax)
                        pDisplayContents->hwLayers[ly].compositionType = HWC_FRAMEBUFFER;
                    else
                        pDisplayContents->hwLayers[ly].compositionType = HWC_OVERLAY;
                }
                mCompositions[d].mLastComposedMin = composeMin;
                mCompositions[d].mLastComposedMax = composeMax;
            }

            bSFRTRequired = ( composeMin != -1 ) || ( composeMax != -1 );
        }
#endif
        if ( bSFRTRequired )
        {
            AbstractBufferManager::get().realizeSurfaceFlingerRenderTargets( d );
        }
        else
        {
            AbstractBufferManager::get().purgeSurfaceFlingerRenderTargets( d );
        }
    }
}

#ifdef uncomment
void SurfaceFlingerComposer::onSet(size_t numDisplays, hwc_display_contents_1_t** ppDisplayContents, nsecs_t frameTime)
{

    mNumDisplays = numDisplays;
    mppDisplayContents = ppDisplayContents;

    for ( uint32_t d = 0; d < numDisplays; d++ )
    {
        hwc_display_contents_1_t* pDisplayContents = mppDisplayContents[d];
        if (pDisplayContents == NULL)
            continue;

        const buffer_handle_t surfaceFlingerRT = pDisplayContents->hwLayers[pDisplayContents->numHwLayers-1].handle;
        if ( surfaceFlingerRT )
        {
            // Tag SF RTs via BufferManager.
            AbstractBufferManager::get().setSurfaceFlingerRT( surfaceFlingerRT, d );
        }

        if (mCompositions[d].composeMax() >= 0)
        {
            // Minimal update of just the per frame state
            mCompositions[d].onUpdateFrameState(pDisplayContents->hwLayers[pDisplayContents->numHwLayers-1], frameTime);
            DTRACEIF(COMPOSITION_DEBUG, "SurfaceFlingerComposer: onSet Updated Display %d RenderTarget to %s", d, mCompositions[d].getRenderTarget().dump().string());
        }
        else
        {
            // Clean up any fence passed in when we know we arnt using this render target
            hwc_layer_1& rt = pDisplayContents->hwLayers[pDisplayContents->numHwLayers-1];
            DTRACEIF( COMPOSITION_DEBUG, "SurfaceFlingerComposer: onSet Closing unused fence %d", rt.acquireFenceFd);
            Timeline::closeFence(&rt.acquireFenceFd);
            rt.acquireFenceFd = -1;
        }
    }
}
#endif

AbstractComposition* SurfaceFlingerComposer::handleAllLayers(uint32_t d)
{
    HWCASSERT(d < mNumDisplays);
    mCompositions[d].mUnsupportedMin = 0;
#ifdef uncomment
    mCompositions[d].mUnsupportedMax = mppDisplayContents[d]->numHwLayers-2;
#endif
    mCompositions[d].mbForceGeometryChange = true;

    DTRACEIF(COMPOSITION_DEBUG, "SFC handleAllLayers %s", mCompositions[d].dump().string());

    return &mCompositions[d];
}

AbstractComposer::ResourceHandle SurfaceFlingerComposer::onAcquire(const Content::LayerStack& source, const Layer& target)
{
    HWC_UNUSED(target);

    int min = -1;
    int max = -1;
    int display = findMatch(source, min, max);
    if (display < 0)
    {
        return NULL;
    }
    Composition &c = mCompositions[display];
    if ((c.mUnsupportedMin >= 0) && (c.mUnsupportedMax >= 0))
    {
        if (c.mUnsupportedMin < min)
        {
            min = c.mUnsupportedMin;
        }
        if (c.mUnsupportedMax > max)
        {
            max = c.mUnsupportedMax;
        }
    }
    c.mComposeMin = min;
    c.mComposeMax = max;
    return &c;
}

void SurfaceFlingerComposer::onRelease(ResourceHandle hResource)
{
    if (hResource)
    {
        Composition* pC = static_cast<Composition*>(hResource);
        pC->mComposeMin = -1;
        pC->mComposeMax = -1;
    }
}

const Layer& SurfaceFlingerComposer::getTarget(ResourceHandle hResource)
{
    HWCASSERT(hResource);
    Composition* pC = static_cast<Composition*>(hResource);
    return pC->getTarget();
}

HWCString SurfaceFlingerComposer::dump()
{
    return HWCString("");
}

SurfaceFlingerComposer::Composition::Composition() :
    mUnsupportedMin(-1),
    mUnsupportedMax(-1),
    mComposeMin(-1),
    mComposeMax(-1),
    mLastComposedMin(-1),
    mLastComposedMax(-1),
    mRenderTargetFormat(INTEL_HWC_DEFAULT_HAL_PIXEL_FORMAT),
    mRenderTargetTilingFormat(TILE_X),
    mbForceGeometryChange(false)
{
#ifdef uncomment
    mRenderTarget.setComposition(this);
#endif
}

SurfaceFlingerComposer::Composition::~Composition()
{
}

void SurfaceFlingerComposer::Composition::onUpdatePending(nsecs_t frameTime)
{
    mRenderTarget.onUpdatePending(frameTime);
    mRenderTarget.setBufferFormat(mRenderTargetFormat);
    mRenderTarget.setBufferTilingFormat(mRenderTargetTilingFormat);
    mRenderTarget.setBlending(composeMin() == 0 ? EBlendMode::NONE : EBlendMode::PREMULT);
#ifdef uncomment
    mRenderTarget.setComposition(this);
#endif
    mRenderTarget.setBufferCompression(AbstractBufferManager::get().getSurfaceFlingerCompression());
    mRenderTarget.onUpdateFlags();
    DTRACEIF(COMPOSITION_DEBUG, "SF Composer onUpdatePending %s", mRenderTarget.dump().string());
}

#ifdef uncomment
void SurfaceFlingerComposer::Composition::onUpdateFrameState(hwc_layer_1& layer, nsecs_t frameTime)
{
    mRenderTarget.onUpdateFrameState(layer, frameTime);
    mRenderTargetFormat = mRenderTarget.getBufferFormat();
    mRenderTargetTilingFormat = mRenderTarget.getBufferTilingFormat();
    mRenderTarget.setBlending(composeMin() == 0 ? EBlendMode::NONE : EBlendMode::PREMULT);
    mRenderTarget.setComposition(this);
    mRenderTarget.onUpdateFlags();
    DTRACEIF(COMPOSITION_DEBUG, "SF Composer onUpdateFrameState S: %s", printLayer(layer).string());
    DTRACEIF(COMPOSITION_DEBUG, "SF Composer onUpdateFrameState RT:%s", mRenderTarget.dump().string());
}

void SurfaceFlingerComposer::Composition::onUpdateAll(hwc_layer_1& layer, nsecs_t frameTime)
{
    mRenderTarget.onUpdateAll(layer, frameTime);
    mRenderTarget.setBufferFormat(mRenderTargetFormat);
    mRenderTarget.setBlending(composeMin() == 0 ? EBlendMode::NONE : EBlendMode::PREMULT);
    mRenderTarget.setBufferCompression(AbstractBufferManager::get().getSurfaceFlingerCompression());
    mRenderTarget.setComposition(this);
    DTRACEIF(COMPOSITION_DEBUG, "SF Composer onUpdateAll S: %s", printLayer(layer).string());
    DTRACEIF(COMPOSITION_DEBUG, "SF Composer onUpdateAll RT:%s", mRenderTarget.dump().string());
}
#endif

void SurfaceFlingerComposer::Composition::onUpdate(const Content::LayerStack&)
{
    DTRACEIF(COMPOSITION_DEBUG, "SF Composer onUpdate %s", mRenderTarget.dump().string());
    // Nothing to do here
}

void SurfaceFlingerComposer::Composition::onUpdateOutputLayer(const Layer&)
{
    DTRACEIF(COMPOSITION_DEBUG, "SF Composer onUpdateOutputLayer %s", mRenderTarget.dump().string());
#if FORCE_HWC_COPY_FOR_VIRTUAL_DISPLAYS
    // NOTE: We should trigger a RT->target composition here.
    // However: the only way we SHOULD get here is if we don't have any
    // composers available... So we can't.
    ALOGW("No composers available for required composition!");
#endif
}

void SurfaceFlingerComposer::Composition::onCompose()
{
    DTRACEIF(COMPOSITION_DEBUG, "SF Composer onCompose Already Composed to: %s", mRenderTarget.dump().string());
    // Nothing to do here, SF has already done this
}

const Layer& SurfaceFlingerComposer::Composition::getTarget()
{
    return mRenderTarget;
}

const char* SurfaceFlingerComposer::Composition::getName() const
{
    return "SF Composer";
}

bool SurfaceFlingerComposer::Composition::onAcquire()
{
    return true;
}

void SurfaceFlingerComposer::Composition::onRelease()
{
}

HWCString SurfaceFlingerComposer::Composition::dump(const char* pIdentifier) const
{
    return HWCString::format("%s SF Composer Layers %d to %d", pIdentifier, composeMin(), composeMax());
}

};
//}; // namespace hwc
//}; // namespace ufo
//}; // namespace intel
