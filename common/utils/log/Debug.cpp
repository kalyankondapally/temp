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

#include "layer.h"
#include "format.h"
#include "timing.h"

#include <drm_fourcc.h>

namespace hwcomposer {
#ifdef uncomment_hwc1
HWCString printLayer(hwc_layer_1_t& hwc_layer)
{
#ifdef uncomment
    if (!sbInternalBuild)
	return HWCString();

    // Composition type isnt part of the Layer state. Add it explicitly
    const char* pCompositionType;
    switch (hwc_layer.compositionType)
    {
    case HWC_FRAMEBUFFER:
        pCompositionType = "FB";
        break;
    case HWC_BACKGROUND:
        pCompositionType = "BG";
        break;
    case HWC_OVERLAY:
        pCompositionType = "OV";
        break;
    case HWC_FRAMEBUFFER_TARGET:
        pCompositionType = "TG";
        break;
#if defined(HWC_DEVICE_API_VERSION_1_4)
    case HWC_SIDEBAND:
        pCompositionType = "SB";
        break;
    case HWC_CURSOR_OVERLAY:
        pCompositionType = "CS";
        break;
#endif
    default:
        pCompositionType = "  ";
        break;
    }

    Layer layer(hwc_layer);
    return layer.dump(pCompositionType);
#endif
}

/** Dump display content
 * \see Hwc::onSet
 * \see Hwc::onPrepare
 */
void dumpDisplayContents(
        const char *pIdentifier,
        hwc_display_contents_1_t* pDisp,
        uint32_t frameIndex)
{
#ifdef uncomment
    if (!sbInternalBuild)
        return;

    ALOG_ASSERT(pDisp);

    // Debug - report to logcat the status of everything that we are being asked to set.
    ALOGD("%s frame:%u retireFenceFd:%d outbuf:%p outbufAcquireFenceFd:%d flags:%x numHwLayers:%zd",
        pIdentifier, frameIndex, pDisp->retireFenceFd, pDisp->outbuf, pDisp->outbufAcquireFenceFd, pDisp->flags, pDisp->numHwLayers);

    for (size_t l = 0; l < pDisp->numHwLayers; l++)
    {
        hwc_layer_1_t& layer = pDisp->hwLayers[l];

	HWCString debug = printLayer(layer);
        ALOGD(" %zd %s", l, debug.string());
    }
#endif
}

void dumpDisplaysContents(
        const char *pIdentifier,
        size_t numDisplays,
        hwc_display_contents_1_t** displays,
        uint32_t frameIndex)
{
#ifdef uncomment
    if (!sbInternalBuild)
        return;

    for (size_t d = 0; d < numDisplays; d++)
    {
        hwc_display_contents_1_t* pDisp = displays[d];
        if (!pDisp)
            continue;
	dumpDisplayContents(HWCString::format("%s Display:%zd", pIdentifier, d), pDisp, frameIndex);
    }
#endif
}
#endif

const char* getDRMFormatString( int32_t drmFormat )
{
#define DRM_FMT_CASE(F)  case DRM_FORMAT_##F : return #F;
    switch ( drmFormat )
    {
        // Formats supported with gralloc HAL mappings at time of writing:
        DRM_FMT_CASE( ABGR8888    )
        DRM_FMT_CASE( XBGR8888    )
        DRM_FMT_CASE( ARGB8888    )
        DRM_FMT_CASE( BGR888      )
        DRM_FMT_CASE( RGB565      )
        DRM_FMT_CASE( NV12        )
        DRM_FMT_CASE( YUYV        )
        // Misc variants:
        DRM_FMT_CASE( RGB888      )
        DRM_FMT_CASE( XRGB8888    )
        DRM_FMT_CASE( RGBX8888    )
        DRM_FMT_CASE( BGRX8888    )
        DRM_FMT_CASE( RGBA8888    )
        DRM_FMT_CASE( BGRA8888    )
        DRM_FMT_CASE( YVYU        )
        DRM_FMT_CASE( UYVY        )
        DRM_FMT_CASE( VYUY        )
        DRM_FMT_CASE( XRGB2101010 )
        DRM_FMT_CASE( XBGR2101010 )
        DRM_FMT_CASE( RGBX1010102 )
        DRM_FMT_CASE( BGRX1010102 )
        DRM_FMT_CASE( ARGB2101010 )
        DRM_FMT_CASE( ABGR2101010 )
        DRM_FMT_CASE( RGBA1010102 )
        DRM_FMT_CASE( BGRA1010102 )
    default:
	return HWCString::format("?=%x(%c%c%c%c)", drmFormat, ((drmFormat >> 0) & 0xff), ((drmFormat >> 8) & 0xff), ((drmFormat >> 16) & 0xff), ((drmFormat >> 24) & 0xff));
    }
#undef DRM_FMT_CASE
}

const char* getTilingFormatString( ETilingFormat tileFormat )
{
    switch(tileFormat)
    {
        case TILE_UNKNOWN: return "?";
        case TILE_LINEAR: return "L";
        case TILE_X: return "X";
        case TILE_Y: return "Y";
        case TILE_Yf: return "Yf";
        case TILE_Ys: return "Ys";
        default:
            return "?";
    }
}

static const char* getDataSpaceStandard( EDataSpaceStandard standard )
{
    switch (standard)
    {
    case EDataSpaceStandard::Unspecified              : return "Unsp";
    case EDataSpaceStandard::BT709                    : return "709";
    case EDataSpaceStandard::BT601_625                : return "601";
    case EDataSpaceStandard::BT601_625_UNADJUSTED     : return "601u";
    case EDataSpaceStandard::BT601_525                : return "601_525";
    case EDataSpaceStandard::BT601_525_UNADJUSTED     : return "601u525";
    case EDataSpaceStandard::BT2020                   : return "2020";
    case EDataSpaceStandard::BT2020_CONSTANT_LUMINANCE: return "2020C";
    case EDataSpaceStandard::BT470M                   : return "470M";
    case EDataSpaceStandard::FILM                     : return "FILM";
    }
    return "UNKNOWN";
}

static const char* getDataSpaceTransfer(EDataSpaceTransfer transfer)
{
    switch(transfer)
    {
    case EDataSpaceTransfer::Unspecified: return "Unsp:";
    case EDataSpaceTransfer::Linear     : return "L:";
    case EDataSpaceTransfer::SRGB       : return "sRGB:";
    case EDataSpaceTransfer::SMPTE_170M : return "";
    case EDataSpaceTransfer::GAMMA2_2   : return "G22:";
    case EDataSpaceTransfer::GAMMA2_8   : return "G28:";
    case EDataSpaceTransfer::ST2084     : return "ST2084:";
    case EDataSpaceTransfer::HLG        : return "HLG:";
    }
};

static const char* getDataSpaceRange(EDataSpaceRange range)
{
    switch(range)
    {
    case EDataSpaceRange::Unspecified: return "U";
    case EDataSpaceRange::Full       : return "F";
    case EDataSpaceRange::Limited    : return "L";
    }
    return "UNKNOWN";
};


HWCString getDataSpaceString( DataSpace dataspace )
{
    return HWCString::format("%s:%s%s", getDataSpaceStandard(dataspace.standard), getDataSpaceTransfer(dataspace.transfer), getDataSpaceRange(dataspace.range));
}

HWCString Timing::dumpRatio(EAspectRatio t)
{
    return (uint32_t)t ? HWCString::format(" %d:%d", (uint32_t)t >> 16, (uint32_t)t & 0xffff) : HWCString("");
}

HWCString Timing::dump() const
{
    return HWCString::format("%dx%d%s %s%dHz%s%s %d.%dMHz (%ux%u)", mWidth, mHeight,
            mFlags & Flag_Interlaced ? "i" : "",
	    (mMinRefresh != mRefresh) ? HWCString::format("%d-", mMinRefresh) : "",
            mRefresh,
            dumpRatio(mRatio).string(),
            mFlags & Flag_Preferred ? " Preferred" : "",
            mPixelClock/1000, (mPixelClock/100) % 10,
            mHTotal, mVTotal);
}

}; // namespace hwccomposer
