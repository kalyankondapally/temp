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

#ifndef COMMON_CORE_FORMAT_H
#define COMMON_CORE_FORMAT_H

#include <stdint.h>

#include "platformdefines.h"

namespace hwcomposer {

// Bitmasks describing the tiling capabilities of the device
enum ETilingFormat
{
    TILE_UNKNOWN    = 0,
    TILE_LINEAR     = 1 << 0,
    TILE_X          = 1 << 1,
    TILE_Y          = 1 << 2,
    TILE_Yf         = 1 << 3,
    TILE_Ys         = 1 << 4,
};

// Values describing the compression capabilities of the device
// ECompressionType is defined in platform specific code only.
// ECompressionType is forward declared here and used only with the constants
// defined here in common code.  This allows common code to compare and pass around
// values safely without knowing the enum.
enum class ECompressionType;
const ECompressionType COMPRESSION_NONE = static_cast<ECompressionType>(0);
const ECompressionType COMPRESSION_ARCH_START = static_cast<ECompressionType>(1);

// Note, define blending modes as a bitfield (for PlaneCaps support)
enum class EBlendMode : uint32_t {
    NONE          = 0, // No Blending
    PREMULT       = 1, // ONE / ONE_MINUS_SRC_ALPHA
    COVERAGE      = 2, // SRC_ALPHA / ONE_MINUS_SRC_ALPHA
};
const uint32_t BLENDING_MASK = static_cast<uint32_t>(EBlendMode::PREMULT) |
                               static_cast<uint32_t>(EBlendMode::COVERAGE);


enum class ETransform : uint32_t {
    NONE          = 0, // No transform
    FLIP_H        = 1, // flip source image horizontally
    FLIP_V        = 2, // flip source image vertically
    ROT_90        = 4, // Rotate image by 90
    ROT_180       = 3, // Rotate image by 180
    ROT_270       = 7, // Rotate image by 270
    FLIP_H_90     = 5,
    FLIP_V_90     = 6,
};
inline bool isTranspose(ETransform t)   { return static_cast<uint32_t>(t) &  static_cast<uint32_t>(ETransform::ROT_90); }
inline bool isFlipH(ETransform t)       { return static_cast<uint32_t>(t) &  static_cast<uint32_t>(ETransform::FLIP_H); }
inline bool isFlipV(ETransform t)       { return static_cast<uint32_t>(t) &  static_cast<uint32_t>(ETransform::FLIP_V); }

// Buffering mode hint flags.
enum EBufferModeFlags
{
    FRONT_BUFFER_RENDER = (1 << 0)  // Rendering may occur to the current presented buffer.
};

enum class EDataSpaceStandard : uint8_t {
    Unspecified               = 0,
    BT709                     = 1,
    BT601_625                 = 2,
    BT601_625_UNADJUSTED      = 3,
    BT601_525                 = 4,
    BT601_525_UNADJUSTED      = 5,
    BT2020                    = 6,
    BT2020_CONSTANT_LUMINANCE = 7,
    BT470M                    = 8,
    FILM                      = 9,
};

enum class EDataSpaceTransfer : uint8_t {
    Unspecified               = 0,
    Linear                    = 1,
    SRGB                      = 2,
    SMPTE_170M                = 3,
    GAMMA2_2                  = 4,
    GAMMA2_8                  = 5,
    ST2084                    = 6,
    HLG                       = 7,
};

enum class EDataSpaceRange : uint8_t {
    Unspecified               = 0,
    Full                      = 1,
    Limited                   = 2,
};

enum class EDataSpaceCustom : uint16_t {
    Unspecified               =      0,
    Arbitrary                 =      1,
    Depth                     = 0x1000,
};

struct DataSpace {
    EDataSpaceCustom   custom   : 16;
    EDataSpaceStandard standard :  6;
    EDataSpaceTransfer transfer :  5;
    EDataSpaceRange    range    :  3;
};

static inline bool operator==(const DataSpace a, const DataSpace b)
{
    return a.custom == b.custom && a.standard == b.standard && a.transfer == b.transfer && a.range == b.range;
}

static inline bool operator!=(const DataSpace a, const DataSpace b)
{
    return !(a == b);
}

// Common dataspace constants
// FIXME
/*
const DataSpace DataSpace_Unknown     { .custom = EDataSpaceCustom::Unspecified };
const DataSpace DataSpace_Arbitrary   { .custom = EDataSpaceCustom::Arbitrary };

const DataSpace DataSpace_SRGB_Linear { .standard = EDataSpaceStandard::BT709,     .transfer = EDataSpaceTransfer::Linear,     .range = EDataSpaceRange::Full };
const DataSpace DataSpace_SRGB        { .standard = EDataSpaceStandard::BT709,     .transfer = EDataSpaceTransfer::SRGB,       .range = EDataSpaceRange::Full };
const DataSpace DataSpace_JFIF        { .standard = EDataSpaceStandard::BT601_625, .transfer = EDataSpaceTransfer::SMPTE_170M, .range = EDataSpaceRange::Full };
const DataSpace DataSpace_BT601_625   { .standard = EDataSpaceStandard::BT601_625, .transfer = EDataSpaceTransfer::SMPTE_170M, .range = EDataSpaceRange::Limited };
const DataSpace DataSpace_BT601_525   { .standard = EDataSpaceStandard::BT601_525, .transfer = EDataSpaceTransfer::SMPTE_170M, .range = EDataSpaceRange::Limited };
const DataSpace DataSpace_BT709       { .standard = EDataSpaceStandard::BT709,     .transfer = EDataSpaceTransfer::SMPTE_170M, .range = EDataSpaceRange::Limited };

// Non-color
const DataSpace DataSpace_Depth       { .custom = EDataSpaceCustom::Depth };
*/

#define DRM_FORMAT_NONE				        fourcc_code('0', '0', '0', '0')
#define HWC_PIXEL_FORMAT_YV12		                fourcc_code('9', '9', '9', '7')
#define HWC_PIXEL_FORMAT_FLEX_IMPLEMENTATION_DEFINED	fourcc_code('9', '9', '9', '8')
#define HWC_PIXEL_FORMAT_FLEX_YCbCr_420_888		fourcc_code('9', '9', '9', '9')

// Intel specific formats.
enum {
    /**
     * Intel NV12 format Y tiled.
     *
     *
     * Additional layout information:
     * - stride: aligned to 128 bytes
     * - height: aligned to 64 lines
     * - tiling: always Y tiled
     *
     *       ____________w___________ ____
     *      |Y0|Y1                   |    |
     *      |                        |    |
     *      h                        h    h'= align(h,64)
     *      |                        |    |
     *      |                        |    |
     *      |____________w___________|    |
     *      :                             |
     *      :________________________ ____|
     *      |U|V|U|V                 |    :
     *     h/2                      h/2   :
     *      |____________w___________|    h"= h'/2
     *      :.............................:
     *
     *      stride = align(w,128)
     *
     */
    HWC_PIXEL_FORMAT_NV12_Y_TILED_INTEL = 0x100,

    /**
     * Intel NV12 format linear.
     *
     *
     * Additional layout information:
     * - stride: aligned to 512, 1024, 1280, 2048, 4096
     * - height: aligned to 32 lines
     * - tiling: always linear
     *
     *       ____________w___________ ____
     *      |Y0|Y1                   |    |
     *      |                        |    |
     *      h                        h    h'= align(h,32)
     *      |                        |    |
     *      |                        |    |
     *      |____________w___________|    |
     *      :                             |
     *      :________________________ ____|
     *      |U|V|U|V                 |    :
     *     h/2                      h/2   :
     *      |____________w___________|    h"= h'/2
     *      :.............................:
     *
     *      stride = align(w,512..1024..1280..2048..4096)
     *
     */
    HWC_PIXEL_FORMAT_NV12_LINEAR_INTEL = 0x101,

    /**
     * Layout information:
     * - U/V are 1/2 width and full height
     * - stride: aligned to 128
     * - height: aligned to 64
     * - tiling: always Y tiled
     *
     *       ____________w___________ ____
     *      |Y                       |    |
     *      |                        |    |
     *      h                        h    h' = align(h,64)
     *      |____________w___________|    |
     *      :                             |
     *      :_____________________________|
     *      |V           |                :
     *      |            |                :
     *      h            h                h' = align(h,64)
     *      |_____w/2____|                :
     *      :____________ ................:
     *      |U           |                :
     *      |            |                :
     *      h            h                h' = align(h,64)
     *      |_____w/2____|                :
     *      :.............................:
     *
     *      stride = align(w,128)
     *
     */
    HWC_PIXEL_FORMAT_YCrCb_422_H_INTEL = 0x102, // YV16

    /**
     * Intel NV12 format packed linear.
     *
     *
     * Additional layout information:
     * - stride: same as width
     * - height: no alignment
     * - tiling: always linear
     *
     *       ____________w___________
     *      |Y0|Y1                   |
     *      |                        |
     *      h                        h
     *      |                        |
     *      |                        |
     *      |____________w___________|
     *      |U|V|U|V                 |
     *     h/2                      h/2
     *      |____________w___________|
     *
     */
    HWC_PIXEL_FORMAT_NV12_LINEAR_PACKED_INTEL = 0x103,

    /**
     * Three planes, 8 bit Y plane followed by 8 bit 2x1 subsampled U and V planes.
     * Similar to IMC3 but U/V are full height.
     * The width must be even.
     * There are no specific restrictions on pitch, height and alignment.
     * It can be linear or tiled if required.
     * Horizontal stride is aligned to 128.
     * Vertical stride is aligned to 64.
     *
     *       ________________________ .....
     *      |Y0|Y1                   |    :
     *      |                        |    :
     *      h                        h    h'= align(h,64)
     *      |____________w___________|    :
     *      :____________ ________________:
     *      |U0|U1       |                :
     *      |            |                :
     *     h|            |                h'= align(h,64)
     *      |_____w/2____|                :
     *      :____________ ................:
     *      |V0|V1       |                :
     *      |            |                :
     *     h|            |                h'= align(h,64)
     *      |______w/2___|                :
     *      :.............................:
     *
     *      stride = align(w,128)
     */
    HWC_PIXEL_FORMAT_YCbCr_422_H_INTEL = 0x104, // YU16

    /**
     * Intel NV12 format X tiled.
     * This is VXD specific format.
     *
     *
     * Additional layout information:
     * - stride: aligned to 512, 1024, 2048, 4096
     * - height: aligned to 32 lines
     * - tiling: always X tiled
     *
     *       ____________w___________ ____
     *      |Y0|Y1                   |    |
     *      |                        |    |
     *      h                        h    h'= align(h,32)
     *      |                        |    |
     *      |                        |    |
     *      |____________w___________|    |
     *      :                             |
     *      :________________________ ____|
     *      |U|V|U|V                 |    :
     *     h/2                      h/2   :
     *      |____________w___________|    h"= h'/2
     *      :.............................:
     *
     *      stride = align(w,512..1024..2048..4096)
     *
     */
    HWC_PIXEL_FORMAT_NV12_X_TILED_INTEL = 0x105,

    /**
     * Only single 8 bit Y plane.
     * Horizontal stride is aligned to 128.
     * Vertical stride is aligned to 64.
     * Tiling is Y-tiled.
     *       ________________________ .....
     *      |Y0|Y1|                  |    :
     *      |                        |    :
     *      h                        h    h'= align(h,64)
     *      |____________w___________|    :
     *      :                             :
     *      :............stride...........:
     *
     *      stride = align(w,128)
     */
    HWC_PIXEL_FORMAT_GENERIC_8BIT_INTEL = 0x108,

    /**
     * Three planes, 8 bit Y plane followed by U, V plane with 1/4 width and full height.
     * The U and V planes have the same stride as the Y plane.
     * An width is multiple of 4 pixels.
     * Horizontal stride is aligned to 128.
     * Vertical stride is aligned to 64.
     *       ________________________ .....
     *      |Y0|Y1                   |    :
     *      |                        |    :
     *      h                        |    h'= align(h,64)
     *      |____________w___________|    :
     *      :_______ .....................:
     *      |U|U    |                     :
     *      |       |                     :
     *      h       |                     h'= align(h,64)
     *      |__w/4__|                     :
     *      :_______ .....................:
     *      |V|V    |                     :
     *      |       |                     :
     *      h       |                     h'= align(h,64)
     *      |__w/4__|                     :
     *      :............stride...........:
     *
     *      stride = align(w,128)
     */
    HWC_PIXEL_FORMAT_YCbCr_411_INTEL    = 0x109,

    /**
     * Three planes, 8 bit Y plane followed by U, V plane with 1/2 width and 1/2 height.
     * The U and V planes have the same stride as the Y plane.
     * Width and height must be even.
     * Horizontal stride is aligned to 128.
     * Vertical stride is aligned to 64.
     *       ________________________ .....
     *      |Y0|Y1                   |    :
     *      |                        |    :
     *      h                        h    h'= align(h,64)
     *      |____________w___________|    :
     *      :                             :
     *      :____________ ................:
     *      |U0|U1       |                :
     *   h/2|_______w/2__|                h"= h'/2
     *      :____________ ................:
     *      |V0|V1       |                :
     *   h/2|_______w/2__|                h"= h'/2
     *      :.................stride......:
     *
     *      stride = align(w,128)
     */
    HWC_PIXEL_FORMAT_YCbCr_420_H_INTEL  = 0x10A,

    /**
     * Three planes, 8 bit Y plane followed by U, V plane with full width and 1/2 height.
     * Horizontal stride is aligned to 128.
     * Vertical stride is aligned to 64.
     *       ________________________ .....
     *      |Y0|Y1                   |    :
     *      |                        |    :
     *      h                        h    h'= align(h,64)
     *      |____________w___________|    :
     *      :                             :
     *      :________________________ ....:
     *      |U0|U1                   |    :
     *   h/2|____________w___________|    h"= h'/2
     *      :________________________ ....:
     *      |V0|V1                   |    :
     *   h/2|____________w___________|    h"= h'/2
     *      :.............................:
     *
     *      stride = align(w,128)
     */
    HWC_PIXEL_FORMAT_YCbCr_422_V_INTEL  = 0x10B,

    /**
     * Three planes, 8 bit Y plane followed by U, V plane with full width and full height.
     * Horizontal stride is aligned to 128.
     * Vertical stride is aligned to 64.
     *       ________________________ .....
     *      |Y0|Y1                   |    :
     *      |                        |    :
     *      h                        h    h'= align(h,64)
     *      |____________w___________|    :
     *      :________________________ ....:
     *      |U0|U1                   |    :
     *      |                        |    :
     *      h                        h    h'= align(h,64)
     *      |____________w___________|    :
     *      :________________________ ....:
     *      |V0|V1                   |    :
     *      |                        |    :
     *      h                        h    h'= align(h,64)
     *      |____________w___________|    :
     *      :.............................:
     *
     *      stride = align(w,128)
     */
    HWC_PIXEL_FORMAT_YCbCr_444_INTEL    = 0x10C,

    /**
     * Intel NV12 format for camera.
     *
     *
     * Additional layout information:
     * - height: must be even
     * - stride: aligned to 64
     * - vstride: same as height
     * - tiling: always linear
     *
     *       ________________________ .....
     *      |Y0|Y1                   |    :
     *      |                        |    :
     *      h                        h    h
     *      |                        |    :
     *      |                        |    :
     *      |____________w___________|....:
     *      |U|V|U|V                 |    :
     *     h/2                      h/2  h/2
     *      |____________w___________|....:
     *
     *      stride = align(w,64)
     *      vstride = h
     *
     */
    HWC_PIXEL_FORMAT_NV12_LINEAR_CAMERA_INTEL = 0x10F,

    /**
     * Intel P010 format.
     *
     * Used for 10bit usage for HEVC/VP9 decoding and video processing.
     *
     * Layout information:
     * - each pixel being represented by 16 bits (2 bytes)
     * - Y plane with even height and width
     * - hstride multiple of 64 pixels (128 bytes)
     * - hstride is specified in pixels, not in bytes
     * - vstride is aligned to 64 lines
     * - U/V are interleaved and 1/2 width and 1/2 height
     * - memory is Y tiled
     *
     *       ____________w___________ ____
     *      |Y0|Y1                   |    |
     *      |                        |    |
     *      h                        h    h'= align(h,64)
     *      |                        |    |
     *      |                        |    |
     *      |____________w___________|    |
     *      :                             |
     *      :________________________ ____|
     *      |U|V|U|V                 |    :
     *     h/2                      h/2   :
     *      |____________w___________|    h"= h'/2
     *      :.............................:
     *
     *      pitch (in bytes) = align(w*2,128)
     *      size (in bytes) = pitch*3/2
     */
    HWC_PIXEL_FORMAT_P010_INTEL = 0x110,

    /**
     * Intel A2RGB10 format.
     *
     * Used for 10bit video processing.
     *
     * \see GMM_FORMAT_B10G10R10A2_UNORM
     *
     *       ________________________ .....
     *      |BGRA|BGRA|              |    :
     *      |                        |    :
     *      h                        h    h'
     *      |____________w___________|    :
     *      :                             :
     *      :............stride...........:
     *
     *       bits
     *      +----+--------------------+--------------------+--------------------+
     *      |3130|29                20|19                10|09                 0|
     *      +----+--------------------+--------------------+--------------------+
     *      |A1A0|R9R8R7R6R5R4R3R2R1R0|G9G8G7G6G5G4G3G2G1G0|B9B8B7B6B5B4B3B2B1B0|
     *      +----+--------------------+--------------------+--------------------+
     *
     *       byte 0           byte 1           byte 2           byte 3
     *      +----------------+----------------+----------------+----------------+
     *      |B7B6B5B4B3B2B1B0|G5G4G3G2G1G0B9B8|R3R2R1R0G9G8G7G6|A1A0R9R8R7R6R5R4|
     *      +----------------+----------------+----------------+----------------+
     *
     */
    HWC_PIXEL_FORMAT_A2R10G10B10_INTEL = 0x113,

    /**
     * Intel A2RGB10 format.
     *
     * Used for 10bit video processing.
     *
     * \see GMM_FORMAT_R10G10B10A2_UNORM
     *
     *       ________________________ .....
     *      |RGBA|RGBA|              |    :
     *      |                        |    :
     *      h                        h    h'
     *      |____________w___________|    :
     *      :                             :
     *      :............stride...........:
     *
     *       bits
     *      +----+--------------------+--------------------+--------------------+
     *      |3130|29                20|19                10|09                 0|
     *      +----+--------------------+--------------------+--------------------+
     *      |A1A0|R9R8R7R6R5R4R3R2R1R0|G9G8G7G6G5G4G3G2G1G0|B9B8B7B6B5B4B3B2B1B0|
     *      +----+--------------------+--------------------+--------------------+
     *
     *       byte 0           byte 1           byte 2           byte 3
     *      +----------------+----------------+----------------+----------------+
     *      |R7R6R5R4R3R2R1R0|G5G4G3G2G1G0R9R8|B3B2B1B0B9B8B7B6|A1A0B9B8B7B6B5B4|
     *      +----------------+----------------+----------------+----------------+
     *
     */
    HWC_PIXEL_FORMAT_A2B10G10R10_INTEL = 0x114,

    /**
     * \note THIS WILL BE GOING AWAY!
     *
     * \deprecated value out of range of reserved pixel formats
     * \see #include <openmax/OMX_IVCommon.h>
     * \see OMX_INTEL_COLOR_FormatYUV420PackedSemiPlanar
     * \see HAL_PIXEL_FORMAT_NV12_LINEAR_INTEL
     */
    HWC_PIXEL_FORMAT_YUV420PackedSemiPlanar_INTEL = 0x7FA00E00,

    /**
     * \note THIS WILL BE GOING AWAY!
     *
     * \deprecated value out of range of reserved pixel formats
     * \see #include <openmax/OMX_IVCommon.h>
     * \see OMX_INTEL_COLOR_FormatYUV420PackedSemiPlanar
     * \see HWC_PIXEL_FORMAT_NV12_Y_TILED_INTEL
     */
    HWC_PIXEL_FORMAT_YUV420PackedSemiPlanar_Tiled_INTEL = 0x7FA00F00,
};

#ifdef uncomment
// Utility function - returns human-readable string from a HAL format number.
const char* getHALFormatString( int32_t halFormat );
const char* getHALFormatShortString( int32_t halFormat );
// Utility function - returns human-readable string from a DRM format number.
const char* getDRMFormatString( int32_t drmFormat );
// Utility function - returns human-readable string from a Tiling format number.
const char* getTilingFormatString( ETilingFormat halFormat );
// Utility function - returns human-readable string from a Dataspace number.
HWCString getDataSpaceString( DataSpace dataspace );
#endif


} // namespace hwcomposer

#endif // COMMON_CORE_FORMAT_H
