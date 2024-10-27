cmake_minimum_required(VERSION 3.11)

project("transport_catalogue" CXX)
set(CMAKE_CXX_STANDARD 17) 

set(SOURCES_DIR "sources")
set(HEADERS_DIR "headers")
set(LIBS_DIR    "libs")

# Директория библиотеки JSON
set(LIB_JSON_DIR "${LIBS_DIR}/json")
# Директория библиотеки SVG
set(LIB_SVG_DIR  "${LIBS_DIR}/svg")

include_directories(${HEADERS_DIR})
include_directories(${LIB_JSON_DIR})
include_directories(${LIB_SVG_DIR})

# Библиотека JSON
add_library("json"
            "${LIB_JSON_DIR}/json.cpp")

# Библиотека SVG
add_library("svg"
            "${LIB_SVG_DIR}/svg.cpp")

add_executable("transport_catalogue"
               "${SOURCES_DIR}/main.cpp"
               "${SOURCES_DIR}/domain.cpp"
               "${SOURCES_DIR}/geo.cpp"
               "${SOURCES_DIR}/json_reader.cpp"
               "${SOURCES_DIR}/map_renderer.cpp"
               "${SOURCES_DIR}/request_handler.cpp"
               "${SOURCES_DIR}/transport_catalogue.cpp")

target_link_libraries("transport_catalogue"
                      "json"
                      "svg")