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

#include "transform.h"
#include "utils.h"
#include <math.h>

namespace hwcomposer {

void clipLayerToDisplay( Layer* pLayer, const uint32_t dispW, const uint32_t dispH )
{
    HWCASSERT( pLayer );
    HwcRect<float>& src = pLayer->editSrc();
    HwcRect<int>& dst = pLayer->editDst();

    // clip dst to display
    clipToDisplay( &src.left, &src.top,
                   &src.right, &src.bottom,
                   pLayer->getTransform(),
                   &dst.left, &dst.top,
                   &dst.right, &dst.bottom,
                   dispW, dispH );

    // clip visibleRegions to display
    std::vector<HwcRect<int>>& visRegions = pLayer->editVisibleRegions();
    for ( uint32_t r = 0; r < visRegions.size(); r++ )
    {
	HwcRect<int>& visRect = visRegions.at(r);
        float sx1 = 0.0;
        float sy1 = 0.0;
        float sx2 = (float)( visRect.right - visRect.left );
        float sy2 = (float)( visRect.bottom - visRect.top );

        clipToDisplay( &sx1, &sy1,
                       &sx2, &sy2,
                       pLayer->getTransform(),
                       &visRect.left, &visRect.top,
                       &visRect.right, &visRect.bottom,
                       dispW, dispH );
    }
}

bool clipToDestRect( float* psx1, float* psy1,
                    float* psx2, float* psy2,
                    const ETransform transform,
                    int32_t* pdx1, int32_t* pdy1,
                    int32_t* pdx2, int32_t* pdy2,
		    const HwcRect<int>& destRect )
{
    HWCASSERT( psx1 );
    HWCASSERT( psy1 );
    HWCASSERT( psx2 );
    HWCASSERT( psy2 );
    HWCASSERT( pdx1 );
    HWCASSERT( pdy1 );
    HWCASSERT( pdx2 );
    HWCASSERT( pdy2 );

    const int32_t visibleX1 = destRect.left;
    const int32_t visibleY1 = destRect.top;
    const int32_t visibleX2 = destRect.right;
    const int32_t visibleY2 = destRect.bottom;

    const uint32_t dispW = visibleX2 - visibleX1;
    const uint32_t dispH = visibleY2 - visibleY1;

    // If sourceCrop or displayFrame or display has NULL size then bail.
    const float scw = *psx2 - *psx1;
    const float sch = *psy2 - *psy1;
    const float dfw = *pdx2 - *pdx1;
    const float dfh = *pdy2 - *pdy1;
    if ( ( dfw == 0 )
      || ( dfh == 0 )
      || ( scw == 0.0f )
      || ( sch == 0.0f )
      || ( dispW == 0 )
      || ( dispH == 0 ) )
    {
        return false;
    }


    // If the displayFrame is entirely off the display then bail.
    if ( ( *pdx2 < visibleX1    )
      || ( *pdy2 < visibleY1    )
      || ( *pdx1 >= (int32_t)visibleX2 )
      || ( *pdy1 >= (int32_t)visibleY2 ) )
    {
        return false;
    }

    // If the displayFrame is entirely in the display then bail.
    if ( ( *pdx1 >= visibleX1  )
      && ( *pdx2 <= (int32_t)visibleX2 )
      && ( *pdy1 >= visibleY1  )
      && ( *pdy2 <= (int32_t)visibleY2 ) )
    {
        return true;
    }

    float scaleX;
    float scaleY;
    if ( isTranspose( transform ) )
    {
        scaleX = sch/(float)dfw;
        scaleY = scw/(float)dfh;
    }
    else
    {
        scaleX = scw/(float)dfw;
        scaleY = sch/(float)dfh;
    }

    // Clip at left display edge.
    if ( *pdx1 < visibleX1 )
    {
        const float crop = visibleX1 -*pdx1;
        switch ( transform )
        {
            case ETransform::NONE:
                *psx1 += scaleX*crop;                     // 0 None
                break;
            case ETransform::FLIP_H:                      // 1 Mirror H
                *psx2 -= scaleX*crop;
                break;
            case ETransform::FLIP_V:                      // 2 Mirror V
                *psx1 += scaleX*crop;
                break;
            case ETransform::ROT_180:                     // 3 Mirror H+V
                *psx2 -= scaleX*crop;
                break;
            case ETransform::ROT_90:                      // 4 Rot90
                *psy2 -= scaleX*crop;
                break;
            case ETransform::FLIP_H_90:                   // 5 Mirror y=1-x
                *psy1 += scaleX*crop;
                break;
            case ETransform::FLIP_V_90:                   // 6 Mirror around y=x
                *psy2 -= scaleX*crop;
                break;
            case ETransform::ROT_270:                     // 7 Rot90 + Mirror H+V
                *psy1 += scaleX*crop;
                break;
        }
        *pdx1 = visibleX1;
    }

    // Clip at right display edge.
    if ( *pdx2 > (int32_t)visibleX2 )
    {
        const float crop = ( *pdx2 - (int32_t)visibleX2 );
        switch ( transform )
        {
            default:
                *psx2 -= scaleX*crop;                       // 0 None
                break;
            case ETransform::FLIP_H:                      // 1 Mirror H
                *psx1 += scaleX*crop;
                break;
            case ETransform::FLIP_V:                      // 2 Mirror V
                *psx2 -= scaleX*crop;
                break;
            case ETransform::ROT_180:                     // 3 Mirror H+V
                *psx1 += scaleX*crop;
                break;
            case ETransform::ROT_90:                      // 4 Rot90
                *psy1 += scaleX*crop;
                break;
            case ETransform::FLIP_H_90:                   // 5 Mirror y=1-x
                *psy2 -= scaleX*crop;
                break;
            case ETransform::FLIP_V_90:                   // 6 Mirror around y=x
                *psy1 += scaleX*crop;
                break;
            case ETransform::ROT_270:                     // 7 Rot90 + Mirror H+V
                *psy2 -= scaleX*crop;
                break;
        }
        *pdx2 = visibleX2;
    }

    // Clip at top display edge.
    if ( *pdy1 < visibleY1 )
    {
        const float crop = visibleY1 -*pdy1;
        switch ( transform )
        {
            default:
                *psy1 += scaleY*crop;                       // 0 None
                break;
            case ETransform::FLIP_H:                      // 1 Mirror H
                *psy1 += scaleY*crop;
                break;
            case ETransform::FLIP_V:                      // 2 Mirror V
                *psy2 -= scaleY*crop;
                break;
            case ETransform::ROT_180:                     // 3 Mirror H+V
                *psy2 -= scaleY*crop;
                break;
            case ETransform::ROT_90:                      // 4 Rot90
                *psx1 += scaleY*crop;
                break;
            case ETransform::FLIP_H_90:                   // 5 Mirror y=1-x
                *psx1 += scaleY*crop;
                break;
            case ETransform::FLIP_V_90:                   // 6 Mirror around y=x
                *psx2 -= scaleY*crop;
                break;
            case ETransform::ROT_270:                     // 7 Rot90 + Mirror H+V
                *psx2 -= scaleY*crop;
                break;
        }
        *pdy1 = visibleY1;
    }

    // Clip at bottom display edge.
    if ( *pdy2 > (int32_t)visibleY2 )
    {
        const float crop = ( *pdy2 - (int32_t)visibleY2 );
        switch ( transform )
        {
            default:
                *psy2 -= scaleY*crop;                       // 0 None
                break;
            case ETransform::FLIP_H:                      // 1 Mirror H
                *psy2 -= scaleY*crop;
                break;
            case ETransform::FLIP_V:                      // 2 Mirror V
                *psy1 += scaleY*crop;
                break;
            case ETransform::ROT_180:                     // 3 Mirror H+V
                *psy1 += scaleY*crop;
                break;
            case ETransform::ROT_90:                      // 4 Rot90
                *psx2 -= scaleY*crop;
                break;
            case ETransform::FLIP_H_90:                   // 5 Mirror y=1-x
                *psx2 -= scaleY*crop;
                break;
            case ETransform::FLIP_V_90:                   // 6 Mirror around y=x
                *psx2 += scaleY*crop;
                break;
            case ETransform::ROT_270:                     // 7 Rot90 + Mirror H+V
                *psx1 += scaleY*crop;
                break;
        }
        *pdy2 = visibleY2;
    }
    return true;
}

bool clipToDisplay( float* psx1, float* psy1,
                    float* psx2, float* psy2,
                    const ETransform transform,
                    int32_t* pdx1, int32_t* pdy1,
                    int32_t* pdx2, int32_t* pdy2,
                    const uint32_t dispW, const uint32_t dispH )
{
    HwcRect<int> destRect;
    destRect.left   = 0;
    destRect.top    = 0;
    destRect.right  = dispW;
    destRect.bottom = dispH;

    return clipToDestRect(psx1, psy1,
                   psx2, psy2,
                   transform,
                   pdx1, pdy1,
                   pdx2, pdy2,
                   destRect );
}

bool clipLayerToDestRect( Layer* pLayer, const HwcRect<int>& destRect )
{
    HWCASSERT( pLayer );
    HwcRect<float>& src = pLayer->editSrc();
    HwcRect<int>&  dst = pLayer->editDst();

    return clipToDestRect( &src.left, &src.top,
                   &src.right, &src.bottom,
                   pLayer->getTransform(),
                   &dst.left, &dst.top,
                   &dst.right, &dst.bottom,
                   destRect );
}

};  // namespace hwcomposer.
