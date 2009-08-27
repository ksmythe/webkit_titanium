/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"

#if ENABLE(3D_CANVAS)

#include "GraphicsContext3D.h"

#include "CachedImage.h"
#include "CanvasBuffer.h"
#include "CanvasFramebuffer.h"
#include "CanvasNumberArray.h"
#include "CanvasObject.h"
#include "CanvasProgram.h"
#include "CanvasRenderbuffer.h"
#include "CanvasShader.h"
#include "CanvasTexture.h"
#include "CString.h"
#include "HTMLCanvasElement.h"
#include "HTMLImageElement.h"
#include "ImageBuffer.h"
#include "NotImplemented.h"
#include "WebKitCSSMatrix.h"

#include <CoreGraphics/CGBitmapContext.h>

namespace WebCore {

GraphicsContext3D::GraphicsContext3D()
{
    CGLPixelFormatAttribute attribs[] =
    {
        (CGLPixelFormatAttribute) kCGLPFAAccelerated,
        (CGLPixelFormatAttribute) kCGLPFAColorSize, (CGLPixelFormatAttribute) 32,
        (CGLPixelFormatAttribute) kCGLPFADepthSize, (CGLPixelFormatAttribute) 32,
        (CGLPixelFormatAttribute) kCGLPFASupersample,
        (CGLPixelFormatAttribute) 0
    };
    
    CGLPixelFormatObj pixelFormatObj;
    GLint numPixelFormats;
    
    CGLChoosePixelFormat(attribs, &pixelFormatObj, &numPixelFormats);
    
    CGLCreateContext(pixelFormatObj, 0, &m_contextObj);
    
    CGLDestroyPixelFormat(pixelFormatObj);
    
    // Set the current context to the one given to us.
    CGLSetCurrentContext(m_contextObj);
    
    // create a texture to render into
    ::glGenTextures(1, &m_texture);
    ::glBindTexture(GL_TEXTURE_2D, m_texture);
    ::glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    ::glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    ::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    ::glBindTexture(GL_TEXTURE_2D, 0);
    
    // create an FBO
    ::glGenFramebuffersEXT(1, &m_fbo);
    ::glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
    
    ::glGenRenderbuffersEXT(1, &m_depthBuffer);
    ::glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthBuffer);
    ::glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, 1, 1);
    ::glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
    
    ::glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_texture, 0);
    ::glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_depthBuffer);
    
    ::glClearColor(0, 0, 0, 0);
}

 GraphicsContext3D::~GraphicsContext3D()
{
    CGLSetCurrentContext(m_contextObj);
    ::glDeleteRenderbuffersEXT(1, & m_depthBuffer);
    ::glDeleteTextures(1, &m_texture);
    ::glDeleteFramebuffersEXT(1, &m_fbo);
    CGLSetCurrentContext(0);
    CGLDestroyContext(m_contextObj);
}

PlatformGraphicsContext3D GraphicsContext3D::platformGraphicsContext3D() const
{
    return m_contextObj;
}

void GraphicsContext3D::checkError() const
{
    // FIXME: This needs to only be done in the debug context. It will probably throw an exception
    // on error and print the error message to the debug console
    GLenum error = ::glGetError();
    if (error != GL_NO_ERROR)
        notImplemented();
}

void GraphicsContext3D::makeContextCurrent()
{
    CGLSetCurrentContext(m_contextObj);
}

void GraphicsContext3D::reshape(int width, int height)
{
    if (width == m_currentWidth && height == m_currentHeight)
        return;
    
    m_currentWidth = width;
    m_currentHeight = height;
    
    CGLSetCurrentContext(m_contextObj);
    
    ::glBindTexture(GL_TEXTURE_2D, m_texture);
    ::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    ::glBindTexture(GL_TEXTURE_2D, 0);
    
    ::glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
    ::glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthBuffer);
    ::glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
    ::glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
    
    ::glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_texture, 0);
    ::glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_depthBuffer);
    GLenum status = ::glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
        // FIXME: cleanup
        notImplemented();
    }

    ::glClear(GL_COLOR_BUFFER_BIT);
}

static inline void ensureContext(CGLContextObj context)
{
    CGLContextObj currentContext = CGLGetCurrentContext();
    if (currentContext != context)
        CGLSetCurrentContext(context);
}

void GraphicsContext3D::glActiveTexture(unsigned long texture)
{
    ensureContext(m_contextObj);
    ::glActiveTexture(texture);
}

void GraphicsContext3D::glAttachShader(CanvasProgram* program, CanvasShader* shader)
{
    if (!program || !shader)
        return;
    ensureContext(m_contextObj);
    ::glAttachShader((GLuint) program->object(), (GLuint) shader->object());
}

void GraphicsContext3D::glBindAttribLocation(CanvasProgram* program, unsigned long index, const String& name)
{
    if (!program)
        return;
    ensureContext(m_contextObj);
    ::glBindAttribLocation((GLuint) program->object(), index, name.utf8().data());
}

void GraphicsContext3D::glBindBuffer(unsigned long target, CanvasBuffer* buffer)
{
    ensureContext(m_contextObj);
    ::glBindBuffer(target, buffer ? (GLuint) buffer->object() : 0);
}


void GraphicsContext3D::glBindFramebuffer(unsigned long target, CanvasFramebuffer* buffer)
{
    ensureContext(m_contextObj);
    ::glBindFramebufferEXT(target, buffer ? (GLuint) buffer->object() : m_fbo);
}

void GraphicsContext3D::glBindRenderbuffer(unsigned long target, CanvasRenderbuffer* renderbuffer)
{
    ensureContext(m_contextObj);
    ::glBindBuffer(target, renderbuffer ? (GLuint) renderbuffer->object() : 0);
}


void GraphicsContext3D::glBindTexture(unsigned target, CanvasTexture* texture)
{
    ensureContext(m_contextObj);
    ::glBindTexture(target, texture ? (GLuint) texture->object() : 0);
}

void GraphicsContext3D::glBlendColor(double red, double green, double blue, double alpha)
{
    ensureContext(m_contextObj);
    ::glBlendColor(static_cast<float>(red), static_cast<float>(green), static_cast<float>(blue), static_cast<float>(alpha));
}

void GraphicsContext3D::glBlendEquation( unsigned long mode )
{
    ensureContext(m_contextObj);
    ::glBlendEquation(mode);
}

void GraphicsContext3D::glBlendEquationSeparate(unsigned long modeRGB, unsigned long modeAlpha)
{
    ensureContext(m_contextObj);
    ::glBlendEquationSeparate(modeRGB, modeAlpha);
}


void GraphicsContext3D::glBlendFunc(unsigned long sfactor, unsigned long dfactor)
{
    ensureContext(m_contextObj);
    ::glBlendFunc(sfactor, dfactor);
}       

void GraphicsContext3D::glBlendFuncSeparate(unsigned long srcRGB, unsigned long dstRGB, unsigned long srcAlpha, unsigned long dstAlpha)
{
    ensureContext(m_contextObj);
    ::glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
}

void GraphicsContext3D::glBufferData(unsigned long target, CanvasNumberArray* array, unsigned long usage)
{
    if (!array || !array->data().size())
        return;
    
    ensureContext(m_contextObj);
    
    // FIXME: This is a complete hack. If the target is GL_ELEMENT_ARRAY_BUFFER then the
    // passed data needs to be bytes or shorts because those are the only types supported
    // by GL ES 2.0. So for now we just convert to a short buffer
    if (target == GL_ELEMENT_ARRAY_BUFFER) {
        uint16_t* buf = (uint16_t*) malloc(array->data().size() * sizeof(uint16_t));
        for (size_t i = 0; i < array->data().size(); ++i)
            buf[i] = (uint16_t) array->data()[i];
        
        ::glBufferData(target, array->data().size() * sizeof(uint16_t), buf, usage);
        free(buf);
    }
    else
        ::glBufferData(target, array->data().size() * sizeof(float), &(array->data()[0]), usage);
    
}

void GraphicsContext3D::glBufferSubData(unsigned long target, long offset, CanvasNumberArray* array)
{
    if (!array || !array->data().size())
        return;
    
    ensureContext(m_contextObj);
    ::glBufferSubData(target, offset, array->data().size() * sizeof(float), &(array->data()[0]));
}

unsigned long GraphicsContext3D::glCheckFramebufferStatus(CanvasFramebuffer* framebuffer)
{
    if (!framebuffer)
        return -1;
    
    ensureContext(m_contextObj);
    return ::glCheckFramebufferStatusEXT((GLuint) framebuffer->object());
}

void GraphicsContext3D::glClearColor(double r, double g, double b, double a)
{
    ensureContext(m_contextObj);
    ::glClearColor(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b), static_cast<float>(a));
}

void GraphicsContext3D::glClear(unsigned long mask)
{
    ensureContext(m_contextObj);
    ::glClear(mask);
}

void GraphicsContext3D::glClearDepth(double depth)
{
    ensureContext(m_contextObj);
    ::glClearDepth(depth);
}

void GraphicsContext3D::glClearStencil(long s)
{
    ensureContext(m_contextObj);
    ::glClearStencil(s);
}

void GraphicsContext3D::glColorMask(bool red, bool green, bool blue, bool alpha)
{
    ensureContext(m_contextObj);
    ::glColorMask(red, green, blue, alpha);
}

void GraphicsContext3D::glCompileShader(CanvasShader* shader)
{
    if (!shader)
        return;
    
    ensureContext(m_contextObj);
    ::glCompileShader((GLuint) shader->object());
}

void GraphicsContext3D::glCopyTexImage2D(unsigned long target, long level, unsigned long internalformat, long x, long y, unsigned long width, unsigned long height, long border)
{
    ensureContext(m_contextObj);
    ::glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}

void GraphicsContext3D::glCopyTexSubImage2D(unsigned long target, long level, long xoffset, long yoffset, long x, long y, unsigned long width, unsigned long height)
{
    ensureContext(m_contextObj);
    ::glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

void GraphicsContext3D::glCullFace(unsigned long mode)
{
    ensureContext(m_contextObj);
    ::glCullFace(mode);
}

void GraphicsContext3D::glDepthFunc(unsigned long func)
{
    ensureContext(m_contextObj);
    ::glDepthFunc(func);
}

void GraphicsContext3D::glDepthMask(bool flag)
{
    ensureContext(m_contextObj);
    ::glDepthMask(flag);
}

void GraphicsContext3D::glDepthRange(double zNear, double zFar)
{
    ensureContext(m_contextObj);
    ::glDepthRange(zNear, zFar);
}

void GraphicsContext3D::glDetachShader(CanvasProgram* program, CanvasShader* shader)
{
    if (!program || !shader)
        return;
    
    ensureContext(m_contextObj);
    ::glDetachShader((GLuint) program->object(), (GLuint) shader->object());
}

void GraphicsContext3D::glDisable(unsigned long cap)
{
    ensureContext(m_contextObj);
    ::glDisable(cap);
}

void GraphicsContext3D::glDisableVertexAttribArray(unsigned long index)
{
    ensureContext(m_contextObj);
    ::glDisableVertexAttribArray(index);
}

void GraphicsContext3D::glDrawArrays(unsigned long mode, long first, unsigned long count)
{
    ensureContext(m_contextObj);
    ::glDrawArrays(mode, first, count);
}

void GraphicsContext3D::glDrawElements(unsigned long mode, unsigned long count, unsigned long type, void* array)
{
    ensureContext(m_contextObj);
    ::glDrawElements(mode, count, type, array);
}

void GraphicsContext3D::glEnable(unsigned long cap)
{
    ensureContext(m_contextObj);
    ::glEnable(cap);
}

void GraphicsContext3D::glEnableVertexAttribArray(unsigned long index)
{
    ensureContext(m_contextObj);
    ::glEnableVertexAttribArray(index);
}

void GraphicsContext3D::glFinish()
{
    ensureContext(m_contextObj);
    ::glFinish();
}

void GraphicsContext3D::glFlush()
{
    ensureContext(m_contextObj);
    ::glFlush();
}

void GraphicsContext3D::glFramebufferRenderbuffer(unsigned long target, unsigned long attachment, unsigned long renderbuffertarget, CanvasRenderbuffer* buffer)
{
    if (!buffer)
        return;
    
    ensureContext(m_contextObj);
    ::glFramebufferRenderbufferEXT(target, attachment, renderbuffertarget, (GLuint) buffer->object());
}

void GraphicsContext3D::glFramebufferTexture2D(unsigned long target, unsigned long attachment, unsigned long textarget, CanvasTexture* texture, long level)
{
    if (!texture)
        return;
    
    ensureContext(m_contextObj);
    ::glFramebufferTexture2DEXT(target, attachment, textarget, (GLuint) texture->object(), level);
}

void GraphicsContext3D::glFrontFace(unsigned long mode)
{
    ensureContext(m_contextObj);
    ::glFrontFace(mode);
}

void GraphicsContext3D::glGenerateMipmap(unsigned long target)
{
    ensureContext(m_contextObj);
    ::glGenerateMipmapEXT(target);
}

int GraphicsContext3D::glGetAttribLocation(CanvasProgram* program, const String& name)
{
    if (!program)
        return -1;
    
    ensureContext(m_contextObj);
    return ::glGetAttribLocation((GLuint) program->object(), name.utf8().data());
}

unsigned long GraphicsContext3D::glGetError()
{
    ensureContext(m_contextObj);
    return ::glGetError();
}

String GraphicsContext3D::glGetString(unsigned long name)
{
    ensureContext(m_contextObj);
    return String((const char*) ::glGetString(name));
}

void GraphicsContext3D::glHint(unsigned long target, unsigned long mode)
{
    ensureContext(m_contextObj);
    ::glHint(target, mode);
}

bool GraphicsContext3D::glIsBuffer(CanvasBuffer* buffer)
{
    if (!buffer)
        return false;
    
    ensureContext(m_contextObj);
    return ::glIsBuffer((GLuint) buffer->object());
}

bool GraphicsContext3D::glIsEnabled(unsigned long cap)
{
    ensureContext(m_contextObj);
    return ::glIsEnabled(cap);
}

bool GraphicsContext3D::glIsFramebuffer(CanvasFramebuffer* framebuffer)
{
    if (!framebuffer)
        return false;
    
    ensureContext(m_contextObj);
    return ::glIsFramebufferEXT((GLuint) framebuffer->object());
}

bool GraphicsContext3D::glIsProgram(CanvasProgram* program)
{
    if (!program)
        return false;
    
    ensureContext(m_contextObj);
    return ::glIsProgram((GLuint) program->object());
}

bool GraphicsContext3D::glIsRenderbuffer(CanvasRenderbuffer* renderbuffer)
{
    if (!renderbuffer)
        return false;
    
    ensureContext(m_contextObj);
    return ::glIsRenderbufferEXT((GLuint) renderbuffer->object());
}

bool GraphicsContext3D::glIsShader(CanvasShader* shader)
{
    if (!shader)
        return false;
    
    ensureContext(m_contextObj);
    return ::glIsShader((GLuint) shader->object());
}

bool GraphicsContext3D::glIsTexture(CanvasTexture* texture)
{
    if (!texture)
        return false;
    
    ensureContext(m_contextObj);
    return ::glIsTexture((GLuint) texture->object());
}

void GraphicsContext3D::glLineWidth(double width)
{
    ensureContext(m_contextObj);
    ::glLineWidth(static_cast<float>(width));
}

void GraphicsContext3D::glLinkProgram(CanvasProgram* program)
{
    if (!program)
        return;
    
    ensureContext(m_contextObj);
    ::glLinkProgram((GLuint) program->object());
}

void GraphicsContext3D::glPixelStorei(unsigned long pname, long param)
{
    ensureContext(m_contextObj);
    ::glPixelStorei(pname, param);
}

void GraphicsContext3D::glPolygonOffset(double factor, double units)
{
    ensureContext(m_contextObj);
    ::glPolygonOffset(static_cast<float>(factor), static_cast<float>(units));
}

void GraphicsContext3D::glReleaseShaderCompiler()
{
    // FIXME: This is not implemented on desktop OpenGL. We need to have ifdefs for the different GL variants
    ensureContext(m_contextObj);
    //::glReleaseShaderCompiler();
}

void GraphicsContext3D::glRenderbufferStorage(unsigned long target, unsigned long internalformat, unsigned long width, unsigned long height)
{
    ensureContext(m_contextObj);
    ::glRenderbufferStorageEXT(target, internalformat, width, height);
}

void GraphicsContext3D::glSampleCoverage(double value, bool invert)
{
    ensureContext(m_contextObj);
    ::glSampleCoverage(static_cast<float>(value), invert);
}

void GraphicsContext3D::glScissor(long x, long y, unsigned long width, unsigned long height)
{
    ensureContext(m_contextObj);
    ::glScissor(x, y, width, height);
}

void GraphicsContext3D::glShaderSource(CanvasShader* shader, const String& string)
{
    if (!shader)
        return;
    
    ensureContext(m_contextObj);
    const CString& cs = string.utf8();
    const char* s = cs.data();
    
    int length = string.length();
    ::glShaderSource((GLuint) shader->object(), 1, &s, &length);
}

void GraphicsContext3D::glStencilFunc(unsigned long func, long ref, unsigned long mask)
{
    ensureContext(m_contextObj);
    ::glStencilFunc(func, ref, mask);
}

void GraphicsContext3D::glStencilFuncSeparate(unsigned long face, unsigned long func, long ref, unsigned long mask)
{
    ensureContext(m_contextObj);
    ::glStencilFuncSeparate(face, func, ref, mask);
}

void GraphicsContext3D::glStencilMask(unsigned long mask)
{
    ensureContext(m_contextObj);
    ::glStencilMask(mask);
}

void GraphicsContext3D::glStencilMaskSeparate(unsigned long face, unsigned long mask)
{
    ensureContext(m_contextObj);
    ::glStencilMaskSeparate(face, mask);
}

void GraphicsContext3D::glStencilOp(unsigned long fail, unsigned long zfail, unsigned long zpass)
{
    ensureContext(m_contextObj);
    ::glStencilOp(fail, zfail, zpass);
}

void GraphicsContext3D::glStencilOpSeparate(unsigned long face, unsigned long fail, unsigned long zfail, unsigned long zpass)
{
    ensureContext(m_contextObj);
    ::glStencilOpSeparate(face, fail, zfail, zpass);
}


void GraphicsContext3D::glTexParameter(unsigned target, unsigned pname, CanvasNumberArray* array)
{
    if (!array || !array->data().size())
        return;
    
    ensureContext(m_contextObj);
    ::glTexParameterfv(target, pname, &(array->data()[0]));
}

void GraphicsContext3D::glTexParameter(unsigned target, unsigned pname, double value)
{
    ensureContext(m_contextObj);
    ::glTexParameterf(target, pname, static_cast<float>(value));
}

void GraphicsContext3D::glUniform(long location, CanvasNumberArray* array)
{
    if (!array || !array->data().size())
        return;
    
    ensureContext(m_contextObj);
    ::glUniform1fv(location, array->data().size(), &(array->data()[0]));
}

void GraphicsContext3D::glUniform(long location, float v0)
{
    ensureContext(m_contextObj);
    ::glUniform1f(location, v0);
}

void GraphicsContext3D::glUniform(long location, float v0, float v1)
{
    ensureContext(m_contextObj);
    ::glUniform2f(location, v0, v1);
}

void GraphicsContext3D::glUniform(long location, float v0, float v1, float v2)
{
    ensureContext(m_contextObj);
    ::glUniform3f(location, v0, v1, v2);
}

void GraphicsContext3D::glUniform(long location, float v0, float v1, float v2, float v3)
{
    ensureContext(m_contextObj);
    ::glUniform4f(location, v0, v1, v2, v3);
}

void GraphicsContext3D::glUniform(long location, int v0)
{
    ensureContext(m_contextObj);
    ::glUniform1i(location, v0);
}

void GraphicsContext3D::glUniform(long location, int v0, int v1)
{
    ensureContext(m_contextObj);
    ::glUniform2i(location, v0, v1);
}

void GraphicsContext3D::glUniform(long location, int v0, int v1, int v2)
{
    ensureContext(m_contextObj);
    ::glUniform3i(location, v0, v1, v2);
}

void GraphicsContext3D::glUniform(long location, int v0, int v1, int v2, int v3)
{
    ensureContext(m_contextObj);
    ::glUniform4i(location, v0, v1, v2, v3);
}

void GraphicsContext3D::glUniformMatrix(long location, long count, bool transpose, CanvasNumberArray* array)
{
    // FIXME: This should never have to deal with bad params. They should be checked higher and throw exceptions
    if (!array || !array->data().size())
        return;
    
    int n = array->data().size() / count;
    if (n < 2)
        return;
    
    if (n > 4)
        n = 4;
    
    ensureContext(m_contextObj);
    
    switch(n) {
        case 2: ::glUniformMatrix2fv(location, count, transpose, &(array->data()[0])); break;
        case 3: ::glUniformMatrix3fv(location, count, transpose, &(array->data()[0])); break;
        case 4: ::glUniformMatrix4fv(location, count, transpose, &(array->data()[0])); break;
    }
}

void GraphicsContext3D::glUniformMatrix(long location, bool transpose, const Vector<WebKitCSSMatrix*>& array)
{
    int count = array.size();
    float* floats = (float*) malloc(count * 16 * sizeof(float));
    
    for (int i = 0; i < count; ++i) {
        floats[i*16+ 0] = static_cast<float>(array[i]->m11());
        floats[i*16+ 1] = static_cast<float>(array[i]->m12());
        floats[i*16+ 2] = static_cast<float>(array[i]->m13());
        floats[i*16+ 3] = static_cast<float>(array[i]->m14());
        floats[i*16+ 4] = static_cast<float>(array[i]->m21());
        floats[i*16+ 5] = static_cast<float>(array[i]->m22());
        floats[i*16+ 6] = static_cast<float>(array[i]->m23());
        floats[i*16+ 7] = static_cast<float>(array[i]->m24());
        floats[i*16+ 8] = static_cast<float>(array[i]->m31());
        floats[i*16+ 9] = static_cast<float>(array[i]->m32());
        floats[i*16+10] = static_cast<float>(array[i]->m33());
        floats[i*16+11] = static_cast<float>(array[i]->m34());
        floats[i*16+12] = static_cast<float>(array[i]->m41());
        floats[i*16+13] = static_cast<float>(array[i]->m42());
        floats[i*16+14] = static_cast<float>(array[i]->m43());
        floats[i*16+15] = static_cast<float>(array[i]->m44());
    }
    
    ::glUniformMatrix4fv(location, count, transpose, floats);
    free(floats);
}

void GraphicsContext3D::glUniformMatrix(long location, bool transpose, const WebKitCSSMatrix* matrix)
{
    float* floats = (float*) malloc(16 * sizeof(float));
    
    floats[ 0] = static_cast<float>(matrix->m11());
    floats[ 1] = static_cast<float>(matrix->m12());
    floats[ 2] = static_cast<float>(matrix->m13());
    floats[ 3] = static_cast<float>(matrix->m14());
    floats[ 4] = static_cast<float>(matrix->m21());
    floats[ 5] = static_cast<float>(matrix->m22());
    floats[ 6] = static_cast<float>(matrix->m23());
    floats[ 7] = static_cast<float>(matrix->m24());
    floats[ 8] = static_cast<float>(matrix->m31());
    floats[ 9] = static_cast<float>(matrix->m32());
    floats[10] = static_cast<float>(matrix->m33());
    floats[11] = static_cast<float>(matrix->m34());
    floats[12] = static_cast<float>(matrix->m41());
    floats[13] = static_cast<float>(matrix->m42());
    floats[14] = static_cast<float>(matrix->m43());
    floats[15] = static_cast<float>(matrix->m44());
    
    ::glUniformMatrix4fv(location, 1, transpose, floats);
    free(floats);
}

void GraphicsContext3D::glUseProgram(CanvasProgram* program)
{
    if (!program)
        return;
    
    ensureContext(m_contextObj);
    ::glUseProgram((GLuint) program->object());
}

void GraphicsContext3D::glValidateProgram(CanvasProgram* program)
{
    if (!program)
        return;
    
    ensureContext(m_contextObj);
    ::glValidateProgram((GLuint) program->object());
}

void GraphicsContext3D::glVertexAttrib(unsigned long indx, float v0)
{
    ensureContext(m_contextObj);
    ::glVertexAttrib1f(indx, v0);
}

void GraphicsContext3D::glVertexAttrib(unsigned long indx, float v0, float v1)
{
    ensureContext(m_contextObj);
    ::glVertexAttrib2f(indx, v0, v1);
}

void GraphicsContext3D::glVertexAttrib(unsigned long indx, float v0, float v1, float v2)
{
    ensureContext(m_contextObj);
    ::glVertexAttrib3f(indx, v0, v1, v2);
}

void GraphicsContext3D::glVertexAttrib(unsigned long indx, float v0, float v1, float v2, float v3)
{
    ensureContext(m_contextObj);
    ::glVertexAttrib4f(indx, v0, v1, v2, v3);
}

void GraphicsContext3D::glVertexAttrib(unsigned long indx, CanvasNumberArray* array)
{
    if (!array || !array->data().size())
        return;
    
    // FIXME: How do we decide between 1fv .. 4fv?
    ensureContext(m_contextObj);
    ::glVertexAttrib1fv(indx, &(array->data()[0]));
}


void GraphicsContext3D::glVertexAttribPointer(unsigned long indx, long size, unsigned long type, bool normalized, unsigned long stride, CanvasNumberArray* array)
{
    ensureContext(m_contextObj);
    
    if (array) {
        if (m_vertexArray.size() <= indx)
            m_vertexArray.resize(indx+1);
        
        m_vertexArray[indx] = array->data();
    }
    else if (m_vertexArray.size() > indx)
        m_vertexArray[indx].clear();
    
    ::glVertexAttribPointer(indx, size, type, normalized, stride, array ? &(m_vertexArray[indx][0]) : 0);
}

void GraphicsContext3D::glViewport(long x, long y, unsigned long width, unsigned long height)
{
    ensureContext(m_contextObj);
    ::glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}

static int sizeForGetParam(unsigned long pname)
{
    switch(pname) {
        case GL_ACTIVE_TEXTURE:                  return 1;
        case GL_ALIASED_LINE_WIDTH_RANGE:        return 2;
        case GL_ALIASED_POINT_SIZE_RANGE:        return 2;
        case GL_ALPHA_BITS:                      return 1;
        case GL_ARRAY_BUFFER_BINDING:            return 1; // (* actually a CanvasBuffer*)
        case GL_BLEND:                           return 1;
        case GL_BLEND_COLOR:                     return 4;
        case GL_BLEND_DST_ALPHA:                 return 1;
        case GL_BLEND_DST_RGB:                   return 1;
        case GL_BLEND_EQUATION_ALPHA:            return 1;
        case GL_BLEND_EQUATION_RGB:              return 1;
        case GL_BLEND_SRC_ALPHA:                 return 1;
        case GL_BLEND_SRC_RGB:                   return 1;
        case GL_BLUE_BITS:                       return 1;
        case GL_COLOR_CLEAR_VALUE:               return 4;
        case GL_COLOR_WRITEMASK:                 return 4;
        case GL_COMPRESSED_TEXTURE_FORMATS:      return GL_NUM_COMPRESSED_TEXTURE_FORMATS;
        case GL_CULL_FACE:                       return 1;
        case GL_CULL_FACE_MODE:                  return 1;
        case GL_CURRENT_PROGRAM:                 return 1; // (* actually a CanvasProgram*)
        case GL_DEPTH_BITS:                      return 1;
        case GL_DEPTH_CLEAR_VALUE:               return 1;
        case GL_DEPTH_FUNC:                      return 1;
        case GL_DEPTH_RANGE:                     return 2;
        case GL_DEPTH_TEST:                      return 1;
        case GL_DEPTH_WRITEMASK:                 return 1;
        case GL_DITHER:                          return 1;
        case GL_ELEMENT_ARRAY_BUFFER_BINDING:    return 1; // (* actually a CanvasBuffer*)
        case GL_FRAMEBUFFER_BINDING_EXT:         return 1; // (* actually a CanvasFramebuffer*)
        case GL_FRONT_FACE:                      return 1;
        case GL_GENERATE_MIPMAP_HINT:            return 1;
        case GL_GREEN_BITS:                      return 1;
            //case GL_IMPLEMENTATION_COLOR_READ_FORMAT:return 1;
            //case GL_IMPLEMENTATION_COLOR_READ_TYPE:  return 1;
        case GL_LINE_WIDTH:                      return 1;
        case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:return 1;
        case GL_MAX_CUBE_MAP_TEXTURE_SIZE:       return 1;
            //case GL_MAX_FRAGMENT_UNIFORM_VECTORS:    return 1;
        case GL_MAX_RENDERBUFFER_SIZE_EXT:       return 1;
        case GL_MAX_TEXTURE_IMAGE_UNITS:         return 1;
        case GL_MAX_TEXTURE_SIZE:                return 1;
            //case GL_MAX_VARYING_VECTORS:             return 1;
        case GL_MAX_VERTEX_ATTRIBS:              return 1;
        case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:  return 1;
            //case GL_MAX_VERTEX_UNIFORM_VECTORS:      return 1;
        case GL_MAX_VIEWPORT_DIMS:               return 2;
        case GL_NUM_COMPRESSED_TEXTURE_FORMATS:  return 1;
            //case GL_NUM_SHADER_BINARY_FORMATS:       return 1;
        case GL_PACK_ALIGNMENT:                  return 1;
        case GL_POLYGON_OFFSET_FACTOR:           return 1;
        case GL_POLYGON_OFFSET_FILL:             return 1;
        case GL_POLYGON_OFFSET_UNITS:            return 1;
        case GL_RED_BITS:                        return 1;
        case GL_RENDERBUFFER_BINDING_EXT:        return 1; // (* actually a CanvasRenderbuffer*)
        case GL_SAMPLE_BUFFERS:                  return 1;
        case GL_SAMPLE_COVERAGE_INVERT:          return 1;
        case GL_SAMPLE_COVERAGE_VALUE:           return 1;
        case GL_SAMPLES:                         return 1;
        case GL_SCISSOR_BOX:                     return 4;
        case GL_SCISSOR_TEST:                    return 1;
            //case GL_SHADER_BINARY_FORMATS:           return GL_NUM_SHADER_BINARY_FORMATS;
            //case GL_SHADER_COMPILER:                 return 1;
        case GL_STENCIL_BACK_FAIL:               return 1;
        case GL_STENCIL_BACK_FUNC:               return 1;
        case GL_STENCIL_BACK_PASS_DEPTH_FAIL:    return 1;
        case GL_STENCIL_BACK_PASS_DEPTH_PASS:    return 1;
        case GL_STENCIL_BACK_REF:                return 1;
        case GL_STENCIL_BACK_VALUE_MASK:         return 1;
        case GL_STENCIL_BACK_WRITEMASK:          return 1;
        case GL_STENCIL_BITS:                    return 1;
        case GL_STENCIL_CLEAR_VALUE:             return 1;
        case GL_STENCIL_FAIL:                    return 1;
        case GL_STENCIL_FUNC:                    return 1;
        case GL_STENCIL_PASS_DEPTH_FAIL:         return 1;
        case GL_STENCIL_PASS_DEPTH_PASS:         return 1;
        case GL_STENCIL_REF:                     return 1;
        case GL_STENCIL_TEST:                    return 1;
        case GL_STENCIL_VALUE_MASK:              return 1;
        case GL_STENCIL_WRITEMASK:               return 1;
        case GL_SUBPIXEL_BITS:                   return 1;
        case GL_TEXTURE_BINDING_2D:              return 1; // (* actually a CanvasTexture*)
        case GL_TEXTURE_BINDING_CUBE_MAP:        return 1; // (* actually a CanvasTexture*)
        case GL_UNPACK_ALIGNMENT:                return 1;
        case GL_VIEWPORT:                        return 4;
    }
    
    return -1;
}

static int typeForGetParam(unsigned long pname)
{
    switch(pname) {
        case GL_ACTIVE_TEXTURE:                  return GL_INT;
        case GL_ALIASED_LINE_WIDTH_RANGE:        return GL_FLOAT;
        case GL_ALIASED_POINT_SIZE_RANGE:        return GL_FLOAT;
        case GL_ALPHA_BITS:                      return GL_INT;
        case GL_ARRAY_BUFFER_BINDING:            return GL_INT; // (* actually a CanvasBuffer*)
        case GL_BLEND:                           return GL_BYTE;
        case GL_BLEND_COLOR:                     return GL_FLOAT;
        case GL_BLEND_DST_ALPHA:                 return GL_INT;
        case GL_BLEND_DST_RGB:                   return GL_INT;
        case GL_BLEND_EQUATION_ALPHA:            return GL_INT;
        case GL_BLEND_EQUATION_RGB:              return GL_INT;
        case GL_BLEND_SRC_ALPHA:                 return GL_INT;
        case GL_BLEND_SRC_RGB:                   return GL_INT;
        case GL_BLUE_BITS:                       return GL_INT;
        case GL_COLOR_CLEAR_VALUE:               return GL_FLOAT;
        case GL_COLOR_WRITEMASK:                 return GL_BYTE;
        case GL_COMPRESSED_TEXTURE_FORMATS:     return GL_INT;
        case GL_CULL_FACE:                       return GL_BYTE;
        case GL_CULL_FACE_MODE:                  return GL_INT;
        case GL_CURRENT_PROGRAM:                 return GL_INT; // (* actually a CanvasProgram*)
        case GL_DEPTH_BITS:                      return GL_INT;
        case GL_DEPTH_CLEAR_VALUE:               return GL_FLOAT;
        case GL_DEPTH_FUNC:                      return GL_INT;
        case GL_DEPTH_RANGE:                     return GL_FLOAT;
        case GL_DEPTH_TEST:                      return GL_BYTE;
        case GL_DEPTH_WRITEMASK:                 return GL_BYTE;
        case GL_DITHER:                          return GL_BYTE;
        case GL_ELEMENT_ARRAY_BUFFER_BINDING:    return GL_INT; // (* actually a CanvasBuffer*)
        case GL_FRAMEBUFFER_BINDING_EXT:         return GL_INT; // (* actually a CanvasFramebuffer*)
        case GL_FRONT_FACE:                      return GL_INT;
        case GL_GENERATE_MIPMAP_HINT:            return GL_INT;
        case GL_GREEN_BITS:                      return GL_INT;
            //case GL_IMPLEMENTATION_COLOR_READ_FORMAT:return GL_INT;
            //case GL_IMPLEMENTATION_COLOR_READ_TYPE:  return GL_INT;
        case GL_LINE_WIDTH:                      return GL_FLOAT;
        case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:return GL_INT;
        case GL_MAX_CUBE_MAP_TEXTURE_SIZE:       return GL_INT;
            //case GL_MAX_FRAGMENT_UNIFORM_VECTORS:    return GL_INT;
        case GL_MAX_RENDERBUFFER_SIZE_EXT:       return GL_INT;
        case GL_MAX_TEXTURE_IMAGE_UNITS:         return GL_INT;
        case GL_MAX_TEXTURE_SIZE:                return GL_INT;
            //case GL_MAX_VARYING_VECTORS:             return GL_INT;
        case GL_MAX_VERTEX_ATTRIBS:              return GL_INT;
        case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:  return GL_INT;
            //case GL_MAX_VERTEX_UNIFORM_VECTORS:      return GL_INT;
        case GL_MAX_VIEWPORT_DIMS:               return GL_INT;
        case GL_NUM_COMPRESSED_TEXTURE_FORMATS:  return GL_INT;
            //case GL_NUM_SHADER_BINARY_FORMATS:       return GL_INT;
        case GL_PACK_ALIGNMENT:                  return GL_INT;
        case GL_POLYGON_OFFSET_FACTOR:           return GL_FLOAT;
        case GL_POLYGON_OFFSET_FILL:             return GL_BYTE;
        case GL_POLYGON_OFFSET_UNITS:            return GL_FLOAT;
        case GL_RED_BITS:                        return GL_INT;
        case GL_RENDERBUFFER_BINDING_EXT:        return GL_INT; // (* actually a CanvasRenderbuffer*)
        case GL_SAMPLE_BUFFERS:                  return GL_INT;
        case GL_SAMPLE_COVERAGE_INVERT:          return GL_BYTE;
        case GL_SAMPLE_COVERAGE_VALUE:           return GL_FLOAT;
        case GL_SAMPLES:                         return GL_INT;
        case GL_SCISSOR_BOX:                     return GL_INT;
        case GL_SCISSOR_TEST:                    return GL_BYTE;
            //case GL_SHADER_BINARY_FORMATS:           return GL_INT;
            //case GL_SHADER_COMPILER:                 return GL_BYTE;
        case GL_STENCIL_BACK_FAIL:               return GL_INT;
        case GL_STENCIL_BACK_FUNC:               return GL_INT;
        case GL_STENCIL_BACK_PASS_DEPTH_FAIL:    return GL_INT;
        case GL_STENCIL_BACK_PASS_DEPTH_PASS:    return GL_INT;
        case GL_STENCIL_BACK_REF:                return GL_INT;
        case GL_STENCIL_BACK_VALUE_MASK:         return GL_INT;
        case GL_STENCIL_BACK_WRITEMASK:          return GL_INT;
        case GL_STENCIL_BITS:                    return GL_INT;
        case GL_STENCIL_CLEAR_VALUE:             return GL_INT;
        case GL_STENCIL_FAIL:                    return GL_INT;
        case GL_STENCIL_FUNC:                    return GL_INT;
        case GL_STENCIL_PASS_DEPTH_FAIL:         return GL_INT;
        case GL_STENCIL_PASS_DEPTH_PASS:         return GL_INT;
        case GL_STENCIL_REF:                     return GL_INT;
        case GL_STENCIL_TEST:                    return GL_BYTE;
        case GL_STENCIL_VALUE_MASK:              return GL_INT;
        case GL_STENCIL_WRITEMASK:               return GL_INT;
        case GL_SUBPIXEL_BITS:                   return GL_INT;
        case GL_TEXTURE_BINDING_2D:              return GL_INT; // (* actually a CanvasTexture*)
        case GL_TEXTURE_BINDING_CUBE_MAP:        return GL_INT; // (* actually a CanvasTexture*)
        case GL_UNPACK_ALIGNMENT:                return GL_INT;
        case GL_VIEWPORT:                        return GL_INT;
    }
    
    return -1;
}

PassRefPtr<CanvasNumberArray> GraphicsContext3D::get(unsigned long pname)
{
    int size = sizeForGetParam(pname);
    if (size < 1) 
        return 0;
    
    ensureContext(m_contextObj);
    
    RefPtr<CanvasNumberArray> array = CanvasNumberArray::create(size);
    bool isAlloced = false;
    
    switch(typeForGetParam(pname)) {
        case GL_INT: {
            GLint buf[4];
            GLint* pbuf = buf;
            
            if (size > 4) {
                pbuf = (GLint*) malloc(size * sizeof(GLint));
                isAlloced = true;
            }
            
            ::glGetIntegerv(pname, pbuf);
            
            for (int i = 0; i < size; ++i)
                array->data()[i] = static_cast<float>(pbuf[i]);
            
            if (isAlloced)
                free(pbuf);
            
            break;
        }
        case GL_FLOAT: {
            GLfloat buf[4];
            GLfloat* pbuf = buf;
            
            if (size > 4) {
                pbuf = (GLfloat*) malloc(size * sizeof(GLfloat));
                isAlloced = true;
            }
            
            ::glGetFloatv(pname, pbuf);
            
            for (int i = 0; i < size; ++i)
                array->data()[i] = static_cast<float>(pbuf[i]);
            
            if (isAlloced)
                free(pbuf);
            
            break;
        }
        case GL_BYTE: {
            GLboolean buf[4];
            GLboolean* pbuf = buf;
            
            if (size > 4) {
                pbuf = (GLboolean*) malloc(size * sizeof(GLboolean));
                isAlloced = true;
            }
            
            ::glGetBooleanv(pname, pbuf);
            
            for (int i = 0; i < size; ++i)
                array->data()[i] = static_cast<float>(pbuf[i]);
            
            if (isAlloced)
                free(pbuf);
            
            break;
        }
    }
    
    return array;
}

PassRefPtr<CanvasNumberArray> GraphicsContext3D::getBufferParameter(unsigned long target, unsigned long pname)
{
    ensureContext(m_contextObj);
    RefPtr<CanvasNumberArray> array = CanvasNumberArray::create(1);
    GLint data;
    ::glGetBufferParameteriv(target, pname, &data);
    array->data()[0] = static_cast<float>(data);
    
    return array;
}

PassRefPtr<CanvasNumberArray> GraphicsContext3D::getFramebufferAttachmentParameter(unsigned long target, unsigned long attachment, unsigned long pname)
{
    ensureContext(m_contextObj);
    RefPtr<CanvasNumberArray> array = CanvasNumberArray::create(1);
    GLint data;
    ::glGetFramebufferAttachmentParameterivEXT(target, attachment, pname, &data);
    array->data()[0] = static_cast<float>(data);
    
    return array;
}

PassRefPtr<CanvasNumberArray> GraphicsContext3D::getProgram(CanvasProgram* program, unsigned long pname)
{
    if (!program)
        return 0;
    
    ensureContext(m_contextObj);
    RefPtr<CanvasNumberArray> array = CanvasNumberArray::create(1);
    GLint data;
    ::glGetProgramiv((GLuint) program->object(), pname, &data);
    array->data()[0] = static_cast<float>(data);
    
    return array;
}

String GraphicsContext3D::glGetProgramInfoLog(CanvasProgram* program)
{
    if (!program)
        return String();
    
    ensureContext(m_contextObj);
    GLint length;
    ::glGetProgramiv((GLuint) program->object(), GL_INFO_LOG_LENGTH, &length);
    
    GLsizei size;
    GLchar* info = (GLchar*) malloc(length);
    ::glGetProgramInfoLog((GLuint) program->object(), length, &size, info);
    String s(info);
    free(info);
    return s;
}

PassRefPtr<CanvasNumberArray> GraphicsContext3D::getRenderbufferParameter(unsigned long target, unsigned long pname)
{
    ensureContext(m_contextObj);
    RefPtr<CanvasNumberArray> array = CanvasNumberArray::create(1);
    GLint data;
    ::glGetBufferParameteriv(target, pname, &data);
    array->data()[0] = static_cast<float>(data);
    
    return array;
}

PassRefPtr<CanvasNumberArray> GraphicsContext3D::getShader(CanvasShader* shader, unsigned long pname)
{
    if (!shader)
        return 0;
    
    ensureContext(m_contextObj);
    RefPtr<CanvasNumberArray> array = CanvasNumberArray::create(1);
    GLint data;
    ::glGetShaderiv((GLuint) shader->object(), pname, &data);
    array->data()[0] = static_cast<float>(data);
    
    return array;
}

String GraphicsContext3D::glGetShaderInfoLog(CanvasShader* shader)
{
    if (!shader)
        return String();
    
    ensureContext(m_contextObj);
    GLint length;
    ::glGetShaderiv((GLuint) shader->object(), GL_INFO_LOG_LENGTH, &length);
    
    GLsizei size;
    GLchar* info = (GLchar*) malloc(length);
    ::glGetShaderInfoLog((GLuint) shader->object(), length, &size, info);
    String s(info);
    free(info);
    return s;
}

String GraphicsContext3D::glGetShaderSource(CanvasShader* shader)
{
    if (!shader)
        return String();
    
    ensureContext(m_contextObj);
    GLint length;
    ::glGetShaderiv((GLuint) shader->object(), GL_SHADER_SOURCE_LENGTH, &length);
    
    GLsizei size;
    GLchar* info = (GLchar*) malloc(length);
    ::glGetShaderSource((GLuint) shader->object(), length, &size, info);
    String s(info);
    free(info);
    return s;
}


PassRefPtr<CanvasNumberArray> GraphicsContext3D::getTexParameter(unsigned long target, unsigned long pname)
{
    ensureContext(m_contextObj);
    RefPtr<CanvasNumberArray> array = CanvasNumberArray::create(1);
    GLfloat data;
    ::glGetTexParameterfv(target, pname, &data);
    array->data()[0] = static_cast<float>(data);
    
    return array;
}

PassRefPtr<CanvasNumberArray> GraphicsContext3D::getUniform(CanvasProgram* program, long location, long size)
{
    if (!program)
        return 0;
    
    ensureContext(m_contextObj);
    RefPtr<CanvasNumberArray> array = CanvasNumberArray::create(size);
    
    // to avoid crashes, let's get into a buffer that will always be big enough
    GLfloat data[16];
    ::glGetUniformfv((GLuint) program->object(), location, data);
    
    for (int i = 0; i < size; ++i)
        array->data()[i] = static_cast<float>(data[i]);
    
    
    return array;
}

long GraphicsContext3D::getUniformLocation(CanvasProgram* program, const String& name)
{
    if (!program)
        return -1;
    
    ensureContext(m_contextObj);
    return ::glGetUniformLocation((GLuint) program->object(), name.utf8().data());
}

static int sizeForGetVertexAttribParam(unsigned long pname)
{
    switch(pname) {
        case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:     return 1; // (* actually a CanvasBuffer*)
        case GL_VERTEX_ATTRIB_ARRAY_ENABLED:            return 1;
        case GL_VERTEX_ATTRIB_ARRAY_SIZE:               return 1;
        case GL_VERTEX_ATTRIB_ARRAY_STRIDE:             return 1;
        case GL_VERTEX_ATTRIB_ARRAY_TYPE:               return 1;
        case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:         return 1;
        case GL_CURRENT_VERTEX_ATTRIB:                  return 4;
    }
    
    return -1;
}

static int typeForGetVertexAttribParam(unsigned long pname)
{
    switch(pname) {
        case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:     return GL_INT; // (* actually a CanvasBuffer*)
        case GL_VERTEX_ATTRIB_ARRAY_ENABLED:            return GL_INT;
        case GL_VERTEX_ATTRIB_ARRAY_SIZE:               return GL_INT;
        case GL_VERTEX_ATTRIB_ARRAY_STRIDE:             return GL_INT;
        case GL_VERTEX_ATTRIB_ARRAY_TYPE:               return GL_INT;
        case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:         return GL_INT;
        case GL_CURRENT_VERTEX_ATTRIB:                  return GL_FLOAT;
    }
    
    return -1;
}

PassRefPtr<CanvasNumberArray> GraphicsContext3D::getVertexAttrib(unsigned long index, unsigned long pname)
{
    int size = sizeForGetVertexAttribParam(pname);
    if (size < 1) 
        return 0;
    
    ensureContext(m_contextObj);
    
    RefPtr<CanvasNumberArray> array = CanvasNumberArray::create(size);
    
    switch(typeForGetVertexAttribParam(pname)) {
        case GL_INT: {
            GLint buf[4];
            ::glGetVertexAttribiv(index, pname, buf);
            
            for (int i = 0; i < size; ++i)
                array->data()[i] = static_cast<float>(buf[i]);
        }
        case GL_FLOAT: {
            GLfloat buf[4];
            
            ::glGetVertexAttribfv(index, pname, buf);
            
            for (int i = 0; i < size; ++i)
                array->data()[i] = static_cast<float>(buf[i]);
        }
    }
    
    return array;
}

// Assumes the texture you want to go into is bound
static void imageToTexture(Image* image, unsigned target, unsigned level)
{
    if (!image)
        return;
    
    CGImageRef textureImage = image->getCGImageRef();
    if (!textureImage)
        return;
    
    size_t textureWidth = CGImageGetWidth(textureImage);
    size_t textureHeight = CGImageGetHeight(textureImage);
    
    GLubyte* textureData = (GLubyte*) malloc(textureWidth * textureHeight * 4);
    CGContextRef textureContext = CGBitmapContextCreate(textureData, textureWidth, textureHeight, 8, textureWidth * 4, 
                                                        CGImageGetColorSpace(textureImage), kCGImageAlphaPremultipliedLast);
    
    CGContextDrawImage(textureContext, CGRectMake(0, 0, (CGFloat)textureWidth, (CGFloat)textureHeight), textureImage);
    CGContextRelease(textureContext);
    
    ::glTexImage2D(target, level, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    free(textureData);
}

int GraphicsContext3D::texImage2D(unsigned target, unsigned level, HTMLImageElement* image)
{
    if (!image)
        return -1;
    
    ensureContext(m_contextObj);
    CachedImage* cachedImage = image->cachedImage();
    if (!cachedImage)
        return -1;
    
    imageToTexture(cachedImage->image(), target, level);
    return 0;
}

int GraphicsContext3D::texImage2D(unsigned target, unsigned level, HTMLCanvasElement* canvas)
{
    if (!canvas)
        return -1;
    
    ensureContext(m_contextObj);
    ImageBuffer* buffer = canvas->buffer();
    if (!buffer)
        return -1;
    
    imageToTexture(buffer->image(), target, level);
    return 0;
}

int GraphicsContext3D::texSubImage2D(unsigned target, unsigned level, unsigned xoff, unsigned yoff, unsigned width, unsigned height, HTMLImageElement* image)
{
    // FIXME: we will need to deal with PixelStore params when dealing with image buffers that differ from the subimage size
    UNUSED_PARAM(target);
    UNUSED_PARAM(level);
    UNUSED_PARAM(xoff);
    UNUSED_PARAM(yoff);
    UNUSED_PARAM(width);
    UNUSED_PARAM(height);
    UNUSED_PARAM(image);
    return -1;
}

int GraphicsContext3D::texSubImage2D(unsigned target, unsigned level, unsigned xoff, unsigned yoff, unsigned width, unsigned height, HTMLCanvasElement* canvas)
{
    // FIXME: we will need to deal with PixelStore params when dealing with image buffers that differ from the subimage size
    UNUSED_PARAM(target);
    UNUSED_PARAM(level);
    UNUSED_PARAM(xoff);
    UNUSED_PARAM(yoff);
    UNUSED_PARAM(width);
    UNUSED_PARAM(height);
    UNUSED_PARAM(canvas);
    return -1;
}

unsigned GraphicsContext3D::createBuffer()
{
    ensureContext(m_contextObj);
    GLuint o;
    glGenBuffers(1, &o);
    return o;
}

unsigned GraphicsContext3D::createFramebuffer()
{
    ensureContext(m_contextObj);
    GLuint o;
    glGenFramebuffersEXT(1, &o);
    return o;
}

unsigned GraphicsContext3D::createProgram()
{
    ensureContext(m_contextObj);
    return glCreateProgram();
}

unsigned GraphicsContext3D::createRenderbuffer()
{
    ensureContext(m_contextObj);
    GLuint o;
    glGenRenderbuffersEXT(1, &o);
    return o;
}

unsigned GraphicsContext3D::createShader(ShaderType type)
{
    ensureContext(m_contextObj);
    return glCreateShader((type == FRAGMENT_SHADER) ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);
}

unsigned GraphicsContext3D::createTexture()
{
    ensureContext(m_contextObj);
    GLuint o;
    glGenTextures(1, &o);
    return o;
}

void GraphicsContext3D::deleteBuffer(unsigned buffer)
{
    ensureContext(m_contextObj);
    glDeleteBuffers(1, &buffer);
}

void GraphicsContext3D::deleteFramebuffer(unsigned framebuffer)
{
    ensureContext(m_contextObj);
    glDeleteFramebuffersEXT(1, &framebuffer);
}

void GraphicsContext3D::deleteProgram(unsigned program)
{
    ensureContext(m_contextObj);
    glDeleteProgram(program);
}

void GraphicsContext3D::deleteRenderbuffer(unsigned renderbuffer)
{
    ensureContext(m_contextObj);
    glDeleteRenderbuffersEXT(1, &renderbuffer);
}

void GraphicsContext3D::deleteShader(unsigned shader)
{
    ensureContext(m_contextObj);
    glDeleteShader(shader);
}

void GraphicsContext3D::deleteTexture(unsigned texture)
{
    ensureContext(m_contextObj);
    glDeleteTextures(1, &texture);
}

}

#endif // ENABLE(3D_CANVAS)