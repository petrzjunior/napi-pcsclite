# CMake config for usage with cmake-js
# see https://github.com/cmake-js/cmake-js/wiki/TUTORIAL-01-Creating-a-native-module-by-using-CMake.js-and-NAN

# Usage in CLion:
#   1. Ensure you have cmake-js installed globally (if not run `npm -g i cmake-js` or `yarn global add cmake-js`).
#   2. In CLion, open Preferences > Build, Execution, Deployment > CMake and:
#      1. Remove Debug config
#      2. Add new config with the following values
#           Name: Node.js Release
#           Build type: Release
#           Generation path: build
#      3. Click "Apply" and "OK"
#   3. Then run `cmake-js configure` in project root
#   4. In CLion, open CMake window and click reload
#   5. CLion should now provide full code completion and analysis

cmake_minimum_required(VERSION 3.1)

# name of the project (will be the name of the plugin)
project(pcsclite)

# Build a shared library named after the project from the files in src/
file(GLOB SOURCE_FILES "src/*.c" "src/*.h")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})

# Gives our library file a .node extension without any "lib" prefix
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Essential include files to build a node addon,
# You should add this line in every CMake.js based project
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC})

# Essential library files to link to a node addon
# You should add this line in every CMake.js based project
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})

find_path(PCSC_INCLUDE NAMES WinSCard.h pcsclite.h PATH_SUFFIXES PCSC)
find_library(PCSC_LIB NAMES winscard PCSC pcsclite)
message("-- PCSC include: " ${PCSC_INCLUDE})
message("-- PCSC lib: " ${PCSC_LIB})

# Link pcsc library using found paths
target_include_directories(${PROJECT_NAME} PRIVATE ${PCSC_INCLUDE})
#link_directories(${PCSC_LIBDIR})
target_link_libraries(${PROJECT_NAME} ${PCSC_LIB})

# Set C99 and C++11 standard
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
