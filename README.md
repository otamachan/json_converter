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
