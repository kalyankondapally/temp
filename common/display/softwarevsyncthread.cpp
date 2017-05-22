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

#include "softwarevsyncthread.h"
#include "hwctrace.h"

// Kernel sleep function - for some reason this isnt exported from bionic even
// though its implemented there. Used by standard Hwcomposer::SoftwareVsyncThread impl.
extern "C" int clock_nanosleep(clockid_t clock_id, int flags,
                           const struct timespec *request,
                           struct timespec *remain);

namespace hwcomposer {

SoftwareVsyncThread::SoftwareVsyncThread(GpuDevice& device, AbstractPhysicalDisplay* pPhysical, uint32_t refreshPeriod)
    : HWCThread(-9, "SoftwareVsyncThread"),
      mDevice(device),
      //mPhysicalDisplayManager( hwc.getPhysicalDisplayManager() ),
      meMode(eModeStopped),
      mNextFakeVSync(0),
      mRefreshPeriod(refreshPeriod),
      mpPhysical(pPhysical)
{
    HWCASSERT( mRefreshPeriod > 0 );
    HWCASSERT( pPhysical != NULL );
}

void SoftwareVsyncThread::enable(void) {
    ScopedSpinLock _l(mLock);
    DTRACEIF( VSYNC_DEBUG, "Display P%u enable SW vsync", mpPhysical->getDisplayManagerIndex() );
    if (meMode != eModeRunning)
    {
        ATRACE_INT_IF( VSYNC_DEBUG, String8::format( "HWC:P%u SW VSYNC", mpPhysical->getDisplayManagerIndex() ).string(), 1 );
        meMode = eModeRunning;

	if (!InitWorker()) {
	  ETRACE("Failed to initalize thread for KMSFenceEventHandler. %s",
		 PRINTERROR());
      }
    }
}

void SoftwareVsyncThread::disable(bool /*bWait*/) {
    ScopedSpinLock _l(mLock);
    DTRACEIF( VSYNC_DEBUG, "Display P%u disable SW vsync", mpPhysical->getDisplayManagerIndex() );
    if (meMode == eModeRunning)
    {
        ATRACE_INT_IF( VSYNC_DEBUG, String8::format( "HWC:P%u SW VSYNC", mpPhysical->getDisplayManagerIndex() ).string(), 0 );
        meMode = eModeStopping;
    }
}

void SoftwareVsyncThread::terminate(void) {
    {
	ScopedSpinLock _l(mLock);
        meMode = eModeTerminating;
    }
    HWCThread::Exit();
}

bool SoftwareVsyncThread::updatePeriod( nsecs_t refreshPeriod )
{
    HWCASSERT( refreshPeriod > 0 );
    if ( mRefreshPeriod != refreshPeriod )
    {
        mRefreshPeriod = refreshPeriod;
        return true;
    }
    return false;
}

void SoftwareVsyncThread::HandleRoutine() {
    { // scope for lock
	ScopedSpinLock _l(mLock);
        if ( meMode == eModeTerminating )
        {
	    return;
        }
    }

    const nsecs_t period = mRefreshPeriod;
    // FIXME:
    //const nsecs_t now = systemTime(CLOCK_MONOTONIC);
    const nsecs_t now = 0;
    nsecs_t next_vsync = mNextFakeVSync;
    nsecs_t sleep = next_vsync - now;
    if (sleep < 0) {
        // we missed, find where the next vsync should be
        sleep = (period - ((now - next_vsync) % period));
        next_vsync = now + sleep;
    }
    mNextFakeVSync = next_vsync + period;

    struct timespec spec;
    spec.tv_sec  = next_vsync / 1000000000;
    spec.tv_nsec = next_vsync % 1000000000;

    int err;
    do {
        err = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &spec, NULL);
    } while (err<0 && errno == EINTR);

    if (err == 0) {
        // Only send vsync in running state
        if ( meMode == eModeRunning )
        {
	   // mPhysicalDisplayManager.notifyPhysicalVSync( mpPhysical, next_vsync );
        }

        // Still call postSoftwareVSync even if in stop state
        mpPhysical->postSoftwareVSync();
    }
}

void SoftwareVsyncThread::HandleWait() {

}

}; // namespace hwcomposer
