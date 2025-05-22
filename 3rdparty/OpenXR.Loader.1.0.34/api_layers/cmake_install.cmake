# Install script for directory: E:/aaaPROJECT/Chapter3/openxr/src/api_layers

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/openxr-tutorial")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Layers" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/api_layers" TYPE FILE FILES "E:/aaaPROJECT/Chapter3/build2/_deps/openxr-build/src/api_layers/XrApiLayer_api_dump.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Layers" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/api_layers" TYPE MODULE FILES "E:/aaaPROJECT/Chapter3/build2/_deps/openxr-build/src/api_layers/XrApiLayer_api_dump.dll")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/api_layers" TYPE MODULE FILES "E:/aaaPROJECT/Chapter3/build2/_deps/openxr-build/src/api_layers/XrApiLayer_api_dump.dll")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Layers" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    include("E:/aaaPROJECT/Chapter3/build2/_deps/openxr-build/src/api_layers/CMakeFiles/XrApiLayer_api_dump.dir/install-cxx-module-bmi-Debug.cmake" OPTIONAL)
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    include("E:/aaaPROJECT/Chapter3/build2/_deps/openxr-build/src/api_layers/CMakeFiles/XrApiLayer_api_dump.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Layers" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/api_layers" TYPE FILE FILES "E:/aaaPROJECT/Chapter3/build2/_deps/openxr-build/src/api_layers/XrApiLayer_core_validation.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Layers" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/api_layers" TYPE MODULE FILES "E:/aaaPROJECT/Chapter3/build2/_deps/openxr-build/src/api_layers/XrApiLayer_core_validation.dll")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/api_layers" TYPE MODULE FILES "E:/aaaPROJECT/Chapter3/build2/_deps/openxr-build/src/api_layers/XrApiLayer_core_validation.dll")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Layers" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    include("E:/aaaPROJECT/Chapter3/build2/_deps/openxr-build/src/api_layers/CMakeFiles/XrApiLayer_core_validation.dir/install-cxx-module-bmi-Debug.cmake" OPTIONAL)
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    include("E:/aaaPROJECT/Chapter3/build2/_deps/openxr-build/src/api_layers/CMakeFiles/XrApiLayer_core_validation.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
  endif()
endif()

