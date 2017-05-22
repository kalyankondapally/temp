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

#ifndef COMMON_DISPLAY_SOFTWAREVSYNCTHREAD_H
#define COMMON_DISPLAY_SOFTWAREVSYNCTHREAD_H

#include "hwcthread.h"
#include "physicaldisplay.h"
#include "spinlock.h"

// Kernel sleep function - for some reason this isnt exported from bionic even
// though its implemented there. Used by standard Hwcomposer::SoftwareVsyncThread impl.
extern "C" int clock_nanosleep(clockid_t clock_id, int flags,
                           const struct timespec *request,
                           struct timespec *remain);

namespace hwcomposer {

//*****************************************************************************
//
// SoftwareVsyncThread class - responsible for generating vsyncs.
//
//*****************************************************************************

class SoftwareVsyncThread : public HWCThread {
public:
    // Construct a software vsync thread.
    SoftwareVsyncThread(GpuDevice& device, AbstractPhysicalDisplay* pPhysical, uint32_t refreshPeriod);
    // Enable generation of vsyncs.
    void enable(void);
    // Disable generation of vsyncs.
    void disable(bool bWait);
    // Terminate the software vsync thread.
    void terminate(void);
    // Change the period between vsyncs.
    bool updatePeriod( nsecs_t refreshPeriod );

private:
    enum EMode { eModeStopped = 0, eModeRunning, eModeStopping, eModeTerminating };

    void HandleRoutine() override;
    void HandleWait() override;

private:
    GpuDevice&                  mDevice;
    // FIXME:
     //PhysicalDisplayManager&     mPhysicalDisplayManager;
    SpinLock                    mLock;
    EMode                       meMode;
    mutable nsecs_t             mNextFakeVSync;
    nsecs_t                     mRefreshPeriod;
    AbstractPhysicalDisplay*    mpPhysical;
};

}; // namespace hwcomposer

#endif // COMMON_DISPLAY_SOFTWAREVSYNCTHREAD_H
