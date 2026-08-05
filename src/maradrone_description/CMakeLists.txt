cmake_minimum_required(VERSION 3.8)
project(maradrone_description)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)

# Assicuriamoci che tutte le directory essenziali vengano copiate in /install durante il colcon build
install(DIRECTORY
  launch
  models
  urdf
  worlds
  conf
  DESTINATION share/${PROJECT_NAME}
)

ament_package()