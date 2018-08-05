cmake_minimum_required(VERSION 3.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

ADD_EXECUTABLE(test_create_movie_instance test_create_movie_instance.c)
TARGET_LINK_LIBRARIES(test_create_movie_instance movie)

set_target_properties (test_create_movie_instance PROPERTIES
    FOLDER tests
)

ADD_TEST(NAME create_movie_instance COMMAND ./test_create_movie_instance)

ADD_EXECUTABLE(test_create_movie_data test_create_movie_data.c)
TARGET_LINK_LIBRARIES(test_create_movie_data movie)

set_target_properties (test_create_movie_data PROPERTIES
    FOLDER tests
)

ADD_TEST(NAME create_movie_data COMMAND ./test_create_movie_data)

ADD_EXECUTABLE(test_load_movie_data test_load_movie_data.c)
TARGET_LINK_LIBRARIES(test_load_movie_data movie)

set_target_properties (test_load_movie_data PROPERTIES
    FOLDER tests
)

ADD_TEST(NAME load_movie_data COMMAND ./test_load_movie_data)