project(cstr
    LANGUAGES   C
    DESCRIPTION "String algorithms in C."
)
add_library(${PROJECT_NAME} STATIC)
add_library(${PROJECT_NAME}::framework ALIAS ${PROJECT_NAME})

configure_file(config.h.in config.h)

file(GLOB SOURCES ./*.h ./*.c)
target_sources(${PROJECT_NAME}
    PRIVATE 
        ${SOURCES}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set(${PROJECT_NAME}_PUBLIC_HEADERS
        cstr.h
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
        C_STANDARD          11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS        OFF
        PUBLIC_HEADER       "${${PROJECT_NAME}_PUBLIC_HEADERS}"
        EXPORT_NAME         framework
)

install(TARGETS   ${PROJECT_NAME}
    EXPORT        ${PROJECT_NAME}Targets
    ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT     library
)
target_compile_options(${PROJECT_NAME}
    PRIVATE
        $<$<C_COMPILER_ID:Clang>:-Wcast-align
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
                                 -Wall>
        $<$<C_COMPILER_ID:AppleClang>:-Wcast-align
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
                                      -Wall>
        $<$<C_COMPILER_ID:GNU>:-Waddress
                               -Waggregate-return
                               -Wformat-nonliteral
                               -Wformat-security
                               -Wformat
                               -Winit-self
                               -Wmissing-declarations
                               -Wmissing-include-dirs
                               -Wno-multichar
                               -Wno-parentheses
                               -Wno-type-limits
                               -Wno-unused-parameter
                               -Wunreachable-code
                               -Wwrite-strings
                               -Wpointer-arith
                               -Werror
                               -Wall>
       $<$<C_COMPILER_ID:MSVC>:/Wall>
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    # If we are building test code, then we need to include unity
    target_link_libraries(${PROJECT_NAME} testlib)
endif(CMAKE_BUILD_TYPE MATCHES Debug)
