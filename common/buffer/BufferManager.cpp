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

#include "BufferManager.h"
#include "drm_internal.h"

namespace hwcomposer {

AbstractBufferManager& AbstractBufferManager::get()
{
#ifdef uncomment
#endif
}

BufferManager::Buffer::Buffer( )
{
    mTag[0] = '\0';
}

void BufferManager::Buffer::setTag( const HWCString& tag )
{
    strncpy( mTag, tag.string(), MAX_TAG_CHARS-1 );
    mTag[ MAX_TAG_CHARS-1] ='\0';
}

HWCString BufferManager::Buffer::getTag( void )
{
    return HWCString( mTag );
}

BufferManager::BufferManager() {
    buffer_handler_.reset(NativeBufferHandler::CreateInstance(Drm::get().getDrmHandle( )));
    if (!buffer_handler_) {
      ETRACE("Failed to create native buffer handler instance");
    }
}

void BufferManager::setBufferTag( HWCNativeHandle handle, const HWCString& tag )
{
    std::shared_ptr<AbstractBufferManager::Buffer> pAbstractBuffer = acquireBuffer( handle );
    if ( pAbstractBuffer == NULL )
    {
        return;
    }
    Buffer* pBuffer = static_cast< Buffer* >( pAbstractBuffer.get() );
    pBuffer->setTag( tag );
}

HWCString BufferManager::getBufferTag( HWCNativeHandle handle )
{
    std::shared_ptr<AbstractBufferManager::Buffer> pAbstractBuffer = acquireBuffer( handle );
    if ( pAbstractBuffer == NULL )
    {
	return HWCString( "UNKNOWN" );
    }
    Buffer* pBuffer = static_cast< Buffer* >( pAbstractBuffer.get() );
    return pBuffer->getTag( );
}

std::shared_ptr<HWCNativeHandlesp> BufferManager::createGraphicBuffer( const char* pchTag,
                                                      uint32_t w, uint32_t h, int32_t format, uint32_t usage )
{
    HWCASSERT( pchTag );
    HWCASSERT( w );
    HWCASSERT( h );
    HWCASSERT( format );
    HWCASSERT(buffer_handler_);

    DTRACEIF( BUFFER_MANAGER_DEBUG,
              "createGraphicBuffer %s allocate GraphicBuffer [%ux%u fmt:%u/%s usage:0x%x]",
              pchTag, w, h, format, getHALFormatShortString(format), usage );

    std::shared_ptr<HWCNativeHandlesp> pGB(buffer_handler_->CreateGraphicsBuffer(w, h, format, usage));
    if ( ( pGB == NULL ) || ( pGB.get()->handle == NULL ) )
    {
#ifdef uncomment
	ETRACE( "createGraphicBuffer %s failed to allocate GraphicBuffer [%ux%u fmt:%u/%s usage:0x%x]",
            pchTag, w, h, format, getHALFormatShortString(format), usage );
#endif
        pGB = NULL;
    }
    else if ( sbInternalBuild )
    {
	setBufferTag( pGB.get(), HWCString::format( "%s", pchTag ) );
    }

    return pGB;
}
#ifdef uncomment
std::shared_ptr<HWCNativeHandlesp> BufferManager::createGraphicBuffer( const char* pchTag,
                                                      uint32_t w, uint32_t h, int32_t format, uint32_t usage,
                                                      uint32_t stride, native_handle_t* handle, bool keepOwnership )
{
    HWCASSERT( pchTag );
    HWCASSERT( w );
    HWCASSERT( h );
    HWCASSERT( format );
    HWCASSERT(buffer_handler_);

    DTRACEIF( BUFFER_MANAGER_DEBUG,
              "createGraphicBuffer %s allocate GraphicBuffer [%ux%u fmt:%u/%s usage:0x%x stride %u handle %p keep %d]",
              pchTag, w, h, format, getHALFormatShortString(format), usage, stride, handle, keepOwnership );

    std::shared_ptr<HWCNativeHandlesp> pGB = buffer_handler_->CreateGraphicsBuffer(w, h, format, usage);
    if ( ( pGB == NULL ) || ( pGB->handle == NULL ) )
    {
	ETRACE( "createGraphicBuffer %s failed to allocate GraphicBuffer [%ux%u fmt:%u/%s usage:0x%x stride %u handle %p keep %d]",
            pchTag, w, h, format, getHALFormatShortString(format), usage, stride, handle, keepOwnership );
        pGB = NULL;
    }
    // Don't overwrite the original GRALLOC tag.

    return pGB;
}
#endif
void BufferManager::reallocateGraphicBuffer( std::shared_ptr<HWCNativeHandlesp>& pGB,
                                             const char* pchTag,
                                             uint32_t w, uint32_t h, int32_t format, uint32_t usage )
{
    HWCASSERT( pchTag );
    HWCASSERT( w );
    HWCASSERT( h );
    HWCASSERT( format );
    if ( pGB == NULL )
    {
        return;
    }

    DTRACEIF( BUFFER_MANAGER_DEBUG,
              "reallocateGraphicBuffer %s allocate GraphicBuffer [%ux%u fmt:%u/%s usage:0x%x]",
              pchTag, w, h, format, getHALFormatShortString(format), usage );

    pGB.reset(buffer_handler_->ReAllocateGraphicsBuffer( w, h, format, usage, pGB.get() ));
    if ( ( pGB == NULL ) || ( pGB->handle == NULL ) )
    {
#ifdef uncomment
	ETRACE( "reallocateGraphicBuffer %s failed to allocate GraphicBuffer [%ux%u fmt:%u/%s usage:0x%x]",
            pchTag, w, h, format, getHALFormatShortString(format), usage );
#endif
        pGB = NULL;
    }
    if ( sbInternalBuild )
    {
	setBufferTag( pGB.get(), HWCString::format( "%s", pchTag ) );
    }
}

std::shared_ptr<HWCNativeHandlesp> BufferManager::createPurgedGraphicBuffer( const char* pchTag,
                                                            uint32_t w, uint32_t h, uint32_t format, uint32_t usage,
                                                            bool* pbIsPurged )
{
    std::shared_ptr<HWCNativeHandlesp> pBuffer = createGraphicBuffer( pchTag, w, h, format, usage );
    bool bIsPurged = false;
    if ( pBuffer != NULL )
    {
        // Purge to release memory (maps all pages to single physical page).
	bIsPurged = ( purgeBuffer( pBuffer.get() ) > 0 );
    }
    if ( pbIsPurged )
    {
        *pbIsPurged = bIsPurged;
    }
    return pBuffer;
}

void BufferManager::setSurfaceFlingerRT( HWCNativeHandle /*handle*/, uint32_t /*displayIndex*/ ) { };

void BufferManager::purgeSurfaceFlingerRenderTargets( uint32_t /*displayIndex*/ ) { };

void BufferManager::realizeSurfaceFlingerRenderTargets( uint32_t /*displayIndex*/ ) { };

uint32_t BufferManager::purgeBuffer( HWCNativeHandle /*handle*/ ) { return 0; }

uint32_t BufferManager::realizeBuffer( HWCNativeHandle /*handle*/ ) { return 0; }

bool BufferManager::getBufferDetails(HWCNativeHandle handle, HwcBuffer *bo) {
  return buffer_handler_->ImportBuffer(handle, bo);
}

}; // namespace hwcomposer
