#if __APPLE__

#include "output/SyphonOutput.h"
#include <iostream>

// AUDIODNA_HAS_SYPHON is defined by CMake (=1) when the Syphon OpenGL-subset
// sources are vendored and compiled into this target (see the Syphon block
// in CMakeLists.txt). When Syphon isn't being built (non-Apple, or
// AUDIODNA_BUILD_SYPHON=OFF), CMake leaves the macro undefined and the
// #ifndef below supplies 0.
//
// This file must never redefine a value CMake already set. It previously
// guessed availability itself via #if __has_include(<Syphon/Syphon.h>),
// which always evaluated false because the framework search path (-F) was
// never passed to the compiler — so this file's own #define silently
// overrode CMake's -DAUDIODNA_HAS_SYPHON=1 with 0 (a -Wmacro-redefined
// warning, not promoted to an error anywhere in this project), producing a
// build that linked and toggled ON in the UI while never publishing a frame.
#ifndef AUDIODNA_HAS_SYPHON
#define AUDIODNA_HAS_SYPHON 0
#endif

#if AUDIODNA_HAS_SYPHON
// glFlush (below) is declared in gl.h, not OpenGL.h (the CGL/platform
// umbrella) — silence its 10.14 deprecation notice consistently with how
// the vendored Syphon sources silence theirs (CMakeLists.txt).
#define GL_SILENCE_DEPRECATION 1
// SyphonOpenGLServer directly (not the umbrella Syphon.h, which also pulls
// in the Metal server/client headers we don't compile — see CMakeLists.txt).
#import <Syphon/SyphonOpenGLServer.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
// NSOpenGLContext lives in NSOpenGL.h, not a same-named NSOpenGLContext.h
// (that path doesn't exist in the SDK; this import was never previously
// exercised — see the AUDIODNA_HAS_SYPHON comment above).
#import <AppKit/NSOpenGL.h>
#endif

// Internal Obj-C implementation
#if AUDIODNA_HAS_SYPHON
@interface SyphonOutputImpl : NSObject
{
    SyphonOpenGLServer* _server;
    NSOpenGLContext* _glContext;
}
- (instancetype)initWithContext:(NSOpenGLContext*)ctx name:(NSString*)name;
- (void)publishTexture:(GLuint)texId width:(int)w height:(int)h;
- (void)setName:(NSString*)name;
- (void)shutdown;
@end

@implementation SyphonOutputImpl

- (instancetype)initWithContext:(NSOpenGLContext*)ctx name:(NSString*)name
{
    self = [super init];
    if (self)
    {
        _glContext = ctx;
        _server = [[SyphonOpenGLServer alloc] initWithName:name
                                                    context:[ctx CGLContextObj]
                                                    options:nil];
        if (!_server)
            NSLog(@"[Syphon] Failed to create server");
        else
            NSLog(@"[Syphon] Server created: %@", name);
    }
    return self;
}

- (void)publishTexture:(GLuint)texId width:(int)w height:(int)h
{
    if (_server)
    {
        // publishFrameTexture: makes no documented flush guarantee (only the
        // bindToDrawFrameOfSize:/unbindAndPublish alternative promises one:
        // "This method will flush the GL context (so you don't have to)").
        // We stay on the publishFrameTexture: path — the caller (Renderer)
        // already blits into its own dedicated texture before calling here,
        // which is simpler to reason about than handing the render target
        // itself over to Syphon — so an explicit flush is needed to ensure
        // the just-blitted texture contents are actually visible to Syphon's
        // internal IOSurface copy. glFlush (not glFinish): Syphon's own
        // "flush" guarantee is a flush, and this project deliberately avoids
        // glFinish() elsewhere for pipeline-stall reasons (Renderer.cpp).
        glFlush();
        [_server publishFrameTexture:texId
                       textureTarget:GL_TEXTURE_2D
                         imageRegion:NSMakeRect(0, 0, w, h)
                   textureDimensions:NSMakeSize(w, h)
                             flipped:NO];
    }
}

- (void)setName:(NSString*)name
{
    if (_server)
        [_server setName:name];
}

- (void)shutdown
{
    if (_server)
    {
        [_server stop];
        _server = nil;
        NSLog(@"[Syphon] Server stopped");
    }
    _glContext = nil;
}

@end
#endif // AUDIODNA_HAS_SYPHON

// C++ wrapper implementation

SyphonOutput::SyphonOutput() = default;

SyphonOutput::~SyphonOutput()
{
    shutdown();
}

void SyphonOutput::init(void* nsOpenGLContext)
{
#if AUDIODNA_HAS_SYPHON
    if (impl_)
        shutdown();

    @autoreleasepool {
        NSOpenGLContext* ctx = (__bridge NSOpenGLContext*)nsOpenGLContext;
        NSString* name = [NSString stringWithUTF8String:serverName_.c_str()];
        SyphonOutputImpl* impl = [[SyphonOutputImpl alloc] initWithContext:ctx name:name];
        impl_ = (__bridge_retained void*)impl;
    }
    initialized_.store(true, std::memory_order_relaxed);
    std::cerr << "[Syphon] Output initialized" << std::endl;
#else
    (void)nsOpenGLContext;
    std::cerr << "[Syphon] Not available (not built — see AUDIODNA_BUILD_SYPHON)" << std::endl;
#endif
}

void SyphonOutput::publishTexture(unsigned int texId, int width, int height)
{
#if AUDIODNA_HAS_SYPHON
    if (!impl_ || !enabled_.load(std::memory_order_relaxed))
        return;

    @autoreleasepool {
        SyphonOutputImpl* impl = (__bridge SyphonOutputImpl*)impl_;
        [impl publishTexture:static_cast<GLuint>(texId) width:width height:height];
    }
#else
    (void)texId; (void)width; (void)height;
#endif
}

void SyphonOutput::setServerName(const std::string& name)
{
    serverName_ = name;
#if AUDIODNA_HAS_SYPHON
    if (impl_)
    {
        @autoreleasepool {
            SyphonOutputImpl* impl = (__bridge SyphonOutputImpl*)impl_;
            [impl setName:[NSString stringWithUTF8String:name.c_str()]];
        }
    }
#endif
}

void SyphonOutput::shutdown()
{
#if AUDIODNA_HAS_SYPHON
    if (impl_)
    {
        @autoreleasepool {
            SyphonOutputImpl* impl = (__bridge_transfer SyphonOutputImpl*)impl_;
            [impl shutdown];
        }
        impl_ = nullptr;
        initialized_.store(false, std::memory_order_relaxed);
    }
#endif
}

#endif // __APPLE__
