# json_converter

ROS2 message to JSON converter library using typesupport introspection.

## How to use

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
