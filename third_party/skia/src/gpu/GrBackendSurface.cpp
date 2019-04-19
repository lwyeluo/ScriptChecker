/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendSurface.h"

#include "gl/GrGLUtil.h"

#ifdef SK_VULKAN
#include "vk/GrVkTypes.h"
#include "vk/GrVkUtil.h"
#endif

GrBackendFormat::GrBackendFormat(GrGLenum format, GrGLenum target)
        : fBackend(kOpenGL_GrBackend)
        , fValid(true) {
    fGL.fTarget = target;
    fGL.fFormat = format;
}

const GrGLenum* GrBackendFormat::getGLFormat() const {
    if (this->isValid() && kOpenGL_GrBackend == fBackend) {
        return &fGL.fFormat;
    }
    return nullptr;
}

const GrGLenum* GrBackendFormat::getGLTarget() const {
    if (this->isValid() && kOpenGL_GrBackend == fBackend) {
        return &fGL.fTarget;
    }
    return nullptr;
}

#ifdef SK_VULKAN
GrBackendFormat::GrBackendFormat(VkFormat vkFormat)
        : fBackend(kVulkan_GrBackend)
        , fValid(true)
        , fVkFormat(vkFormat) {
}

const VkFormat* GrBackendFormat::getVkFormat() const {
    if (this->isValid() && kVulkan_GrBackend == fBackend) {
        return &fVkFormat;
    }
    return nullptr;
}
#endif

GrBackendFormat::GrBackendFormat(GrPixelConfig config)
        : fBackend(kMock_GrBackend)
        , fValid(true)
        , fMockFormat(config) {
}

const GrPixelConfig* GrBackendFormat::getMockFormat() const {
    if (this->isValid() && kMock_GrBackend == fBackend) {
        return &fMockFormat;
    }
    return nullptr;
}

#ifdef SK_VULKAN
GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   const GrVkImageInfo& vkInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fConfig(GrVkFormatToPixelConfig(vkInfo.fFormat))
        , fMipMapped(GrMipMapped(vkInfo.fLevelCount > 1))
        , fBackend(kVulkan_GrBackend)
        , fVkInfo(vkInfo) {}
#endif

#if GR_TEST_UTILS

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrPixelConfig config,
                                   const GrGLTextureInfo& glInfo)
        : GrBackendTexture(width, height, config, GrMipMapped::kNo, glInfo) {}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrPixelConfig config,
                                   GrMipMapped mipMapped,
                                   const GrGLTextureInfo& glInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fConfig(config)
        , fMipMapped(mipMapped)
        , fBackend(kOpenGL_GrBackend)
        , fGLInfo(glInfo) {}
#endif

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipMapped mipMapped,
                                   const GrGLTextureInfo& glInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fConfig(GrGLSizedFormatToPixelConfig(glInfo.fFormat))
        , fMipMapped(mipMapped)
        , fBackend(kOpenGL_GrBackend)
        , fGLInfo(glInfo) {}

GrBackendTexture::GrBackendTexture(int width,
                                   int height,
                                   GrMipMapped mipMapped,
                                   const GrMockTextureInfo& mockInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fConfig(mockInfo.fConfig)
        , fMipMapped(mipMapped)
        , fBackend(kMock_GrBackend)
        , fMockInfo(mockInfo) {}

#ifdef SK_VULKAN
const GrVkImageInfo* GrBackendTexture::getVkImageInfo() const {
    if (this->isValid() && kVulkan_GrBackend == fBackend) {
        return &fVkInfo;
    }
    return nullptr;
}
#endif

const GrGLTextureInfo* GrBackendTexture::getGLTextureInfo() const {
    if (this->isValid() && kOpenGL_GrBackend == fBackend) {
        return &fGLInfo;
    }
    return nullptr;
}

const GrMockTextureInfo* GrBackendTexture::getMockTextureInfo() const {
    if (this->isValid() && kMock_GrBackend == fBackend) {
        return &fMockInfo;
    }
    return nullptr;
}

GrBackendFormat GrBackendTexture::format() const {
    switch (this->backend()) {
#ifdef SK_VULKAN
        case kVulkan_GrBackend: {
            const GrVkImageInfo* vkInfo = this->getVkImageInfo();
            SkASSERT(vkInfo);
            return GrBackendFormat::MakeVk(vkInfo->fFormat);
        }
#endif
        case kOpenGL_GrBackend: {
            const GrGLTextureInfo* glInfo = this->getGLTextureInfo();
            SkASSERT(glInfo);
            return GrBackendFormat::MakeGL(glInfo->fFormat, glInfo->fTarget);
        }
        case kMock_GrBackend: {
            const GrMockTextureInfo* mockInfo = this->getMockTextureInfo();
            SkASSERT(mockInfo);
            return GrBackendFormat::MakeMock(mockInfo->fConfig);
        }
        default:
            return GrBackendFormat();
    }
}

#if GR_TEST_UTILS
bool GrBackendTexture::TestingOnly_Equals(const GrBackendTexture& t0, const GrBackendTexture& t1) {
    if (!t0.isValid() || !t1.isValid()) {
        return false; // two invalid backend textures are not considered equal
    }

    if (t0.fWidth != t1.fWidth ||
        t0.fHeight != t1.fHeight ||
        t0.fConfig != t1.fConfig ||
        t0.fMipMapped != t1.fMipMapped ||
        t0.fBackend != t1.fBackend) {
        return false;
    }

    switch (t0.fBackend) {
    case kOpenGL_GrBackend:
        return t0.fGLInfo == t1.fGLInfo;
    case kMock_GrBackend:
        return t0.fMockInfo == t1.fMockInfo;
    case kVulkan_GrBackend:
#ifdef SK_VULKAN
        return t0.fVkInfo == t1.fVkInfo;
#else
        // fall through
#endif
    case kMetal_GrBackend: // fall through
    default:
        return false;
    }

    SkASSERT(0);
    return false;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_VULKAN
GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             const GrVkImageInfo& vkInfo)
        : GrBackendRenderTarget(width, height, sampleCnt, vkInfo) {
    // This is a deprecated constructor that takes a bogus stencil bits.
    SkASSERT(0 == stencilBits);
}

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             const GrVkImageInfo& vkInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fSampleCnt(SkTMax(1, sampleCnt))
        , fStencilBits(0)  // We always create stencil buffers internally for vulkan
        , fConfig(GrVkFormatToPixelConfig(vkInfo.fFormat))
        , fBackend(kVulkan_GrBackend)
        , fVkInfo(vkInfo) {}
#endif

#if GR_TEST_UTILS

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             GrPixelConfig config,
                                             const GrGLFramebufferInfo& glInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fSampleCnt(SkTMax(1, sampleCnt))
        , fStencilBits(stencilBits)
        , fConfig(config)
        , fBackend(kOpenGL_GrBackend)
        , fGLInfo(glInfo) {}
#endif

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             const GrGLFramebufferInfo& glInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fSampleCnt(SkTMax(1, sampleCnt))
        , fStencilBits(stencilBits)
        , fConfig(GrGLSizedFormatToPixelConfig(glInfo.fFormat))
        , fBackend(kOpenGL_GrBackend)
        , fGLInfo(glInfo) {}

GrBackendRenderTarget::GrBackendRenderTarget(int width,
                                             int height,
                                             int sampleCnt,
                                             int stencilBits,
                                             const GrMockRenderTargetInfo& mockInfo)
        : fIsValid(true)
        , fWidth(width)
        , fHeight(height)
        , fSampleCnt(SkTMax(1, sampleCnt))
        , fStencilBits(stencilBits)
        , fConfig(mockInfo.fConfig)
        , fMockInfo(mockInfo) {}

#ifdef SK_VULKAN
const GrVkImageInfo* GrBackendRenderTarget::getVkImageInfo() const {
    if (kVulkan_GrBackend == fBackend) {
        return &fVkInfo;
    }
    return nullptr;
}
#endif

const GrGLFramebufferInfo* GrBackendRenderTarget::getGLFramebufferInfo() const {
    if (kOpenGL_GrBackend == fBackend) {
        return &fGLInfo;
    }
    return nullptr;
}

const GrMockRenderTargetInfo* GrBackendRenderTarget::getMockRenderTargetInfo() const {
    if (kMock_GrBackend == fBackend) {
        return &fMockInfo;
    }
    return nullptr;
}

#if GR_TEST_UTILS
bool GrBackendRenderTarget::TestingOnly_Equals(const GrBackendRenderTarget& r0,
                                               const GrBackendRenderTarget& r1) {
    if (!r0.isValid() || !r1.isValid()) {
        return false; // two invalid backend rendertargets are not considered equal
    }

    if (r0.fWidth != r1.fWidth ||
        r0.fHeight != r1.fHeight ||
        r0.fSampleCnt != r1.fSampleCnt ||
        r0.fStencilBits != r1.fStencilBits ||
        r0.fConfig != r1.fConfig ||
        r0.fBackend != r1.fBackend) {
        return false;
    }

    switch (r0.fBackend) {
    case kOpenGL_GrBackend:
        return r0.fGLInfo == r1.fGLInfo;
    case kMock_GrBackend:
        return r0.fMockInfo == r1.fMockInfo;
    case kVulkan_GrBackend:
#ifdef SK_VULKAN
        return r0.fVkInfo == r1.fVkInfo;
#else
        // fall through
#endif
    case kMetal_GrBackend: // fall through
    default:
        return false;
    }

    SkASSERT(0);
    return false;
}
#endif
