cmake_minimum_required(VERSION 3.8)
project(json_converter_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rosidl_typesupport_introspection_cpp REQUIRED)
find_package(RapidJSON REQUIRED)

# Create library
add_library(${PROJECT_NAME} SHARED
  src/converter.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
  ${RAPIDJSON_INCLUDE_DIRS}
)

ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  rosidl_typesupport_introspection_cpp
)

target_link_libraries(${PROJECT_NAME}
  ${CMAKE_DL_LIBS}
)

# Install library
install(TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Install headers
install(DIRECTORY include/
  DESTINATION include
)

# Example executables
add_executable(topic example/topic.cpp)
target_link_libraries(topic ${PROJECT_NAME})
ament_target_dependencies(topic rclcpp)

add_executable(service example/service.cpp)
target_link_libraries(service ${PROJECT_NAME})
ament_target_dependencies(service rclcpp)

install(TARGETS topic service
  DESTINATION lib/${PROJECT_NAME}
)

# Export dependencies
ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(
  rclcpp
  rosidl_typesupport_introspection_cpp
  nlohmann_json
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  find_package(ament_cmake_gtest REQUIRED)
  find_package(std_msgs REQUIRED)
  find_package(std_srvs REQUIRED)
  find_package(geometry_msgs REQUIRED)
  find_package(test_msgs REQUIRED)

  ament_add_gtest(test_converter test/test_converter.cpp)
  target_link_libraries(test_converter ${PROJECT_NAME})
  ament_target_dependencies(test_converter
    std_msgs
    std_srvs
    geometry_msgs
    test_msgs
  )
endif()

ament_package()
