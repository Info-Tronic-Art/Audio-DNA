#if __APPLE__

#include "output/SyphonOutput.h"
#include <iostream>

// Syphon.framework is optional — only link if available.
// When the framework is not installed, Syphon features are disabled at runtime.
// Users can install Syphon.framework to /Library/Frameworks/ to enable it.

#if __has_include(<Syphon/Syphon.h>)
#define AUDIODNA_HAS_SYPHON 1
#import <Syphon/Syphon.h>
#import <OpenGL/OpenGL.h>
#import <AppKit/NSOpenGLContext.h>
#else
#define AUDIODNA_HAS_SYPHON 0
#endif

// Internal Obj-C implementation
#if AUDIODNA_HAS_SYPHON
@interface SyphonOutputImpl : NSObject
{
    SyphonServer* _server;
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
        _server = [[SyphonServer alloc] initWithName:name
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
    std::cerr << "[Syphon] Output initialized" << std::endl;
#else
    (void)nsOpenGLContext;
    std::cerr << "[Syphon] Not available (framework not installed)" << std::endl;
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
    }
#endif
}

#endif // __APPLE__
