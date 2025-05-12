add_executable(test_beb test_beb.cc)
target_link_libraries(test_beb PRIVATE distrifein)

add_test(NAME RunTests COMMAND test_beb)
