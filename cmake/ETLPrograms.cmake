if(FEATURE_RENDERER2)
	# Find GNU sed executable, BSD variant does not work with our script
	if(UNIX AND NOT CMAKE_SYSTEM_NAME MATCHES "Linux")
		find_program(SED_EXECUTABLE gsed)
	else()
		find_program(SED_EXECUTABLE sed)
	endif()
endif(FEATURE_RENDERER2)

# Enable pk3 creation if zip executable is found. Download it on Windows
find_program(ZIP_EXECUTABLE zip PATHS ${CMAKE_BINARY_DIR})
if(NOT ZIP_EXECUTABLE AND WIN32)
	message(STATUS "Downloading zip.exe to " ${CMAKE_BINARY_DIR}/zip.exe)
	file(DOWNLOAD http://stahlworks.com/dev/zip.exe ${CMAKE_BINARY_DIR}/zip.exe SHOW_PROGRESS TIMEOUT 10)
	find_program(ZIP_EXECUTABLE zip PATHS ${CMAKE_BINARY_DIR})
endif()
find_program(UNZIP_EXECUTABLE unzip PATHS ${CMAKE_BINARY_DIR})
if(NOT UNZIP_EXECUTABLE AND WIN32)
	message(STATUS "Downloading unzip.exe to " ${CMAKE_BINARY_DIR}/unzip.exe)
	file(DOWNLOAD http://stahlworks.com/dev/unzip.exe ${CMAKE_BINARY_DIR}/unzip.exe SHOW_PROGRESS TIMEOUT 10)
	find_program(UNZIP_EXECUTABLE unzip PATHS ${CMAKE_BINARY_DIR})
endif()
