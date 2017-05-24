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

#ifndef COMMON_UTILS_HWCTRACE_H_
#define COMMON_UTILS_HWCTRACE_H_

#include <vector>
#include <string>
#include <chrono>

#include <errno.h>
#include <stdio.h>
#include <time.h>

#include "displayplane.h"
#include "platformdefines.h"

#ifdef _cplusplus
extern "C" {
#endif

// #define ENABLE_DISPLAY_DUMP 1
// #define ENABLE_DISPLAY_MANAGER_TRACING 1
// #define ENABLE_PAGE_FLIP_EVENT_TRACING 1
// #define ENABLE_HOT_PLUG_EVENT_TRACING 1
// #define FUNCTION_CALL_TRACING 1
#define COMPOSITOR_TRACING 1
// FIXME: Make this a build option.
//#define HWC_DEVELOPER_BUILD 1

// Function call tracing
#ifdef FUNCTION_CALL_TRACING
class TraceFunc {
 public:
  explicit TraceFunc(std::string func_name) {
    func_name_ = func_name;
    ITRACE("Calling ----- %s", func_name_.c_str());
    t_ = std::chrono::high_resolution_clock::now();
  }
  ~TraceFunc() {
    std::chrono::high_resolution_clock::time_point t2 =
        std::chrono::high_resolution_clock::now();
    ITRACE(
        "Total time spent in --- %s Time(msec): %lld", func_name_.c_str(),
        std::chrono::duration_cast<std::chrono::microseconds>(t2 - t_).count());
    ITRACE("Leaving --- %s", func_name_.c_str());
  }

 private:
  std::chrono::high_resolution_clock::time_point t_;
  std::string func_name_;
};
#define CTRACE() TraceFunc hwctrace(__func__);
#else
#define CTRACE()  STRACE()
#endif

// Arguments tracing
#if 0
#define ATRACE(fmt, ...) VTRACE("%s(args): " fmt, __func__, ##__VA_ARGS__);
#else
#define ATRACE(fmt, ...) ((void)0)
#endif

// Useful Debug tracing
#ifdef ENABLE_DISPLAY_DUMP
#define DUMPTRACE ITRACE
#else
#define DUMPTRACE(fmt, ...) ((void)0)
#endif

// Page Flip event tracing
#ifdef ENABLE_PAGE_FLIP_EVENT_TRACING
#define IPAGEFLIPEVENTTRACE ITRACE
#else
#define IPAGEFLIPEVENTTRACE(fmt, ...) ((void)0)
#endif

#ifdef ENABLE_DISPLAY_MANAGER_TRACING
#define IDISPLAYMANAGERTRACE ITRACE
#else
#define IDISPLAYMANAGERTRACE(fmt, ...) ((void)0)
#endif

#ifdef ENABLE_HOT_PLUG_EVENT_TRACING
#define IHOTPLUGEVENTTRACE ITRACE
#else
#define IHOTPLUGEVENTTRACE(fmt, ...) ((void)0)
#endif

#ifdef COMPOSITOR_TRACING
#define ICOMPOSITORTRACE ITRACE
#else
#define ICOMPOSITORTRACE(fmt, ...) ((void)0)
#endif

// Errors
#define PRINTERROR() strerror(-errno)

// Helper to abort the execution if object is not initialized.
// This should never happen if the rules below are followed during design:
// 1) Create an object.
// 2) Initialize the object immediately.
// 3) If failed, delete the object.
// These helpers should be disabled and stripped out of release build

#define RETURN_X_IF_NOT_INIT(X)                                              \
  do {                                                                       \
    CTRACE();                                                                \
    if (false == mInitialized) {                                             \
      LOG_ALWAYS_FATAL("%s: Object is not initialized! Line = %d", __func__, \
                       __LINE__);                                            \
      return X;                                                              \
    }                                                                        \
  } while (0)

#if 1
#define RETURN_FALSE_IF_NOT_INIT() RETURN_X_IF_NOT_INIT(false)
#define RETURN_VOID_IF_NOT_INIT() RETURN_X_IF_NOT_INIT()
#define RETURN_NULL_IF_NOT_INIT() RETURN_X_IF_NOT_INIT(0)
#else
#define RETURN_FALSE_IF_NOT_INIT() ((void)0)
#define RETURN_VOID_IF_NOT_INIT() ((void)0)
#define RETURN_NULL_IF_NOT_INIT() ((void)0)
#endif

// Helper to log error message, call de-initializer and return false.
#define DEINIT_AND_RETURN_FALSE(...) \
  do {                               \
    ETRACE(__VA_ARGS__);             \
    deinitialize();                  \
    return false;                    \
  } while (0)

#define DEINIT_AND_DELETE_OBJ(X) \
  if (X) {                       \
    X->deinitialize();           \
    delete X;                    \
    X = NULL;                    \
  }

#define WARN_IF_NOT_DEINIT()                                                 \
  CTRACE();                                                                  \
  if (mInitialized) {                                                        \
    LOG_ALWAYS_FATAL("%s: Object is not deinitialized! Line = %d", __func__, \
                     __LINE__);                                              \
  }

#ifdef ENABLE_DISPLAY_DUMP
#define DUMP_CURRENT_COMPOSITION_PLANES()                                  \
  frame_++;                                                                \
  DUMPTRACE(                                                               \
      "Dumping DisplayPlaneState of Current Composition planes "           \
      "-----------------------------");                                    \
  DUMPTRACE("Frame: %d", frame_);                                          \
  DUMPTRACE("Total Layers for this Frame: %d", layers.size()); \
  DUMPTRACE("Total Planes in use for this Frame: %d",                      \
            current_composition_planes.size());                            \
  int plane_index = 1;                                                     \
  for (DisplayPlaneState & comp_plane : current_composition_planes) {      \
    DUMPTRACE("Composition Plane State for Index: %d", plane_index);       \
    const std::vector<size_t> &source_layers = comp_plane.source_layers(); \
    switch (comp_plane.GetCompositionState()) {                            \
      case DisplayPlaneState::State::kRender:                              \
        DUMPTRACE("DisplayPlane state: kRender. Total layers: %lu",        \
                  source_layers.size());                                   \
        DUMPTRACE("Layers Index:");                                        \
        for (size_t primary_index : source_layers) {                       \
          DUMPTRACE("index: %d", primary_index);                           \
          layers.at(primary_index).Dump();                     \
        }                                                                  \
        break;                                                             \
      case DisplayPlaneState::State::kScanout:                             \
        if (source_layers.size() > 1)                                      \
          DUMPTRACE(                                                       \
              "Plane has more than one layer associated when its type is " \
              "kScanout. This needs to be fixed.");                        \
        DUMPTRACE("DisplayPlane State: kScanout. Total layers: %lu",       \
                  source_layers.size());                                   \
        DUMPTRACE("Layers Index:");                                        \
        for (size_t overlay_index : source_layers) {                       \
          DUMPTRACE("index: %d", overlay_index);                           \
          layers.at(overlay_index).Dump();                     \
        }                                                                  \
        break;                                                             \
      default:                                                             \
        break;                                                             \
    }                                                                      \
    comp_plane.plane()->Dump();                                            \
    DUMPTRACE("Composition Plane State ends for Index: %d", plane_index);  \
    plane_index++;                                                         \
  }                                                                        \
  DUMPTRACE(                                                               \
      "Dumping DisplayPlaneState of Current Composition planes ends. "     \
      "-----------------------------");
#else
#define DUMP_CURRENT_COMPOSITION_PLANES() ((void)0)
#endif


#ifdef HWC_DEVELOPER_BUILD
#define HWC_ASSERT_LOCK_NOT_HELD(mLock) HWCASSERT(!mLock.islocked());
#define HWC_ASSERT_LOCK_NOT_HELD2( mTimingLock ) HWCASSERT(!mTimingLock.islocked());
#define HWC_ASSERT_LOCK_NOT_HELD3( mDisplayTimingsLock ) HWCASSERT(!mDisplayTimingsLock.islocked());
#define HWC_ASSERT_LOCK_NOT_HELD4( mSetVSyncLock ) HWCASSERT(!mSetVSyncLock.islocked());
#else
#define HWC_ASSERT_LOCK_NOT_HELD(mLock) ((void)0)
#define HWC_ASSERT_LOCK_NOT_HELD2( mTimingLock ) ((void)0);
#define HWC_ASSERT_LOCK_NOT_HELD3( mDisplayTimingsLock ) ((void)0);
#define HWC_ASSERT_LOCK_NOT_HELD4( mSetVSyncLock ) ((void)0);
#endif

#define BUFFER_MANAGER_DEBUG            0 // Debug from buffer manager.
#define BUFFERQUEUE_DEBUG               0 // Debug from the BufferQueue class
#define COMPOSITION_DEBUG               0 // Debug related to the Hardware classes
#define COMPOSER_DEBUG                  0 // Debug from the GPU composer subsystems
#define CONTENT_DEBUG                   0 // Debug related to the Content classes
#define DISPLAY_QUEUE_DEBUG             0 // Debug from display queue.
#define DRM_DEBUG                       0 // Debug from DRM (general).
#define DRM_DISPLAY_DEBUG               0 // Debug from DRM display.
#define DRM_STATE_DEBUG                 0 // Debug from DRM state update.
#define DRM_SUSPEND_DEBUG               0 // Debug from DRM suspend/resume.
#define DRM_BLANKING_DEBUG              0 // Debug from DRM blanking.
#define DRM_PAGEFLIP_DEBUG              0 // Debug from DRM pageflip handler.
#define ESD_DEBUG                       0 // Debug from DRM ESD processing.
#define FILTER_DEBUG                    0 // Debugging from filters
#define GLOBAL_SCALING_DEBUG            0 // Debug from global scaling processing (includes panel fitter for DRM displays).
#define HWC_DEBUG                       0 // Dump HWC entrypoints
#define HWC_SYNC_DEBUG                  0 // Debug from HWC synchronization methods.
#define HWCLOG_DEBUG                    0 // Debug HWC logger
#define HPLUG_DEBUG                     0 // Debug from DRM hotplug processing.
#define LOGDISP_DEBUG                   0 // Debug related to the LogicalDisplay classes
#define LOWLOSS_COMPOSER_DEBUG          0 // Debug Lowloss composer
#define MDS_DEBUG                       0 // Debug from MDS (multidisplay server) and related.
#define MODE_DEBUG                      0 // Dump debug about mode enumeration/update.
#define MUTEX_CONDITION_DEBUG           0 // Debug mutex/conditions.
#define PAVP_DEBUG                      0 // Debug from PAVP.
#define PHYDISP_DEBUG                   0 // Debug related to the PhysicalDisplay classes
#define PARTITION_DEBUG                 0 // Partitioning info from the PartitionedComposer
#define PERSISTENT_REGISTRY_DEBUG       0 // Debug persistent registry.
#define PLANEALLOC_OPT_DEBUG            0 // Debug from plane allocator optimizer (detailed).
#define PLANEALLOC_CAPS_DEBUG           0 // Debug from plane allocator plane caps pre-check.
#define PLANEALLOC_SUMMARY_DEBUG        0 // Summary debug from plane allocator module.
#define PRIMARYDISPLAYPROXY_DEBUG       0 // Debug for Proxy Display
#define SYNC_FENCE_DEBUG                0 // Debug relating to sync fences
#define VIRTUALDISPLAY_DEBUG            0 // Debug from the Virtual Display subsystem
#define VISIBLERECTFILTER_DEBUG         0 // Debug VisibleRect Filter
#define VSYNC_DEBUG                     0 // Debug about vsync.
#define VSYNC_RATE_DEBUG                0 // Debug about vsync issue rate.
#define WIDI_DEBUG                      0 // Dump debug from WIDI display

// Mode related debug combo.
#define DRMDISPLAY_MODE_DEBUG ( DRM_DEBUG || MODE_DEBUG || HPLUG_DEBUG || DRM_SUSPEND_DEBUG || DRM_BLANKING_DEBUG )

// Dump HWC input state on prepare
#define PREPARE_INFO_DEBUG      (0 || DRM_DEBUG)

// Dump HWC input state on set
#define SET_INFO_DEBUG          (0 || DRM_DEBUG)

// Trace enabling tags
#define DISPLAY_TRACE                   sbInternalBuild
#define DRM_CALL_TRACE                  sbInternalBuild
#define HWC_TRACE                       sbInternalBuild
#define RENDER_TRACE                    sbInternalBuild
#define BUFFER_WAIT_TRACE               sbInternalBuild
#define TRACKER_TRACE                   sbInternalBuild

// _cplusplus
#ifdef _cplusplus
}
#endif

#endif  // COMMON_UTILS_HWCTRACE_H_
