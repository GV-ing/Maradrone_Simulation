cmake_minimum_required(VERSION 3.8)
project(maradrone_framework)

# 1. Dipendenze
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(px4_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)

# 2. Eseguibile
add_executable(offboard_takeoff src/offboard_takeoff.cpp)
target_include_directories(offboard_takeoff PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
  
# Importante: linkare le px4_msgs
ament_target_dependencies(offboard_takeoff
  rclcpp
  px4_msgs
  geometry_msgs
)

# 3. Installazione
install(TARGETS offboard_takeoff
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME})

install(DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)

ament_package()