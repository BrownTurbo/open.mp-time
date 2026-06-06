cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 17)

if(NOT DEFINED UNIX)
    set(VCPKG_TARGET_TRIPLET "x86-windows")
endif()

project(omp-time LANGUAGES C CXX VERSION 1.1.2)

set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)
set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB32_PATHS ON)

file(GLOB PLUGIN_STRUCTURES "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
foreach(PROJECT_FILE IN ITEMS ${PLUGIN_STRUCTURES})
    get_filename_component(FileExt ${PROJECT_FILE} EXT)
    if(FileExt STREQUAL ".hpp" OR FileExt STREQUAL ".h")
        set(PLUGIN_HDR
            ${PLUGIN_HDR}
            ${PROJECT_FILE}
        )
    elseif(FileExt STREQUAL ".cpp" OR FileExt STREQUAL ".c" OR FileExt STREQUAL ".cc")
        set(PLUGIN_SRC
            ${PLUGIN_SRC}
            ${PROJECT_FILE}
        )
    endif()
endforeach()

add_library(omp-time SHARED)
target_sources(omp-time PUBLIC ${PLUGIN_HDR} ${PLUGIN_SRC})

set_target_properties(omp-time PROPERTIES PREFIX "")

if(DEFINED UNIX)
	target_compile_options(omp-time PRIVATE -m32)
	target_link_options(omp-time PRIVATE -m32)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/date)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/date/include)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/pawn-natives)
target_include_directories(omp-time PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/pawn-natives>
    $<INSTALL_INTERFACE:pawn-natives>
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/pawn-natives/pawn/source)
if(DEFINED UNIX)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/pawn-natives/pawn/source/linux)
endif()
file(GLOB AMX_SRC "${CMAKE_CURRENT_SOURCE_DIR}/lib/amx/*.c")
if(DEFINED UNIX)
    file(GLOB AMXL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/lib/amx/linux/*.c")
endif()
file(GLOB AMX_HDR "${CMAKE_CURRENT_SOURCE_DIR}/lib/amx/*.h")
if(DEFINED UNIX)
    file(GLOB AMXL_HDR "${CMAKE_CURRENT_SOURCE_DIR}/lib/amx/linux/*.h")
endif()
add_library(AMX ${AMX_SRC} ${AMXL_SRC} ${AMX_HDR} ${AMXL_HDR})
set_target_properties(AMX PROPERTIES LINKER_LANGUAGE CXX)
if(DEFINED UNIX)
    target_compile_options(AMX PRIVATE -m32)
    target_link_options(AMX PRIVATE -m32)
endif()
target_include_directories(AMX
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/amx>
        $<INSTALL_INTERFACE:amx>
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/omp-sdk)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/omp-sdk/include)
target_link_libraries(OMP-SDK INTERFACE
    AMX
)

file(GLOB NTP_SRC "${CMAKE_SOURCE_DIR}/lib/ntp/*.cpp")
file(GLOB UDP_SRC "${CMAKE_SOURCE_DIR}/lib/ntp/UDP/*.cpp")
file(GLOB NTP_HDR "${CMAKE_SOURCE_DIR}/lib/ntp/*.h")
file(GLOB UDP_HDR "${CMAKE_SOURCE_DIR}/lib/ntp/UDP/*.h")
add_library(NTP ${NTP_SRC} ${NTP_HDR} ${UDP_SRC} ${UDP_HDR})
set_target_properties(NTP PROPERTIES LINKER_LANGUAGE CXX)
if(DEFINED UNIX)
	target_compile_options(NTP PRIVATE -m32)
	target_link_options(NTP PRIVATE -m32)
endif()

target_link_libraries(omp-time PRIVATE
	OMP-SDK
	NTP
    pawn-natives
)

target_compile_definitions(omp-time PRIVATE
    HAVE_STDINT_H=1
    _SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING
    USE_SYSTEM_TZ_DB=ON
    ONLY_C_LOCALE=1
    PAWN_CELL_SIZE=64
)

if(NOT DEFINED UNIX)
    if(DEFINED CMAKE_CXX_COMPILER_ID)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0 /Wv:18 /p:VcpkgEnableManifest=true")
    elseif(DEFINED CMAKE_C_COMPILER_ID)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W0 /Wv:18 /p:VcpkgEnableManifest=true")
    endif()
endif()

# no idea why this is necessary...
if(UNIX)
	set(ENABLE_DATE_TESTING OFF CACHE BOOL "Enable unit tests")
	set(USE_SYSTEM_TZ_DB ON CACHE BOOL "Use the operating system's timezone database")
elseif(WIN32)
	set(ENABLE_DATE_TESTING OFF)
	set(USE_SYSTEM_TZ_DB ON)
endif()
