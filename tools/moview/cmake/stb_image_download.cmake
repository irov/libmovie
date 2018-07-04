if(NOT EXISTS ${DEPENDENCIES_DIR}/stb)
	message("Downloading stb_image, this may take some time!")
	file(DOWNLOAD "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" "${DEPENDENCIES_DIR}/stb/stb_image.h" SHOW_PROGRESS)
endif()