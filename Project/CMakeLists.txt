# CMakeList.txt : CMake project for Project, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (Project "Project.cpp" "Project.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Project PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
