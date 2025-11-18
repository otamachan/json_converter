# json_converter

ROS2 message to JSON converter library using typesupport introspection.

## How to use

### Using typed messages (compile-time type known)

```cpp
#include "json_converter_cpp/converter.hpp"

json_converter_cpp::Converter converter;

// ROS2 message to JSON
std_msgs::msg::String msg;
msg.data = "hello";
nlohmann::json json;
if (converter.to_json(msg, json)) {
  std::cout << json.dump() << std::endl;
}

// JSON to ROS2 message
nlohmann::json input = {{"data", "world"}};
std_msgs::msg::String output;
if (converter.to_msg(input, output)) {
  std::cout << output.data << std::endl;
}
```

### Using serialized messages (runtime type loading)

```cpp
#include "json_converter_cpp/converter.hpp"
#include <rclcpp/serialization.hpp>

json_converter_cpp::Converter converter;

// SerializedMessage to JSON (dynamic type loading)
rclcpp::SerializedMessage serialized_msg;
// ... receive serialized_msg from topic, service, etc.

nlohmann::json json;
if (converter.to_json("std_msgs/msg/String", serialized_msg, json)) {
  std::cout << json.dump() << std::endl;
}

// JSON to SerializedMessage
nlohmann::json input = {{"data", "world"}};
rclcpp::SerializedMessage output;
if (converter.to_msg("std_msgs/msg/String", input, output)) {
  // ... publish or use serialized output
}
```

### Using service types

```cpp
// Service Request
nlohmann::json json;
rclcpp::SerializedMessage serialized_request;
if (converter.to_json("test_msgs/srv/BasicTypes::Request", serialized_request, json)) {
  std::cout << json.dump() << std::endl;
}

// Service Response
rclcpp::SerializedMessage serialized_response;
if (converter.to_msg("test_msgs/srv/BasicTypes::Response", json, serialized_response)) {
  // ... use serialized response
}
```

## Example CLI tools

The package includes command-line tools for working with ROS2 topics and services using JSON.

### Echo topics as JSON

```bash
# Auto-detect topic type
ros2 run json_converter_cpp topic echo /cmd_vel

# Or specify type explicitly
ros2 run json_converter_cpp topic echo /cmd_vel geometry_msgs/msg/Twist
```

This will output each message as a single-line JSON:
```json
{"linear":{"x":0.0,"y":0.0,"z":0.0},"angular":{"x":0.0,"y":0.0,"z":0.0}}
```

### Publish JSON to topics

```bash
ros2 run json_converter_cpp topic pub /cmd_vel geometry_msgs/msg/Twist \
  '{"linear":{"x":1.0,"y":0.0,"z":0.0},"angular":{"x":0.0,"y":0.0,"z":0.0}}'
```

This will wait for at least one subscriber, publish the message once, and exit.

### Call services with JSON

```bash
ros2 run json_converter_cpp service call /add_two_ints example_interfaces/srv/AddTwoInts \
  '{"a":5,"b":3}'
```

This will call the service with the given request and output the response as JSON:
```json
{"sum":8}
```

## How to build and test

```bash
# Build the Docker container
./dev.sh build

# Build the package
./dev.sh run "colcon build"

# Run tests
./dev.sh run "colcon test"

# View test results
./dev.sh run "colcon test-result --verbose"

# Enter container shell
./dev.sh shell
```
