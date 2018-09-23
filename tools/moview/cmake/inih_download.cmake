if(NOT EXISTS ${DEPENDENCIES_DIR}/inih)
    message("Downloading ini.h, this may take some time!")
    file(DOWNLOAD "https://raw.githubusercontent.com/mattiasgustavsson/libs/master/ini.h" "${DEPENDENCIES_DIR}/inih/ini.h" SHOW_PROGRESS)
endif()
