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

#ifndef COMMON_UTILS_UTILS_H
#define COMMON_UTILS_UTILS_H
#include <drm_fourcc.h>
#include <math.h>
#include "format.h"
#include <hwcdefs.h>

namespace hwcomposer {

// Return true if buffer format can be used for direct driving the encoder (WiDi)
inline bool isEncoderReadyVideo(int format)
{
    return format == HWC_PIXEL_FORMAT_NV12_Y_TILED_INTEL
	|| format == HWC_PIXEL_FORMAT_YUV420PackedSemiPlanar_Tiled_INTEL;
}

inline bool isVideoFormat(int format)
{
    return format == HWC_PIXEL_FORMAT_NV12_X_TILED_INTEL
	|| format == HWC_PIXEL_FORMAT_NV12_Y_TILED_INTEL
	|| format == HWC_PIXEL_FORMAT_NV12_LINEAR_INTEL
	|| format == HWC_PIXEL_FORMAT_NV12_LINEAR_CAMERA_INTEL
	|| format == HWC_PIXEL_FORMAT_NV12_LINEAR_PACKED_INTEL
	|| format == HWC_PIXEL_FORMAT_YUV420PackedSemiPlanar_INTEL
	|| format == HWC_PIXEL_FORMAT_YUV420PackedSemiPlanar_Tiled_INTEL
	|| format == HWC_PIXEL_FORMAT_P010_INTEL
	|| format == DRM_FORMAT_YUYV
	|| format == HWC_PIXEL_FORMAT_YV12;
}

inline bool isNV12(int format)
{
    return format == HWC_PIXEL_FORMAT_NV12_X_TILED_INTEL
	|| format == HWC_PIXEL_FORMAT_NV12_Y_TILED_INTEL
	|| format == HWC_PIXEL_FORMAT_NV12_LINEAR_INTEL
	|| format == HWC_PIXEL_FORMAT_NV12_LINEAR_CAMERA_INTEL
	|| format == HWC_PIXEL_FORMAT_NV12_LINEAR_PACKED_INTEL
	|| format == HWC_PIXEL_FORMAT_YUV420PackedSemiPlanar_INTEL
	|| format == HWC_PIXEL_FORMAT_YUV420PackedSemiPlanar_Tiled_INTEL;
}

inline bool isYUV422(int format)
{
    return format == DRM_FORMAT_YUYV;
}

inline bool isYUV420Planar(int format)
{
    // Our YUV420planar formats are (currently) all NV12.
    return isNV12( format );
}

inline bool mustBeYTiled(int format)
{
    return ( format == HWC_PIXEL_FORMAT_NV12_Y_TILED_INTEL )
	|| ( format == HWC_PIXEL_FORMAT_YUV420PackedSemiPlanar_Tiled_INTEL );
}

inline bool mustBeXTiled(int format)
{
    return ( format == HWC_PIXEL_FORMAT_NV12_X_TILED_INTEL );
}

inline bool mustBeLinear(int format)
{
    return ( format == HWC_PIXEL_FORMAT_NV12_LINEAR_INTEL )
	|| ( format == HWC_PIXEL_FORMAT_NV12_LINEAR_CAMERA_INTEL )
	|| ( format == HWC_PIXEL_FORMAT_NV12_LINEAR_PACKED_INTEL )
	|| ( format == HWC_PIXEL_FORMAT_YUV420PackedSemiPlanar_INTEL );
}

inline ETilingFormat formatToTiling( int format )
{
    if ( mustBeYTiled( format ) )
    {
        return TILE_Y;
    }
    if ( mustBeXTiled( format ) )
    {
        return TILE_X;
    }
    if ( mustBeLinear( format ) )
    {
        return TILE_LINEAR;
    }
    return TILE_UNKNOWN;
}

inline bool isYTile( ETilingFormat eTileFormat )
{
    return ( eTileFormat == TILE_Y )
        || ( eTileFormat == TILE_Yf )
        || ( eTileFormat == TILE_Ys );
}

inline bool isPacked(int format)
{
    return format == DRM_FORMAT_ABGR8888
	|| format == DRM_FORMAT_XBGR8888
	|| format == DRM_FORMAT_BGR888
	|| format == DRM_FORMAT_RGB565
	|| format == DRM_FORMAT_ARGB8888
	|| format == DRM_FORMAT_YUYV
	|| format == HWC_PIXEL_FORMAT_A2R10G10B10_INTEL
	|| format == HWC_PIXEL_FORMAT_A2B10G10R10_INTEL
	|| format == HWC_PIXEL_FORMAT_P010_INTEL;
}

inline bool isAlphaFormat(int format)
{
    return format == DRM_FORMAT_ABGR8888
	|| format == DRM_FORMAT_ARGB8888
	|| format == HWC_PIXEL_FORMAT_A2R10G10B10_INTEL
	|| format == HWC_PIXEL_FORMAT_A2B10G10R10_INTEL;
}

inline int equivalentFormatWithAlpha(int format)
{
    switch ( format )
    {
    case DRM_FORMAT_XBGR8888:
	return DRM_FORMAT_ABGR8888;
    default:
        break;
    }
    return format;
}

inline int bitsPerPixelForFormat(int format)
{
    switch (format) {
    case DRM_FORMAT_ARGB8888:
    case DRM_FORMAT_ABGR8888:
    case DRM_FORMAT_XBGR8888:
    case HWC_PIXEL_FORMAT_A2R10G10B10_INTEL:
    case HWC_PIXEL_FORMAT_A2B10G10R10_INTEL:
    case HWC_PIXEL_FORMAT_P010_INTEL:
        return 32;
    case DRM_FORMAT_BGR888:
    case HWC_PIXEL_FORMAT_YCbCr_444_INTEL:
        return 24;
    case DRM_FORMAT_RGB565:
    case HWC_PIXEL_FORMAT_YCrCb_422_H_INTEL: /* YV16 */
    case HWC_PIXEL_FORMAT_YCbCr_422_H_INTEL: /* YU16 */
    case HWC_PIXEL_FORMAT_YCbCr_422_V_INTEL:
    case DRM_FORMAT_YUYV:  /* deprecated */
        return 16;
    case HWC_PIXEL_FORMAT_NV12_X_TILED_INTEL:
    case HWC_PIXEL_FORMAT_NV12_Y_TILED_INTEL:
    case HWC_PIXEL_FORMAT_NV12_LINEAR_INTEL:
    case HWC_PIXEL_FORMAT_NV12_LINEAR_CAMERA_INTEL:
    case HWC_PIXEL_FORMAT_NV12_LINEAR_PACKED_INTEL:
    case HWC_PIXEL_FORMAT_YUV420PackedSemiPlanar_INTEL: /* deprecated */
    case HWC_PIXEL_FORMAT_YUV420PackedSemiPlanar_Tiled_INTEL: /* deprecated */
    case HWC_PIXEL_FORMAT_YV12:
    case HWC_PIXEL_FORMAT_YCbCr_411_INTEL:
    case HWC_PIXEL_FORMAT_YCbCr_420_H_INTEL:
        return 12;
    case HWC_PIXEL_FORMAT_GENERIC_8BIT_INTEL:
   // case HAL_PIXEL_FORMAT_Y8:
        return 8;

    default:
	WTRACE("format %d unknown, assuming 32bpp", format);
        return 32;
    }
}

inline float calculateBandwidthInKilobytes(uint32_t width, uint32_t height, int32_t format)
{
    return (float(width) * height * bitsPerPixelForFormat(format)) / 1024;
}

inline int floatToFixed16(float v)
{
    return int(v * 65536.0f);
}


inline float fixed16ToFloat(int v)
{
    return float(v) / 65536.0f;
}

inline bool isInteger(float f)
{
    return (fabs(f - round(f)) < 0.000001f);
}

// swap two int32 values
inline void swap_int32(int32_t &a, int32_t &b)
{
    int32_t tmp = a;
    a = b;
    b = tmp;
}

// swap two uint32 values
inline void swap_uint32(uint32_t &a, uint32_t &b)
{
    uint32_t tmp = a;
    a = b;
    b = tmp;
}

// Percentage difference.
inline float pctDiff( const float a, const float b )
{
    const float diff = a-b;
    const float avg  = 0.5f*(a+b);
    if ( avg == 0.0f )
        return 0.0f;
    return 100.0f * fabs( diff/avg );
}

// Safe bitmask function for 32 bitMask.
// Return the bit[idx] set to 1.
// Return 0 if idx is out of range.
inline uint32_t bitMask32(uint32_t idx)
{
    if(idx < 32)
        return ((uint32_t)1)<<idx;
    else
        return 0;
}

inline HwcRect<int> floatToIntRect (const HwcRect<float>& fr)
{
    HwcRect<int> ir;
    ir.left = fr.left;
    ir.right = fr.right;
    ir.top = fr.top;
    ir.bottom = fr.bottom;
    return ir;
}

inline HwcRect<float> intToFloatRect (const HwcRect<int>& fr)
{
    HwcRect<float> ir;
    ir.left = fr.left;
    ir.right = fr.right;
    ir.top = fr.top;
    ir.bottom = fr.bottom;
    return ir;
}

inline bool computeOverlap (const HwcRect<int> &rect1, const HwcRect<int> &rect2, HwcRect<int> *newRect)
{
    newRect->left = max(rect1.left, rect2.left);
    newRect->right = min(rect1.right, rect2.right);
    newRect->top = max(rect1.top, rect2.top);
    newRect->bottom = min(rect1.bottom, rect2.bottom);
    if (newRect->left >= newRect->right || newRect->top >= newRect->bottom)
        return false;
    return true;
}

inline void combineRect(HwcRect<float>& src, const HwcRect<float>& dst)
{
    src.left = src.left < dst.left ? src.left : dst.left;
    src.top = src.top < dst.top ? src.top : dst.top;
    src.right = src.right > dst.right ? src.right : dst.right;
    src.bottom = src.bottom > dst.bottom ? src.bottom : dst.bottom;
}

inline void computeRelativeRect( const HwcRect<float>& inCoordSpace,
				     const HwcRect<float>& outCoordSpace,
				     const HwcRect<float>& rect,
				     HwcRect<float>& dstRect)
{
    float x_ratio = (outCoordSpace.right - outCoordSpace.left) / (inCoordSpace.right - inCoordSpace.left);
    float y_ratio = (outCoordSpace.bottom - outCoordSpace.top) / (inCoordSpace.bottom - inCoordSpace.top);

    dstRect.left = outCoordSpace.left + (rect.left - inCoordSpace.left) * x_ratio;
    dstRect.right = outCoordSpace.left + (rect.right - inCoordSpace.left) * x_ratio;
    dstRect.top = outCoordSpace.top + (rect.top - inCoordSpace.top) * y_ratio;
    dstRect.bottom = outCoordSpace.top + (rect.bottom - inCoordSpace.top) * y_ratio;
}

}; // namespace hwcomposer

#endif // COMMON_UTILS_UTILS_H
