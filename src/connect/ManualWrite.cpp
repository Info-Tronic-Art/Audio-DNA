#include "connect/ManualWrite.h"

// Lane C0 scaffold: every function is a stub so tests/test_manual_write.cpp
// (written now, against these stubs) FAILS -- the fail-first evidence for
// Lane C1, which replaces every body below with the real implementation
// (s-rta-0923 lane 3 plan section 5, Lane C1). Parameters are intentionally
// unnamed; nothing here reads them yet.

std::optional<ControlRef> resolveControl(Composition&, MacroBank&, const ControlPath&)
{
    return std::nullopt;
}

bool gripActive(const ParamConnection&, double, float)
{
    return false;
}

bool manualTouchCore(const ControlRef&, Hand, ParamConnection::Grip::Kind, double, float)
{
    return false;
}

bool manualWriteCore(const ControlRef&, float, Hand, ParamConnection::Grip::Kind, double, float)
{
    return false;
}

void manualReleaseCore(const ControlRef&, Hand)
{
    // no-op stub
}

void disconnect(ParamConnection&, LiveValue*)
{
    // no-op stub
}
