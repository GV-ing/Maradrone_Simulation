cmake_minimum_required(VERSION 3.8)
project(maradrone_slam)

find_package(ament_cmake REQUIRED)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME})

ament_package()
