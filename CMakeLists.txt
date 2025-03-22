cmake_minimum_required(VERSION 3.10)
project(layer1_cli C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wformat=2
        -Wno-unused-parameter
        -Wshadow
        -Wwrite-strings
        -Wstrict-prototypes
        -Wold-style-definition
        -Wredundant-decls
        -Wnested-externs
        -Wmissing-include-dirs
    )
endif()

# Find required packages
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CURL_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})

# Add cJSON as a library
add_library(cjson STATIC lib/cJSON/cJSON.c)
target_include_directories(cjson PUBLIC lib/cJSON)

# Add layer1_client library
add_library(layer1_client STATIC
    src/layer1_client.c
    src/http_signer.c
    src/arg_parser.c
    src/commands/create_address.c
    src/commands/create_address_by_asset.c
    src/commands/create_transaction.c
    src/commands/list_transactions.c
)
target_link_libraries(layer1_client cjson ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES})

# Add main executable
add_executable(layer1_cli
    src/main.c
    src/layer1_client.c
    src/http_signer.c
    src/arg_parser.c
    src/commands/create_address.c
    src/commands/create_address_by_asset.c
)
target_link_libraries(layer1_cli layer1_client)

# Install
install(TARGETS layer1_cli DESTINATION bin)