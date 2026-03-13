# CompilerWarnings.cmake — Per-compiler warning flags for Audio-DNA

function(set_project_warnings target_name)
    set(MSVC_WARNINGS
        /W4
        /w14242 /w14254 /w14263 /w14265 /w14287
        /we4289 /w14296 /w14311 /w14545 /w14546
        /w14547 /w14549 /w14555 /w14619 /w14640
        /w14826 /w14905 /w14906 /w14928
    )

    set(CLANG_WARNINGS
        -Wall -Wextra -Wpedantic
        -Wshadow
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
    )

    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wuseless-cast
    )

    if(MSVC)
        target_compile_options(${target_name} PRIVATE ${MSVC_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        target_compile_options(${target_name} PRIVATE ${CLANG_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target_name} PRIVATE ${GCC_WARNINGS})
    endif()
endfunction()
