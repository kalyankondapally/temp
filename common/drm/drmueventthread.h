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

#ifndef COMMON_DRM_DRMUEVENTTHREAD_H
#define COMMON_DRM_DRMUEVENTTHREAD_H

#include "drm.h"
#include "hwcthread.h"

namespace hwcomposer {

class Drm;
class GpuDevice;

//*****************************************************************************
//
// DrmUEventThread class - responsible for handling HDMI uevents
//
//*****************************************************************************
class DrmUEventThread : public HWCThread
{
public:
    DrmUEventThread(GpuDevice& device, Drm& drm);
    virtual ~DrmUEventThread();

    bool Initialize();

private:
    //Thread functions
    void HandleRoutine() override;

    // Decode the most recent message and forward it to DRM for the appropriate displays.
    // Returns -1 if not decoded.
    int onUEvent( void );

    // Decodes the most recent message into an event.
    Drm::UEvent decodeUEvent( void );

private:
    GpuDevice&            mDevice;
    // TODO: change to HotPlugListener
    //Drm pointer
    Drm&            mDrm;
    uint32_t        mESDConnectorType;
    uint32_t        mESDConnectorID;

    // Maximum supported message size.
    static const int MSG_LEN = 256;
    // UEvent file handle (opened in readyToRun).
    int             mUeventFd;
    // Most recent read message.
    char            mUeventMsg[MSG_LEN];
    // Most recent read message size.
    size_t          mUeventMsgSize;
};

}; // namespace hwcomposer

#endif // COMMON_DRM_DRMUEVENTTHREAD_H
