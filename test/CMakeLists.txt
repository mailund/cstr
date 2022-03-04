
add_library(testlib STATIC)
target_sources(testlib 
    PRIVATE
        testlib.h testlib.c
)
target_include_directories(testlib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)
set_target_properties(testlib
    PROPERTIES FOLDER Tests/TestLib
)

# while this creates a circular dependency, we need it for
# generating test strings, and it is only setting up the
# dependency for header inclusion and linking.
target_link_libraries(testlib cstr)

# make all files that end in _test.c into tests
file(GLOB test_sources *_test.c)
foreach(test_c ${test_sources})
    get_filename_component(testname ${test_c} NAME_WE)
    add_executable(${testname} ${test_c})
    # FIXME: make this portable (but right now, the variable I used to switch on
    # doesn't seem to be set, so that didn't work).
    target_compile_options(${testname}
        PRIVATE
            -Wcast-align
            -Wcast-qual
            -Wconversion
            -Wexit-time-destructors
            -Wglobal-constructors
            -Wmissing-noreturn
            -Wmissing-prototypes
            -Wno-missing-braces
            -Wold-style-cast
            -Wshadow
            -Wweak-vtables
            -Werror
            -Wall
            #-O3
    )
    target_link_libraries(${testname} cstr testlib)
    add_test(
        ${testname} ${testname}
    )
    set_target_properties(
        ${testname} PROPERTIES FOLDER Tests
    )
endforeach(test_c)
