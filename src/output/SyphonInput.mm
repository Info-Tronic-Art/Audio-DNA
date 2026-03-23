#if __APPLE__

#include "output/SyphonInput.h"
#include <iostream>

#if __has_include(<Syphon/Syphon.h>)
#define AUDIODNA_HAS_SYPHON 1
#import <Syphon/Syphon.h>
#import <OpenGL/OpenGL.h>
#import <AppKit/NSOpenGLContext.h>
#else
#define AUDIODNA_HAS_SYPHON 0
#endif

SyphonInput::SyphonInput() = default;

SyphonInput::~SyphonInput()
{
    shutdown();
}

void SyphonInput::init(void* nsOpenGLContext)
{
    context_ = nsOpenGLContext;
}

bool SyphonInput::connectToServer(const std::string& appName, const std::string& serverName)
{
#if AUDIODNA_HAS_SYPHON
    disconnect();

    if (!context_)
    {
        std::cerr << "[Syphon Input] No GL context set" << std::endl;
        return false;
    }

    @autoreleasepool {
        // Find the server description
        NSDictionary* serverDesc = nil;
        NSArray* servers = [[SyphonServerDirectory sharedDirectory] servers];

        for (NSDictionary* desc in servers)
        {
            NSString* app = desc[SyphonServerDescriptionAppNameKey];
            NSString* name = desc[SyphonServerDescriptionNameKey];

            if ([app isEqualToString:[NSString stringWithUTF8String:appName.c_str()]] &&
                [name isEqualToString:[NSString stringWithUTF8String:serverName.c_str()]])
            {
                serverDesc = desc;
                break;
            }
        }

        if (!serverDesc)
        {
            std::cerr << "[Syphon Input] Server not found: " << appName << " / " << serverName << std::endl;
            return false;
        }

        NSOpenGLContext* ctx = (__bridge NSOpenGLContext*)context_;
        SyphonClient* syphonClient = [[SyphonClient alloc] initWithServerDescription:serverDesc
                                                                             context:[ctx CGLContextObj]
                                                                             options:nil
                                                                     newFrameHandler:nil];
        if (!syphonClient)
        {
            std::cerr << "[Syphon Input] Failed to connect to server" << std::endl;
            return false;
        }

        client_ = (__bridge_retained void*)syphonClient;
        std::cerr << "[Syphon Input] Connected to: " << appName << " / " << serverName << std::endl;
        return true;
    }
#else
    (void)appName; (void)serverName;
    std::cerr << "[Syphon Input] Not available (framework not installed)" << std::endl;
    return false;
#endif
}

void SyphonInput::disconnect()
{
#if AUDIODNA_HAS_SYPHON
    if (client_)
    {
        @autoreleasepool {
            SyphonClient* c = (__bridge_transfer SyphonClient*)client_;
            [c stop];
        }
        client_ = nullptr;
        std::cerr << "[Syphon Input] Disconnected" << std::endl;
    }
#endif
}

unsigned int SyphonInput::getLatestTexture(int& outWidth, int& outHeight)
{
    outWidth = 0;
    outHeight = 0;

#if AUDIODNA_HAS_SYPHON
    if (!client_)
        return 0;

    @autoreleasepool {
        SyphonClient* c = (__bridge SyphonClient*)client_;
        SyphonImage* image = [c newFrameImage];
        if (!image)
            return 0;

        NSSize size = [image textureSize];
        outWidth = static_cast<int>(size.width);
        outHeight = static_cast<int>(size.height);
        GLuint texId = [image textureName];

        // Note: SyphonImage must be released each frame.
        // The texture is only valid until the next call to newFrameImage.
        return texId;
    }
#else
    return 0;
#endif
}

bool SyphonInput::hasNewFrame() const
{
#if AUDIODNA_HAS_SYPHON
    if (!client_)
        return false;

    @autoreleasepool {
        SyphonClient* c = (__bridge SyphonClient*)client_;
        return [c hasNewFrame];
    }
#else
    return false;
#endif
}

std::vector<std::pair<std::string, std::string>> SyphonInput::listServers()
{
    std::vector<std::pair<std::string, std::string>> result;

#if AUDIODNA_HAS_SYPHON
    @autoreleasepool {
        NSArray* servers = [[SyphonServerDirectory sharedDirectory] servers];
        for (NSDictionary* desc in servers)
        {
            NSString* app = desc[SyphonServerDescriptionAppNameKey];
            NSString* name = desc[SyphonServerDescriptionNameKey];
            result.emplace_back(
                app ? [app UTF8String] : "",
                name ? [name UTF8String] : ""
            );
        }
    }
#endif

    return result;
}

void SyphonInput::shutdown()
{
    disconnect();
    context_ = nullptr;
}

#endif // __APPLE__
