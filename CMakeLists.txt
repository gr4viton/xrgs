cmake_minimum_required(VERSION 3.6...3.31)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)

get_filename_component(PROJNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
Project(${PROJNAME} LANGUAGES C CXX)
Message(STATUS "-------------------------------")
Message(STATUS "Processing Project ${PROJNAME}:")
#####################################################################################
# look for openxr
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(TARGET_NAME openxr_loader)

if(WIN32)
  # Windows: Use specific OpenXR SDK structure
  add_library(${TARGET_NAME} STATIC IMPORTED GLOBAL)
  target_include_directories(${TARGET_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/OpenXR.Loader.1.0.34/include")
  set_target_properties(${TARGET_NAME} PROPERTIES
    IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_LIST_DIR}/3rdparty/OpenXR.Loader.1.0.34/native/x64/release/lib/openxr_loaderd.lib"
    IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/3rdparty/OpenXR.Loader.1.0.34/native/x64/release/lib/openxr_loader.lib"
    IMPORTED_LOCATION_MINSIZEREL "${CMAKE_CURRENT_LIST_DIR}/3rdparty/OpenXR.Loader.1.0.34/native/x64/release/lib/openxr_loader.lib"
    IMPORTED_LOCATION_RELWITHDEBINFO "${CMAKE_CURRENT_LIST_DIR}/3rdparty/OpenXR.Loader.1.0.34/native/x64/release/lib/openxr_loaderd.lib"
  )
else()
  # Linux: Use system-installed OpenXR libraries
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(OPENXR REQUIRED openxr)
  
  add_library(${TARGET_NAME} INTERFACE)
  target_include_directories(${TARGET_NAME} INTERFACE ${OPENXR_INCLUDE_DIRS})
  target_link_libraries(${TARGET_NAME} INTERFACE ${OPENXR_LIBRARIES})
  target_link_directories(${TARGET_NAME} INTERFACE ${OPENXR_LIBRARY_DIRS})
  target_compile_options(${TARGET_NAME} INTERFACE ${OPENXR_CFLAGS_OTHER})
endif()

# Files
set(XR_SOURCES
  "src/Common/GraphicsAPI.cpp"
  "src/Common/GraphicsAPI_Vulkan.cpp"
  "src/Common/OpenXRDebugUtils.cpp"
  "src/gameMechanics/GameBehaviour.cpp"
  "src/gameMechanics/LocomotionBehaviour.cpp"
)
set(XR_HEADERS
  "src/Common/DebugOutput.h"
  "src/Common/GraphicsAPI.h"
  "src/Common/GraphicsAPI_Vulkan.h"
  "src/Common/HelperFunctions.h"
  "src/Common/OpenXRDebugUtils.h"
  "src/Common/OpenXRHelper.h"
  "src/gameMechanics/GameBehaviour.h"
  "src/gameMechanics/LocomotionBehaviour.h"
)
#####################################################################################
# look for nvpro_core 1) as a sub-folder 2) at some other locations
# this cannot be put anywhere else since we still didn't find setup.cmake yet

# which nvprocore tag or branch to download if repo not found
set(NVPRO_GIT_TAG main)
# Where to decompress nvprocore source code if repo not found
set(NVPRO_TGT_SRC_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/_deps)

if(NOT BASE_DIRECTORY)
  find_path(BASE_DIRECTORY
    NAMES nvpro_core/cmake/setup.cmake
    PATHS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_SOURCE_DIR}/external
    DOC "Directory containing nvpro_core"
    )
endif()
if(EXISTS ${BASE_DIRECTORY}/nvpro_core/cmake/setup.cmake)
  include(${BASE_DIRECTORY}/nvpro_core/cmake/setup.cmake)
else()
  # nvpro_core not found, will try to download.
  # first find where the current sample comes from
  execute_process( 
    COMMAND git config --get remote.origin.url 
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
    OUTPUT_VARIABLE GIT_REPO_URL OUTPUT_STRIP_TRAILING_WHITESPACE 
  ) 
  # Check if "github.com" is in URL
  string(FIND "${GIT_REPO_URL}" "github.com" FOUND_INDEX)
  if (FOUND_INDEX GREATER -1)
    # Use regex to extract everything up to and including "github.com"
    string(REGEX MATCH ".*github\\.com" GIT_BASE_URL "${GIT_REPO_URL}")
    # construct URL
    string(FIND "${GIT_REPO_URL}" "git@" SSH_FOUND_INDEX)
    if (SSH_FOUND_INDEX GREATER -1) # ssh
      set(NVPRO_GIT_URL ${GIT_BASE_URL}:nvpro-samples/nvpro_core.git)
    else() # https
      set(NVPRO_GIT_URL ${GIT_BASE_URL}/nvpro-samples/nvpro_core.git)
    endif()
    if("${NVPRO_GIT_TAG}" STREQUAL "main" )
      set(NVPRO_GIT_TAG master)
    endif()
    message("Sample comes from github , nvprocore is at " ${NVPRO_GIT_URL} )
  else ()
    # reconstruct the path to nvpro_core, preserving the protocol
    string(REGEX MATCH "^[^/]+//[^/]+/" GIT_BASE_URL "${GIT_REPO_URL}")
    # construct URL
    set(NVPRO_GIT_URL ${GIT_BASE_URL}devtechproviz/nvpro-samples/nvpro_core.git)
    # message("Sample comes from prod server, nvprocore is at " ${NVPRO_GIT_URL})
  endif()
  # let's clone the commit we need, depth to 1 so that we do not download the full history
  execute_process( 
    COMMAND git clone --depth 1 --branch ${NVPRO_GIT_TAG} ${NVPRO_GIT_URL} ${CMAKE_CURRENT_BINARY_DIR}/_deps/nvpro_core
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
  ) 
  # do the search again with downloaded version, use find to be sure everyting runs ok
  find_path(BASE_DIRECTORY
    NAMES nvpro_core
    PATHS ${CMAKE_CURRENT_BINARY_DIR}/_deps
    REQUIRED
    DOC "Directory containing nvpro_core"
    )
  # invoke the setup
  if(EXISTS ${BASE_DIRECTORY}/nvpro_core/cmake/setup.cmake)
    set(OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin_x64)
    include(${BASE_DIRECTORY}/nvpro_core/cmake/setup.cmake)
  else()
    message(FATAL_ERROR "could not find base directory or download nvpro_core, please set BASE_DIRECTORY to folder containing nvpro_core")
  endif()
endif()
set(NVPRO_CORE_DIR ${BASE_DIRECTORY}/nvpro_core)

_add_project_definitions(${PROJNAME})

# Set the install diectory in the project directory
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/_install" CACHE PATH "folder in which INSTALL will put everything needed to run the binaries" FORCE)

# set with -DDISABLE_DEFAULT_SCENE=ON on command line or use cmake-gui to set teh parameter
# if default scene is not disabled at build using this option, one can still disable it by 
# providing "-loadDefaultScene 0" to the sample command line
option(DISABLE_DEFAULT_SCENE "If set to true the default scene is not downloaded by CMake, and the sample does not try to open it when starting up with no scene as parameter" OFF)

# Download the default scene
if (NOT DISABLE_DEFAULT_SCENE)
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/downloaded_resources})
        add_definitions(-DWITH_DEFAULT_SCENE_FEATURE)
        download_files(FILENAMES flowers_1.zip EXTRACT)
     endif()
endif()

#####################################################################################
# additions from packages needed for this sample
# add refs  in LIBRARIES_OPTIMIZED
# add refs  in LIBRARIES_DEBUG
# add files in PACKAGE_SOURCE_FILES

_add_package_VulkanSDK()
_add_package_ShaderC()
_add_package_IMGUI()
_add_package_NVML()

#####
# Include Vulkan Radix Sort
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/vulkan_radix_sort)

set_property(TARGET vk_radix_sort PROPERTY FOLDER "ThirdParty/vulkan_radix_sort")
set_property(TARGET spine_comp PROPERTY FOLDER "ThirdParty/vulkan_radix_sort")
set_property(TARGET downsweep_comp PROPERTY FOLDER "ThirdParty/vulkan_radix_sort")
set_property(TARGET downsweep_key_value_comp PROPERTY FOLDER "ThirdParty/vulkan_radix_sort")
set_property(TARGET upsweep_comp PROPERTY FOLDER "ThirdParty/vulkan_radix_sort")

#####################################################################################
# process the rest of some cmake code that needs to be done *after* the packages add
_add_nvpro_core_lib()

#####################################################################################
# Source files for this project
#
file(GLOB SOURCE_FILES src/*.*)
file(GLOB SHADER_FILES shaders/*.glsl shaders/*.h)
file(GLOB EXTERN_FILES 3rdparty/miniply/*.*)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src 
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders 
  ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/miniply
  ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/vulkan_radix_sort)

#####################################################################################
# Executable
#

if(WIN32 AND NOT GLUT_FOUND)
  add_definitions(/wd4996) #remove printf warning
  add_definitions(/wd4244) #remove double to float conversion warning
  add_definitions(/wd4305) #remove double to float truncation warning
else()
  add_definitions(-fpermissive)
endif()
add_executable(${PROJNAME} ${SOURCE_FILES} ${COMMON_SOURCE_FILES} ${PACKAGE_SOURCE_FILES} ${SHADER_FILES} ${EXTERN_FILES} ${XR_SOURCES} ${XR_HEADERS})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJNAME})

target_compile_definitions(${PROJNAME} PRIVATE NVPRO_CORE_DIR="${NVPRO_CORE_DIR}")

target_compile_definitions(${PROJNAME} PUBLIC XR_TUTORIAL_USE_VULKAN)

#####################################################################################
# common source code needed for this sample
#
source_group(common FILES 
  ${COMMON_SOURCE_FILES}
  ${PACKAGE_SOURCE_FILES}
)
source_group("Shader Files" FILES ${SHADER_FILES})
source_group("Source Files" FILES ${SOURCE_FILES})
source_group("Extern Files" FILES ${EXTERN_FILES})

if(UNIX)
  set(UNIXLINKLIBS dl pthread)
else()
  set(UNIXLINKLIBS)
endif()

#####################################################################################
# Linkage
#
target_include_directories(
    ${PROJNAME}
    PRIVATE
    # In this repo
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Common/
)
target_link_libraries(${PROJNAME} ${PLATFORM_LIBRARIES} nvpro_core /usr/lib/x86_64-linux-gnu/libopenxr_loader.so Vulkan::Vulkan vk_radix_sort)

foreach(DEBUGLIB ${LIBRARIES_DEBUG})
  target_link_libraries(${PROJNAME} debug ${DEBUGLIB})
endforeach(DEBUGLIB)

foreach(RELEASELIB ${LIBRARIES_OPTIMIZED})
  target_link_libraries(${PROJNAME} optimized ${RELEASELIB})
endforeach(RELEASELIB)

#####################################################################################
# copies binaries that need to be put next to the exe files (ZLib, etc.)
#

_finalize_target( ${PROJNAME} )

install(FILES ${SHADER_FILES} CONFIGURATIONS Release DESTINATION "bin_${ARCH}/GLSL_${PROJNAME}")
install(FILES ${SHADER_FILES} CONFIGURATIONS Debug DESTINATION "bin_${ARCH}_debug/GLSL_${PROJNAME}")

if(WIN32)
  set(OPENXR_LOADER_DLL_RELEASE "${CMAKE_CURRENT_LIST_DIR}/3rdparty/OpenXR.Loader.1.0.34/native/x64/release/bin/openxr_loader.dll")

  set(TARGET_OUTPUT_DIR "${CMAKE_CURRENT_LIST_DIR}/bin_x64")

  add_custom_command(TARGET ${PROJNAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${TARGET_OUTPUT_DIR}/release"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OPENXR_LOADER_DLL_RELEASE}" "${TARGET_OUTPUT_DIR}/release"
    COMMENT "Copying openxr_loader.dll to ${TARGET_OUTPUT_DIR}/release"
  )
endif()