project(mtp)

set(CPACK_PACKAGE_VERSION "2.4")
set(CPACK_PACKAGE_VERSION_MAJOR 2)
set(CPACK_PACKAGE_VERSION_MAJOR 4)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_SOURCE_DIR}/README.md)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Android File Transfer for Linux (and Mac OS X) - open source implementation of MTP protocol used on every modern android device")

message(STATUS "building for ${CMAKE_SYSTEM_NAME}")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(USB_BACKEND_DARWIN TRUE)
	#set(CPACK_GENERATOR Bundle)
endif()

include(CPack)

cmake_minimum_required(VERSION 2.8)
set(CMAKE_USE_RELATIVE_PATHS TRUE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

include(FindPkgConfig)
include(CheckFunctionExists)

find_package ( Threads )

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	pkg_check_modules ( FUSE fuse )
endif()

if (FUSE_FOUND)
	message(STATUS "fuse found, building mount helper")
	add_definitions(${FUSE_CFLAGS} -DFUSE_USE_VERSION=26)
endif()

option(BUILD_QT_UI "Build reference Qt application" ON)
option(BUILD_SHARED_LIB "Build shared library" OFF)

option(USB_BACKEND_LIBUSB "Use libusb-1.0" OFF)
if (USB_BACKEND_LIBUSB)
	message(WARNING "WARNING! You're using libusb, this is known to be broken -- large memory consumption, violating kernel memory limits and bugs. Continue at your own risk")
endif()

add_definitions(-Wall -pthread -std=c++11)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	add_definitions(-ggdb)
endif()

include_directories(. ${CMAKE_SOURCE_DIR})

set(SOURCES
	mtp/ptp/Device.cpp
	mtp/ptp/ObjectFormat.cpp
	mtp/ptp/PipePacketer.cpp
	mtp/ptp/Response.cpp
	mtp/ptp/Session.cpp

	mtp/usb/BulkPipe.cpp
)

set(MTP_LIBRARIES mtp-ng-static)

if (USB_BACKEND_LIBUSB)

	pkg_check_modules(LIBUSB libusb-1.0 REQUIRED)
	include_directories(${LIBUSB_INCLUDE_DIRS})
	include_directories(mtp/backend/libusb)

	list(APPEND SOURCES
		mtp/backend/libusb/usb/Context.cpp
		mtp/backend/libusb/usb/Device.cpp
		mtp/backend/libusb/usb/DeviceDescriptor.cpp
		mtp/backend/libusb/usb/Exception.cpp
	)
	list(APPEND MTP_LIBRARIES ${LIBUSB_LIBRARIES})
elseif (USB_BACKEND_DARWIN)
	find_library(CORE_LIBRARY CoreFoundation)
	find_library(IOKIT_LIBRARY IOKit)
	list(APPEND MTP_LIBRARIES ${IOKIT_LIBRARY} ${CORE_LIBRARY})
	include_directories(mtp/backend/darwin)
	list(APPEND SOURCES
		mtp/backend/darwin/usb/Context.cpp
		mtp/backend/darwin/usb/Device.cpp
		mtp/backend/darwin/usb/DeviceDescriptor.cpp
		mtp/backend/darwin/usb/Exception.cpp
		mtp/backend/darwin/usb/Interface.cpp
	)
else()
	include_directories(mtp/backend/linux)
	list(APPEND SOURCES
		mtp/backend/linux/usb/Exception.cpp
		mtp/backend/linux/usb/Context.cpp
		mtp/backend/linux/usb/Device.cpp
		mtp/backend/linux/usb/Interface.cpp
		mtp/backend/linux/usb/DeviceDescriptor.cpp
	)
endif()


add_library(mtp-ng-static STATIC ${SOURCES})
if (BUILD_SHARED_LIB)
	add_library(mtp-ng SHARED ${SOURCES})
	target_link_libraries(mtp-ng ${CMAKE_THREAD_LIBS_INIT})

	if (USB_BACKEND_LIBUSB)
		target_link_libraries(mtp-ng ${LIBUSB_LIBRARIES})
	endif()
endif (BUILD_SHARED_LIB)

add_definitions(-D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64)

add_subdirectory(cli)

if (FUSE_FOUND)
	add_subdirectory(fuse)
endif()

if (BUILD_QT_UI)
	add_subdirectory(qt)
endif()
