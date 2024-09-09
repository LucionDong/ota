cmake_minimum_required(VERSION 3.13)
project(ota_update VERSION 1.0)

enable_language(C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
add_compile_definitions(NEU_PLATFORM_LINUX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

if(NOT CMAKE_SYSTEM_NAME)
  set(CMAKE_SYSTEM_NAME "Linux")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always -Wall -Wextra -g -lpaho-mqtt3a -lpaho-mqtt3c -ljansson -larchive -lcurl -lssl -lcrypto")
set(CMAKE_C_FLAGS_RELEASE "-O1")

if(NOT DISABLE_ASAN)
endif()

if (CMAKE_STAGING_PREFIX)
else()
endif()

set(EASE_CPM_OTA_SRC
	src/mqtt5_test.c
	# src/mqtt_async.c
	# src/parse_json.c
	src/culr_install.c
	src/parse_json.c
	src/read_file.c
	src/log.c
	src/version_hash.c
	# src/rs232_recv.c
	# src/main.c
	# src/mcurs232/mcurs232_helper_functions.c
	# src/mcurs232/serial_port.c
	)

add_executable (${PROJECT_NAME} 
	${EASE_CPM_OTA_SRC}
	)
# add_executable(${PROJECT_NAME# }
	# ${LNX-DMX_SRC}
	# )
target_include_directories(${PROJECT_NAME} PRIVATE 
	# ${CMAKE_CURRENT_SOURCE_DIR}/../../include/core-plugin-manager
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/include
	)

# message(STATUS include: ${CMAKE_CURRENT_SOURCE_DIR}/../../include/core-plugin-manager)
message(STATUS PROJECT_NAME: ${PROJECT_NAME})

# find_library(DL_LIB NAMES dl REQUIRED)
# find_library(MATH_LIB NAMES m REQUIRED)
# find_library(PTHREAD_LIB NAMES pthread REQUIRED)
# find_library(ZLOG_LIB NAMES zlog REQUIRED)
# find_library( JANSSON_LIB NAMES jansson REQUIRED)
# find_library(NNG_LIB NAMES nng REQUIRED)
# find_library(JWT_LIB NAMES jwt REQUIRED)
# find_library(CRYPTO_LIB NAMES crypto REQUIRED)
# find_library(SSL_LIB NAMES ssl REQUIRED)
# find_library(SQLITE3_LIB NAMES sqlite3 REQUIRED)
# find_library(ESVCPMBASE_LIB NAMES esvcpm-base REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE 
   #  ${DL_LIB}
	# ${MATH_LIB}
	# ${PTHREAD_LIB}
	# ${ZLOG_LIB}
	# ${JANSSON_LIB}
	# ${NNG_LIB}
	# ${JWT_LIB}
	# ${CRYPTO_LIB}
	# ${SSL_LIB}
   #  ${SQLITE3_LIB}
	# ${ESVCPMBASE_LIB}
	)

