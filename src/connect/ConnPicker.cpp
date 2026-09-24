#include "connect/ConnPicker.h"

// Lane C0 scaffold: stub, so tests/test_conn_picker.cpp (written now,
// against this stub) FAILS -- the fail-first evidence for Lane C4, which
// replaces both bodies below with the real translation (s-rta-0923 lane 3
// plan section 5, Lane C4).

ConnSource sourceFromPicker(const PickerChoice&)
{
    return ConnSource{};   // Kind::None -- "Manual" is a pin (plan section 5 C4 / critic finding #5)
}

std::string describeSource(const ConnSource&)
{
    return "";   // "" is a pin for ConnSource{} (Manual/None) -- critic finding #5
}
