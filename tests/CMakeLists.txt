cmake_minimum_required(VERSION 3.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

ADD_EXECUTABLE(test_create test_create.c)
TARGET_LINK_LIBRARIES(test_create movie)

set_target_properties (test_create PROPERTIES
    FOLDER tests
)

ADD_TEST(NAME create COMMAND ./test_create)