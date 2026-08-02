// syphon_check.m
//
// Headless verification CLI for Audio-DNA's Syphon output (P22.1). Confirms
// that a Syphon server is announced and discoverable via
// SyphonServerDirectory. This needs NO GL context, NO Metal device, and NO
// NSApplication/window: SyphonServerDirectory is pure Foundation +
// distributed notifications (see .harmony/scout-syphon-sdk.md section 5).
//
// This is a gate tool, not a Catch2 unit test — the positive check (a
// matching server was found) requires a live, Syphon-enabled Audio-DNA
// instance to be running, so it's meant to be run manually / from an
// external behavioral gate after launching the app. Only the negative
// check (no matching server -> exit 1) is deterministic with no app
// running, and is the one registered as a ctest (see tests/CMakeLists.txt).
//
// Usage:
//   syphon-check [appName] [settleSeconds]
//     appName       — app name to match (default "Audio-DNA")
//     settleSeconds — run-loop settle window in seconds (default 1.0)
//
// Exit codes:
//   0 — a Syphon server from appName was found
//   1 — no matching server found (expected result with no matching app running)
//
// IMPORTANT: discovery is request/reply over NSDistributedNotificationCenter,
// not a persistent registry. SyphonServerDirectory posts an announce-request
// on init; replies only arrive while a run loop is spinning. Reading
// `.servers` immediately after construction is ALWAYS empty — this is why
// runUntilDate: below is mandatory, not a nicety.

#import <Foundation/Foundation.h>
#import <Syphon/SyphonServerDirectory.h>

int main(int argc, const char *argv[])
{
    @autoreleasepool
    {
        NSString *appName = @"Audio-DNA";
        NSTimeInterval settleSeconds = 1.0;

        if (argc > 1)
            appName = [NSString stringWithUTF8String:argv[1]];
        if (argc > 2)
            settleSeconds = atof(argv[2]);

        SyphonServerDirectory *dir = [SyphonServerDirectory sharedDirectory];

        // Mandatory settle window — see file header. Without this, `servers`
        // and `serversMatchingName:appName:` below would always be empty,
        // regardless of what is actually running.
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:settleSeconds]];

        NSArray<NSDictionary<NSString *, id<NSCoding>> *> *servers = [dir servers];
        for (NSDictionary<NSString *, id<NSCoding>> *description in servers)
        {
            // objectForKey: returns id<NSCoding> per the dictionary's declared
            // generic type; the values are actually NSString — cast before
            // calling the NSString-only UTF8String.
            NSString *seenAppName = (NSString *)[description objectForKey:SyphonServerDescriptionAppNameKey];
            NSString *seenServerName = (NSString *)[description objectForKey:SyphonServerDescriptionNameKey];
            printf("seen: %s | %s\n", [seenAppName UTF8String], [seenServerName UTF8String]);
        }

        NSArray<NSDictionary<NSString *, id<NSCoding>> *> *match =
            [dir serversMatchingName:nil appName:appName];

        if ([match count] > 0)
        {
            printf("FOUND: %lu Syphon server(s) from app '%s'\n",
                   (unsigned long)[match count], [appName UTF8String]);
            return 0;
        }

        printf("NOT FOUND: no Syphon server from app '%s' (saw %lu server(s) total)\n",
               [appName UTF8String], (unsigned long)[servers count]);
        return 1;
    }
}
