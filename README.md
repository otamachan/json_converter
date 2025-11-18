# json_converter

ROS2 message to JSON converter library using typesupport introspection.

This library uses [RapidJSON](https://rapidjson.org/) for fast and efficient JSON processing.

## How to use

### Using typed messages (compile-time type known)

```cpp
#include "json_converter_cpp/converter.hpp"
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

json_converter_cpp::Converter converter;

// ROS2 message to JSON
std_msgs::msg::String msg;
msg.data = "hello";

rapidjson::Document doc;
doc.SetObject();
auto& allocator = doc.GetAllocator();

if (converter.to_json(msg, doc, allocator)) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);
  std::cout << buffer.GetString() << std::endl;
}

// JSON to ROS2 message
rapidjson::Document input;
input.SetObject();
input.AddMember("data", "world", input.GetAllocator());

std_msgs::msg::String output;
if (converter.to_msg(input, output)) {
  std::cout << output.data << std::endl;
}
```

### Using serialized messages (runtime type loading)

```cpp
#include "json_converter_cpp/converter.hpp"
#include <rclcpp/serialization.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

json_converter_cpp::Converter converter;

// SerializedMessage to JSON (dynamic type loading)
rclcpp::SerializedMessage serialized_msg;
// ... receive serialized_msg from topic, service, etc.

rapidjson::Document doc;
doc.SetObject();
auto& allocator = doc.GetAllocator();

if (converter.to_json("std_msgs/msg/String", serialized_msg, doc, allocator)) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);
  std::cout << buffer.GetString() << std::endl;
}

// JSON to SerializedMessage
rapidjson::Document input;
input.SetObject();
input.AddMember("data", "world", input.GetAllocator());

rclcpp::SerializedMessage output;
if (converter.to_msg("std_msgs/msg/String", input, output)) {
  // ... publish or use serialized output
}
```

### Using service types (with GenericClient)

```cpp
// JSON to message object (for service request)
rapidjson::Document request_doc;
request_doc.Parse(R"({"a": 5, "b": 3})");

std::shared_ptr<void> request_msg;
if (converter.to_msg("example_interfaces/srv/AddTwoInts::Request", request_doc, request_msg)) {
  // Use request_msg with GenericClient
}

// Message object to JSON (for service response)
void* response_ptr = /* ... from GenericClient ... */;
rapidjson::Document response_doc;
response_doc.SetObject();
auto& allocator = response_doc.GetAllocator();

if (converter.to_json("example_interfaces/srv/AddTwoInts::Response", response_ptr,
                      response_doc, allocator)) {
  // Output: {"sum":8}
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
