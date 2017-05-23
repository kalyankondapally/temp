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

#ifndef COMMON_BUFFER_BUFFERMANAGER_H
#define COMMON_BUFFER_BUFFERMANAGER_H

#include "AbstractBufferManager.h"
#include "nativebufferhandler.h"

namespace hwcomposer {

// This class implements generic AbstractBufferManager interfaces.
class BufferManager : public AbstractBufferManager
{
public:

    // Extended Buffer object.
    class Buffer : public AbstractBufferManager::Buffer
    {
    public:
        enum
        {
            MAX_TAG_CHARS = 16
        };
        Buffer( );
        void setTag( const String8& tag );
        String8 getTag( void );
    protected:
        char mTag[ MAX_TAG_CHARS ];  // User specified tag for logs.
    };

    BufferManager();

    // Set buffer tag.
    virtual void setBufferTag( HWCNativeHandle handle, const String8& tag );

    // Get buffer tag.
    virtual String8 getBufferTag( HWCNativeHandle handle );

    // Wrapper to create a graphic buffer.
    // If sucessful, returns succesfully allocated buffer, else returns NULL.
    virtual std::shared_ptr<HWCNativeHandlesp> createGraphicBuffer( const char* pchTag,
                                                   uint32_t w, uint32_t h, int32_t format, uint32_t usage );
#ifdef uncomment
    // Wrapper to create a graphic buffer.
    // If sucessful, returns succesfully allocated buffer, else returns NULL.
    virtual std::shared_ptr<HWCNativeHandlesp> createGraphicBuffer( const char* pchTag,
                                                   uint32_t w, uint32_t h, int32_t format, uint32_t usage,
						   uint32_t stride, native_handle_t* handle, bool keepOwnership );
#endif
    // Wrapper to reallocate a graphic buffer.
    // If sucessful, on return pGB will be the succesfully reallocated buffer, else pGB will be NULL.
    virtual void reallocateGraphicBuffer( std::shared_ptr<HWCNativeHandlesp>& pGB,
                                          const char* pchTag,
                                          uint32_t w, uint32_t h, int32_t format, uint32_t usage );

    // Wrapper to create a graphic buffer with minimal backing store (e.g. for "empty" buffers).
    // If sucessful, returns succesfully allocated buffer, else returns NULL.
    virtual std::shared_ptr<HWCNativeHandlesp> createPurgedGraphicBuffer( const char* pchTag,
                                                         uint32_t w, uint32_t h, uint32_t format, uint32_t usage,
                                                         bool* pbIsPurged = NULL );

    // Specify this buffer as a SurfaceFlinger RenderTarget for a display.
    // handle must be non-NULL.
    // OPTIONAL.
    virtual void setSurfaceFlingerRT( HWCNativeHandle handle, uint32_t displayIndex );

    // This will be called before leaving onPrepare to inform the buffer manager
    // that SurfaceFlinger compositions will not be used on a display.
    // OPTIONAL.
    virtual void purgeSurfaceFlingerRenderTargets( uint32_t displayIndex );

    // This will be called before leaving onPrepare to inform the buffer manager
    // that SurfaceFlinger compositions will be used on a display.
    // If the buffermanager implementation purges unused SF buffers then it MUST
    // implement this to ensure SF buffers are ready in time.
    // OPTIONAL.
    virtual void realizeSurfaceFlingerRenderTargets( uint32_t displayIndex );

    // Purge the backing for this buffer.
    // Returns the full buffer size in bytes if succesful.
    // Returns zero if the call fails or is not implemented.
    // OPTIONAL.
    virtual uint32_t purgeBuffer( HWCNativeHandle handle );

    // Realize the backing for this buffer.
    // Returns the full buffer size in bytes if succesful.
    // Returns zero if the call fails or is not implemented.
    // OPTIONAL.
    virtual uint32_t realizeBuffer( HWCNativeHandle handle );

    bool getBufferDetails(HWCNativeHandle handle, HwcBuffer *bo) override;
 private:
   std::unique_ptr<NativeBufferHandler> buffer_handler_;
};

}; // namespace hwcomposer

#endif // COMMON_BUFFER_BUFFERMANAGER_H
