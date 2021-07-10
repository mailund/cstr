
add_executable(cstr_test cstr_test.c)
target_link_libraries(cstr_test cstr unity)
add_test(cstr_test cstr_test)

add_executable(alphabet_test alphabet_test.c)
target_link_libraries(alphabet_test cstr unity)
add_test(alphabet_test alphabet_test)

add_executable(sa_test sa_test.c)
target_link_libraries(sa_test cstr unity)
add_test(sa_test sa_test)

add_executable(bwt_test bwt_test.c)
target_link_libraries(bwt_test cstr unity)
add_test(bwt_test bwt_test)
