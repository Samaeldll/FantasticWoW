#
# This file is part of the WarheadCore Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# User has manually chosen to ignore the git-tests, so throw them a warning.
# This is done EACH compile so they can be alerted about the consequences.
#

if(UNIX)
  # Look for an installed bzip2 on unix
  find_package(BZip2 REQUIRED)

  add_library(bzip2 SHARED IMPORTED GLOBAL)

  set_target_properties(bzip2
    PROPERTIES
      IMPORTED_LOCATION
        "${BZIP2_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES
        "${BZIP2_INCLUDE_DIRS}")
else()
  # Use the bundled source on windows
  file(GLOB sources *.c)
  add_library(bzip2 STATIC
    ${sources})

  # Group sources
  GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

  target_include_directories(bzip2
    PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR})

  target_link_libraries(bzip2
    PRIVATE
      warhead-dependency-interface)

  set_target_properties(bzip2
    PROPERTIES
      FOLDER
        "deps")
endif()
