cmake_minimum_required(VERSION 3.18)

# Setup Toolchain if not specified
# Could propably avoided by using arm-none-eabi-cmake
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{DEVKITPRO})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{DEVKITPRO}/cmake/3DS.cmake" CACHE PATH "toolchain file")
    else()
        message(FATAL_ERROR "Please define DEVKITPRO to point to your SDK path!")
    endif()
endif()

find_program(MAKEROM makerom)
find_program(BANNERTOOL bannertool)

execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_SHORT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Set Result specific Data
set(APP_NAME "BCSTM-Player")
set(APP_DESC "BCSTM Player for the 3ds")
set(APP_AUTHOR "tobid7")
set(APP_ICON "${CMAKE_SOURCE_DIR}/app/icon.png")
set(APP_ROMFS "${CMAKE_SOURCE_DIR}/romfs")

set(APP_BANNER "${CMAKE_SOURCE_DIR}/app/banner.png")
set(APP_BANNERAUDIO "${CMAKE_SOURCE_DIR}/app/BannerAudio.wav")
set(APP_RSF "${CMAKE_SOURCE_DIR}/app/build-cia.rsf")
set(APP_LOGO "${CMAKE_SOURCE_DIR}/app/splash.lz")

# Set 3ds IP for make send
set(3DS_IP "" CACHE STRING "3ds IP (make send)")

# Set Project
project(BCSTM-Player LANGUAGES C CXX VERSION 2.0.0)

# Enable Compile Command Export
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Force C++ 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Better location to not always crash Citra / Azahar
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(palladium)
add_subdirectory(palladium/backends/3ds)
add_subdirectory(ctrff)

# Set Executable and its sources
add_executable(BCSTM-Player
    source/app.cpp
    source/main.cpp
    source/pd_ctr_ext.cpp
    source/colors.cpp
    source/filebrowser.cpp
    source/inspector_view.cpp
    source/inspector_view_bcwav.cpp
    source/msg_handler.cpp
    source/stagemgr.cpp
    source/lang.cpp

    source/bcstm/bcstmv2.cpp
    source/bcstm/ctrff_decode.cpp

    source/flex/objects.cpp
)

# Set dependencies, include dirs and definitions
target_link_libraries(BCSTM-Player PUBLIC m ctrff pd-3ds palladium citro3d ctrud)
target_include_directories(BCSTM-Player PUBLIC
    source
    ${DEVKITPRO}/portlibs/3ds/include
)
target_compile_definitions(BCSTM-Player PUBLIC
    -D_GNU_SOURCE=1
    -DVERSION="${PROJECT_VERSION}"
    -DGIT_COMMIT="${GIT_SHORT_HASH}"
    -DGIT_BRANCH="${GIT_BRANCH}"
)
target_compile_options(BCSTM-Player PUBLIC
    -Wno-psabi
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_options(BCSTM-Player PUBLIC
        -fno-rtti
    )
endif()

# Command to send to console (make send)
add_custom_target(
    send
    COMMAND 3dslink -a "${3DS_IP}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.3dsx"
)

# Generate 3DSX
ctr_generate_smdh(
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.smdh
    NAME "${APP_NAME}"
    DESCRIPTION "${APP_DESC}"
    AUTHOR "${APP_AUTHOR}"
    ICON "${APP_ICON}"
)
ctr_create_3dsx(
    BCSTM-Player
    OUTPUT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.3dsx"
    SMDH "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.smdh"
    ROMFS "${APP_ROMFS}"
)

if(MAKEROM AND BANNERTOOL)
    message(STATUS "Found Makerom and Bannertool. Building Cia...")

    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bnr
        COMMAND ${BANNERTOOL} makebanner
        -i ${APP_BANNER}
        -a ${APP_BANNERAUDIO}
        -o ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bnr
        DEPENDS ${APP_BANNER} ${APP_BANNERAUDIO}
        COMMENT "Creating Banner for cia file..."
        VERBATIM
    )

    add_custom_command(
        OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.cia
        COMMAND ${MAKEROM}
        -f cia -target t -exefslogo
        -o "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.cia"
        -elf "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.elf"
        -rsf ${APP_RSF}
        -banner "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bnr"
        -icon "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.smdh"
        -logo ${APP_LOGO} -DAPP_ROMFS="${APP_ROMFS}"
        -major 2 -minor 0 -micro 0
        -DAPP_VERSION_MAJOR=1
        DEPENDS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.elf ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.smdh
        COMMENT "Creating Cia file..."
        VERBATIM
    )

    add_custom_target(make_banner ALL
        DEPENDS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bnr
    )

    add_custom_target(make_cia ALL
        DEPENDS make_banner ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.elf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.cia
    )
endif()

# Add clang-format target if clang-format was found
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    message(STATUS "Clang Format found.")
    set(SOURCES ${CMAKE_SOURCE_DIR}/source)
    set(COMMENT "Formatting Project Sources")
    if(WIN32)
        add_custom_target(clang-format
            COMMAND powershell.exe -Command "Get-ChildItem '${SOURCES}/*' -Include *.cpp,*.hpp -Recurse | Foreach {&'${CLANG_FORMAT}' -i $_.fullname}"
            COMMENT ${CCOMMENT})
    else()
        add_custom_target(clang-format
            COMMAND find ${SOURCES} -iname *.hpp -o -iname *.cpp | xargs ${CLANG_FORMAT} -i
            COMMENT ${COMMENT}
        )
    endif()
    unset(COMMENT)
endif()
