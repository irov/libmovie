cmake_minimum_required(VERSION 3.1)
set(EXAMPLE_DIR "${CMAKE_CURRENT_LIST_DIR}/example")
set(LIBMOVIE_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")
if (NOT EXISTS ${EXAMPLE_DIR}/cocos2d)
    message("Downloading cocos2dx, this may take some time!")
    file(DOWNLOAD "http://cdn.cocos2d-x.org/cocos2d-x-3.16.zip" "${EXAMPLE_DIR}/cocos2dx.zip")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar x  ${EXAMPLE_DIR}/cocos2dx.zip
        WORKING_DIRECTORY ${EXAMPLE_DIR}
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E rename
        "${EXAMPLE_DIR}/cocos2d-x-3.16" "${EXAMPLE_DIR}/cocos2d"
    )
endif()

file(COPY ${LIBMOVIE_DIR}/include/movie DESTINATION ${EXAMPLE_DIR}/Classes/)
file(COPY ${LIBMOVIE_DIR}/src DESTINATION ${EXAMPLE_DIR}/Classes/movie)
file(COPY ${LIBMOVIE_DIR}/frameworks/cocos2d-x/src DESTINATION ${EXAMPLE_DIR}/Classes/)
file(COPY ${LIBMOVIE_DIR}/examples/resources/Knight DESTINATION ${EXAMPLE_DIR}/Resources/AEM/)
file(COPY ${LIBMOVIE_DIR}/examples/resources/ui DESTINATION ${EXAMPLE_DIR}/Resources/AEM/)
file(REMOVE_RECURSE ${EXAMPLE_DIR}/Classes/AEMovie/)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E rename
    "${EXAMPLE_DIR}/Classes/src" "${EXAMPLE_DIR}/Classes/AEMovie/"
)
