
# make all files that end in _test.c into tests
file(GLOB test_sources *_test.c)
foreach(test_c ${test_sources})
    get_filename_component(testname ${test_c} NAME_WE)
    add_executable(${testname} ${test_c})
    target_link_libraries(${testname} cstr unity)
    add_test(
        ${testname} ${testname}
    )
    set_target_properties(
        ${testname} PROPERTIES FOLDER Tests
    )
endforeach(test_c)
