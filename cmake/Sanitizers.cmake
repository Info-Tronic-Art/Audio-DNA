# Sanitizers.cmake — Sanitizer build-variant wiring for Audio-DNA
#
# ADNA_SANITIZE is a CACHE STRING taking a semicolon-separated list drawn
# from: address, undefined, thread. Default is empty (sanitizers off) — an
# empty value must produce zero flag changes vs. a pre-sanitizer build.
# `address` and `undefined` combine freely; `thread` is mutually exclusive
# with `address` (AddressSanitizer and ThreadSanitizer instrumentation
# cannot coexist in one binary) — invalid combos hit a FATAL_ERROR at
# configure time instead of failing obscurely at link time.
#
# Usage — always configure a SEPARATE build dir per variant; never point
# a sanitizer build at build/ (the live Release dir the app runs from):
#   ASan+UBSan:  cmake -S . -B build-asan -DADNA_SANITIZE="address;undefined" -DCMAKE_BUILD_TYPE=Debug
#   TSan:        cmake -S . -B build-tsan -DADNA_SANITIZE=thread -DCMAKE_BUILD_TYPE=Debug
#
# apply_sanitizers(<target>) must be called for every target (the app and
# each test executable) that should carry the instrumentation.

set(ADNA_SANITIZE "" CACHE STRING
    "Sanitizers to build with: semicolon-separated list of address, undefined, thread. Empty (default) = off, zero flag changes.")

function(apply_sanitizers target_name)
    if(NOT ADNA_SANITIZE)
        return()
    endif()

    foreach(_adna_san IN LISTS ADNA_SANITIZE)
        if(NOT _adna_san MATCHES "^(address|undefined|thread)$")
            message(FATAL_ERROR "ADNA_SANITIZE: unknown sanitizer '${_adna_san}' for target '${target_name}' (expected one of: address, undefined, thread)")
        endif()
    endforeach()

    list(FIND ADNA_SANITIZE "thread" _adna_has_thread)
    list(FIND ADNA_SANITIZE "address" _adna_has_address)
    if(_adna_has_thread GREATER -1 AND _adna_has_address GREATER -1)
        message(FATAL_ERROR "ADNA_SANITIZE: 'thread' is mutually exclusive with 'address' (got '${ADNA_SANITIZE}') for target '${target_name}' — ThreadSanitizer and AddressSanitizer instrumentation cannot coexist in one binary. Configure separate build dirs for each.")
    endif()

    if(MSVC)
        message(FATAL_ERROR "ADNA_SANITIZE is not supported with MSVC (different sanitizer flag syntax) — unset ADNA_SANITIZE for MSVC builds.")
    endif()

    string(REPLACE ";" "," _adna_san_csv "${ADNA_SANITIZE}")
    target_compile_options(${target_name} PRIVATE -fsanitize=${_adna_san_csv} -fno-omit-frame-pointer -g)
    target_link_options(${target_name} PRIVATE -fsanitize=${_adna_san_csv})
endfunction()
