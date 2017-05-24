/*
// Copyright (c) 2016 Intel Corporation
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

#ifndef PUBLIC_GPUDEVICE_H_
#define PUBLIC_GPUDEVICE_H_

#include <scopedfd.h>
#include <stdint.h>

#include <memory>
#include <vector>

#include "option.h"

namespace hwcomposer {

class NativeDisplay;
class PhysicalDisplayManager;

#define DRM_HOTPLUG_EVENT_SIZE 256

class DisplayHotPlugEventCallback {
 public:
  virtual ~DisplayHotPlugEventCallback() {
  }
  virtual void Callback(std::vector<NativeDisplay*> connected_displays) = 0;
};

class GpuDevice {
 public:
  GpuDevice();

  virtual ~GpuDevice();

  // Open device.
  bool Initialize();

  NativeDisplay* GetDisplay(uint32_t display);

  NativeDisplay* GetVirtualDisplay();

  std::vector<NativeDisplay*> GetConnectedPhysicalDisplays();

  void RegisterHotPlugEventCallback(
      std::shared_ptr<DisplayHotPlugEventCallback> callback);

  // Get physical display manager.
  PhysicalDisplayManager& GetPhysicalDisplayManager( void ) { return *mPhysicalDisplayManager_; }

  // Block until SF is up and running (is fully open and is not blanked/powered-off).
  void EnsureCompositorIsReady( void ) { }

  // Force a geometry change on the next onPrepare and redraw.
  void ForceGeometryChangeAndRedraw() { }

  // Force a geometry change on the next onPrepare.
  void forceGeometryChange() { }

  void forceRedraw( ) { }

  // This will force a fresh redraw and wait for it to complete.
  // It flushes up to the issued frame on all displays before returning to caller.
  // Any trailing plug changes will also have been fully processed.
  // If timeoutNs is 0 this is blocking.
  void synchronize( nsecs_t timeoutNs = 5000000000 ) { }

  // Force a geometry change on the next onPrepare and redraw.
  void forceGeometryChangeAndRedraw() {  }

  // Notify plug change has completed, so that make sure plug event can be
  // fully serialized and synchronized.
  void notifyPlugChangeCompleted( ) { }

 private:
  class DisplayManager;
  // Order is important here as we need fd_ to be valid
  // till all cleanup is done.
  ScopedFd fd_;
  std::unique_ptr<DisplayManager> display_manager_;
  Option mOptionVppComposer;
  Option mOptionPartGlComp;
  PhysicalDisplayManager* mPhysicalDisplayManager_;
  bool initialized_;
};

}  // namespace hwcomposer
#endif  // PUBLIC_GPUDEVICE_H_
