// Copyright 2025 Tamaki Nishino
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>

#include <geometry_msgs/msg/point.hpp>
#include <geometry_msgs/msg/polygon.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <rclcpp/serialization.hpp>
#include <std_msgs/msg/int32.hpp>
#include <std_msgs/msg/int32_multi_array.hpp>
#include <std_msgs/msg/string.hpp>
#include <test_msgs/msg/arrays.hpp>
#include <test_msgs/msg/basic_types.hpp>
#include <test_msgs/srv/basic_types.hpp>
#include <test_msgs/srv/empty.hpp>

#include "json_converter_cpp/converter.hpp"

using json_converter_cpp::Converter;

TEST(ConverterTest, StringMessageToJson)
{
  Converter converter;
  std_msgs::msg::String msg;
  msg.data = "hello world";

  nlohmann::json json;
  ASSERT_TRUE(converter.to_json(msg, json));
  EXPECT_EQ(json["data"], "hello world");
}

TEST(ConverterTest, JsonToStringMessage)
{
  Converter converter;
  nlohmann::json json;
  json["data"] = "test message";

  std_msgs::msg::String msg;
  ASSERT_TRUE(converter.to_msg(json, msg));
  EXPECT_EQ(msg.data, "test message");
}

TEST(ConverterTest, Int32ArrayToJson)
{
  Converter converter;
  std_msgs::msg::Int32MultiArray msg;
  msg.data = {1, 2, 3, 4, 5};

  nlohmann::json json;
  ASSERT_TRUE(converter.to_json(msg, json));
  ASSERT_TRUE(json["data"].is_array());
  EXPECT_EQ(json["data"].size(), 5);
  EXPECT_EQ(json["data"][0], 1);
  EXPECT_EQ(json["data"][4], 5);
}

TEST(ConverterTest, JsonToInt32Array)
{
  Converter converter;
  nlohmann::json json;
  json["data"] = {10, 20, 30};

  std_msgs::msg::Int32MultiArray msg;
  ASSERT_TRUE(converter.to_msg(json, msg));
  ASSERT_EQ(msg.data.size(), 3);
  EXPECT_EQ(msg.data[0], 10);
  EXPECT_EQ(msg.data[1], 20);
  EXPECT_EQ(msg.data[2], 30);
}

TEST(ConverterTest, NestedMessageToJson)
{
  Converter converter;
  geometry_msgs::msg::Pose msg;
  msg.position.x = 1.0;
  msg.position.y = 2.0;
  msg.position.z = 3.0;
  msg.orientation.x = 0.0;
  msg.orientation.y = 0.0;
  msg.orientation.z = 0.0;
  msg.orientation.w = 1.0;

  nlohmann::json json;
  ASSERT_TRUE(converter.to_json(msg, json));
  EXPECT_DOUBLE_EQ(json["position"]["x"], 1.0);
  EXPECT_DOUBLE_EQ(json["position"]["y"], 2.0);
  EXPECT_DOUBLE_EQ(json["position"]["z"], 3.0);
  EXPECT_DOUBLE_EQ(json["orientation"]["w"], 1.0);
}

TEST(ConverterTest, JsonToNestedMessage)
{
  Converter converter;
  nlohmann::json json;
  json["position"]["x"] = 5.0;
  json["position"]["y"] = 6.0;
  json["position"]["z"] = 7.0;
  json["orientation"]["x"] = 0.0;
  json["orientation"]["y"] = 0.0;
  json["orientation"]["z"] = 0.0;
  json["orientation"]["w"] = 1.0;

  geometry_msgs::msg::Pose msg;
  ASSERT_TRUE(converter.to_msg(json, msg));
  EXPECT_DOUBLE_EQ(msg.position.x, 5.0);
  EXPECT_DOUBLE_EQ(msg.position.y, 6.0);
  EXPECT_DOUBLE_EQ(msg.position.z, 7.0);
  EXPECT_DOUBLE_EQ(msg.orientation.w, 1.0);
}

TEST(ConverterTest, ArrayOfNestedMessagesToJson)
{
  Converter converter;
  geometry_msgs::msg::Polygon msg;

  geometry_msgs::msg::Point32 p1;
  p1.x = 1.0;
  p1.y = 2.0;
  p1.z = 0.0;

  geometry_msgs::msg::Point32 p2;
  p2.x = 3.0;
  p2.y = 4.0;
  p2.z = 0.0;

  geometry_msgs::msg::Point32 p3;
  p3.x = 5.0;
  p3.y = 6.0;
  p3.z = 0.0;

  msg.points = {p1, p2, p3};

  nlohmann::json json;
  ASSERT_TRUE(converter.to_json(msg, json));
  ASSERT_TRUE(json["points"].is_array());
  EXPECT_EQ(json["points"].size(), 3);
  EXPECT_FLOAT_EQ(json["points"][0]["x"], 1.0);
  EXPECT_FLOAT_EQ(json["points"][1]["x"], 3.0);
  EXPECT_FLOAT_EQ(json["points"][2]["x"], 5.0);
}

TEST(ConverterTest, JsonToArrayOfNestedMessages)
{
  Converter converter;
  nlohmann::json json;
  json["points"] = nlohmann::json::array();
  json["points"].push_back({{"x", 10.0}, {"y", 20.0}, {"z", 0.0}});
  json["points"].push_back({{"x", 30.0}, {"y", 40.0}, {"z", 0.0}});

  geometry_msgs::msg::Polygon msg;
  ASSERT_TRUE(converter.to_msg(json, msg));
  ASSERT_EQ(msg.points.size(), 2);
  EXPECT_FLOAT_EQ(msg.points[0].x, 10.0);
  EXPECT_FLOAT_EQ(msg.points[0].y, 20.0);
  EXPECT_FLOAT_EQ(msg.points[1].x, 30.0);
  EXPECT_FLOAT_EQ(msg.points[1].y, 40.0);
}

TEST(ConverterTest, JsonWithMissingFieldsUsesDefaults)
{
  Converter converter;
  nlohmann::json json;
  json["x"] = 10.0;
  // y and z fields are missing

  geometry_msgs::msg::Point msg;
  msg.y = 99.0;  // Set non-default value
  msg.z = 88.0;  // Set non-default value

  ASSERT_TRUE(converter.to_msg(json, msg));
  EXPECT_DOUBLE_EQ(msg.x, 10.0);
  // Missing fields should keep their existing values
  EXPECT_DOUBLE_EQ(msg.y, 99.0);
  EXPECT_DOUBLE_EQ(msg.z, 88.0);
}

TEST(ConverterTest, JsonWithExtraFieldsAreIgnored)
{
  Converter converter;
  nlohmann::json json;
  json["x"] = 1.0;
  json["y"] = 2.0;
  json["z"] = 3.0;
  json["unknown_field"] = "should be ignored";
  json["extra_number"] = 123;

  geometry_msgs::msg::Point msg;
  ASSERT_TRUE(converter.to_msg(json, msg));
  EXPECT_DOUBLE_EQ(msg.x, 1.0);
  EXPECT_DOUBLE_EQ(msg.y, 2.0);
  EXPECT_DOUBLE_EQ(msg.z, 3.0);
  // Extra fields should be silently ignored without error
}

TEST(ConverterTest, ComplexRoundTrip)
{
  Converter converter;

  // Original message with nested structure and array
  geometry_msgs::msg::Polygon original;
  geometry_msgs::msg::Point32 p1, p2, p3;
  p1.x = 1.5;
  p1.y = 2.5;
  p1.z = 3.5;
  p2.x = 4.5;
  p2.y = 5.5;
  p2.z = 6.5;
  p3.x = 7.5;
  p3.y = 8.5;
  p3.z = 9.5;
  original.points = {p1, p2, p3};

  // Convert to JSON
  nlohmann::json json;
  ASSERT_TRUE(converter.to_json(original, json));

  // Convert back to message
  geometry_msgs::msg::Polygon result;
  ASSERT_TRUE(converter.to_msg(json, result));

  // Verify
  ASSERT_EQ(result.points.size(), original.points.size());
  for (size_t i = 0; i < result.points.size(); ++i) {
    EXPECT_FLOAT_EQ(result.points[i].x, original.points[i].x);
    EXPECT_FLOAT_EQ(result.points[i].y, original.points[i].y);
    EXPECT_FLOAT_EQ(result.points[i].z, original.points[i].z);
  }
}

TEST(ConverterTest, SerializedMessageString)
{
  Converter converter;

  // Create message
  std_msgs::msg::String msg;
  msg.data = "serialized message test";

  // Serialize
  rclcpp::Serialization<std_msgs::msg::String> serializer;
  rclcpp::SerializedMessage serialized_msg;
  serializer.serialize_message(&msg, &serialized_msg);

  // Convert SerializedMessage to JSON
  nlohmann::json json;
  ASSERT_TRUE(converter.to_json("std_msgs/msg/String", serialized_msg, json));
  EXPECT_EQ(json["data"], "serialized message test");

  // Convert JSON back to SerializedMessage
  rclcpp::SerializedMessage result_serialized;
  ASSERT_TRUE(converter.to_msg("std_msgs/msg/String", json, result_serialized));

  // Deserialize to verify
  std_msgs::msg::String result;
  serializer.deserialize_message(&result_serialized, &result);
  EXPECT_EQ(result.data, msg.data);
}

TEST(ConverterTest, SerializedMessagePose)
{
  Converter converter;

  // Create pose message
  geometry_msgs::msg::Pose pose;
  pose.position.x = 1.5;
  pose.position.y = 2.5;
  pose.position.z = 3.5;
  pose.orientation.x = 0.0;
  pose.orientation.y = 0.0;
  pose.orientation.z = 0.0;
  pose.orientation.w = 1.0;

  // Serialize
  rclcpp::Serialization<geometry_msgs::msg::Pose> serializer;
  rclcpp::SerializedMessage serialized_msg;
  serializer.serialize_message(&pose, &serialized_msg);

  // Convert SerializedMessage to JSON
  nlohmann::json json;
  ASSERT_TRUE(converter.to_json("geometry_msgs/msg/Pose", serialized_msg, json));
  EXPECT_FLOAT_EQ(json["position"]["x"], 1.5);
  EXPECT_FLOAT_EQ(json["position"]["y"], 2.5);
  EXPECT_FLOAT_EQ(json["position"]["z"], 3.5);
  EXPECT_FLOAT_EQ(json["orientation"]["w"], 1.0);

  // Convert JSON back to SerializedMessage
  rclcpp::SerializedMessage result_serialized;
  ASSERT_TRUE(converter.to_msg("geometry_msgs/msg/Pose", json, result_serialized));

  // Deserialize to verify
  geometry_msgs::msg::Pose result;
  serializer.deserialize_message(&result_serialized, &result);
  EXPECT_FLOAT_EQ(result.position.x, pose.position.x);
  EXPECT_FLOAT_EQ(result.position.y, pose.position.y);
  EXPECT_FLOAT_EQ(result.position.z, pose.position.z);
  EXPECT_FLOAT_EQ(result.orientation.w, pose.orientation.w);
}

TEST(ConverterTest, ServiceRequestBasicTypes)
{
  Converter converter;

  // Create service request
  test_msgs::srv::BasicTypes::Request request;
  request.bool_value = true;
  request.byte_value = 123;
  request.char_value = 'A';
  request.float32_value = 1.5f;
  request.float64_value = 2.5;
  request.int8_value = -8;
  request.uint8_value = 8;
  request.int16_value = -16;
  request.uint16_value = 16;
  request.int32_value = -32;
  request.uint32_value = 32;
  request.int64_value = -64;
  request.uint64_value = 64;
  request.string_value = "test string";

  // Serialize
  rclcpp::Serialization<test_msgs::srv::BasicTypes::Request> serializer;
  rclcpp::SerializedMessage serialized_msg;
  serializer.serialize_message(&request, &serialized_msg);

  // Convert SerializedMessage to JSON
  nlohmann::json json;
  ASSERT_TRUE(converter.to_json("test_msgs/srv/BasicTypes::Request", serialized_msg, json));
  EXPECT_EQ(json["bool_value"], true);
  EXPECT_EQ(json["byte_value"], 123);
  EXPECT_EQ(json["char_value"], 'A');
  EXPECT_FLOAT_EQ(json["float32_value"], 1.5f);
  EXPECT_DOUBLE_EQ(json["float64_value"], 2.5);
  EXPECT_EQ(json["int8_value"], -8);
  EXPECT_EQ(json["uint8_value"], 8);
  EXPECT_EQ(json["int16_value"], -16);
  EXPECT_EQ(json["uint16_value"], 16);
  EXPECT_EQ(json["int32_value"], -32);
  EXPECT_EQ(json["uint32_value"], 32);
  EXPECT_EQ(json["int64_value"], -64);
  EXPECT_EQ(json["uint64_value"], 64);
  EXPECT_EQ(json["string_value"], "test string");

  // Convert JSON back to SerializedMessage
  rclcpp::SerializedMessage result_serialized;
  ASSERT_TRUE(converter.to_msg("test_msgs/srv/BasicTypes::Request", json, result_serialized));

  // Deserialize to verify
  test_msgs::srv::BasicTypes::Request result;
  serializer.deserialize_message(&result_serialized, &result);
  EXPECT_EQ(result.bool_value, request.bool_value);
  EXPECT_EQ(result.byte_value, request.byte_value);
  EXPECT_EQ(result.char_value, request.char_value);
  EXPECT_FLOAT_EQ(result.float32_value, request.float32_value);
  EXPECT_DOUBLE_EQ(result.float64_value, request.float64_value);
  EXPECT_EQ(result.string_value, request.string_value);
}

TEST(ConverterTest, ServiceResponseBasicTypes)
{
  Converter converter;

  // Create service response
  test_msgs::srv::BasicTypes::Response response;
  response.bool_value = false;
  response.byte_value = 255;
  response.char_value = 'Z';
  response.float32_value = 3.14f;
  response.float64_value = 2.718;
  response.int8_value = -100;
  response.uint8_value = 200;
  response.int16_value = -1000;
  response.uint16_value = 2000;
  response.int32_value = -100000;
  response.uint32_value = 200000;
  response.int64_value = -9876543210;
  response.uint64_value = 9876543210;
  response.string_value = "response data";

  // Serialize
  rclcpp::Serialization<test_msgs::srv::BasicTypes::Response> serializer;
  rclcpp::SerializedMessage serialized_msg;
  serializer.serialize_message(&response, &serialized_msg);

  // Convert SerializedMessage to JSON
  nlohmann::json json;
  ASSERT_TRUE(converter.to_json("test_msgs/srv/BasicTypes::Response", serialized_msg, json));
  EXPECT_EQ(json["bool_value"], false);
  EXPECT_EQ(json["byte_value"], 255);
  EXPECT_EQ(json["char_value"], 'Z');
  EXPECT_FLOAT_EQ(json["float32_value"], 3.14f);
  EXPECT_DOUBLE_EQ(json["float64_value"], 2.718);
  EXPECT_EQ(json["string_value"], "response data");

  // Convert JSON back to SerializedMessage
  rclcpp::SerializedMessage result_serialized;
  ASSERT_TRUE(converter.to_msg("test_msgs/srv/BasicTypes::Response", json, result_serialized));

  // Deserialize to verify
  test_msgs::srv::BasicTypes::Response result;
  serializer.deserialize_message(&result_serialized, &result);
  EXPECT_EQ(result.bool_value, response.bool_value);
  EXPECT_EQ(result.byte_value, response.byte_value);
  EXPECT_EQ(result.char_value, response.char_value);
  EXPECT_FLOAT_EQ(result.float32_value, response.float32_value);
  EXPECT_DOUBLE_EQ(result.float64_value, response.float64_value);
  EXPECT_EQ(result.string_value, response.string_value);
}

TEST(ConverterTest, ServiceRequestEmpty)
{
  Converter converter;

  // Create empty service request
  test_msgs::srv::Empty::Request request;

  // Serialize
  rclcpp::Serialization<test_msgs::srv::Empty::Request> serializer;
  rclcpp::SerializedMessage serialized_msg;
  serializer.serialize_message(&request, &serialized_msg);

  // Convert SerializedMessage to JSON
  nlohmann::json json;
  ASSERT_TRUE(converter.to_json("test_msgs/srv/Empty::Request", serialized_msg, json));
  // Empty message should produce empty JSON object
  EXPECT_TRUE(json.is_object());

  // Convert JSON back to SerializedMessage
  rclcpp::SerializedMessage result_serialized;
  ASSERT_TRUE(converter.to_msg("test_msgs/srv/Empty::Request", json, result_serialized));

  // Deserialize to verify (should not throw)
  test_msgs::srv::Empty::Request result;
  EXPECT_NO_THROW(serializer.deserialize_message(&result_serialized, &result));
}

TEST(ConverterTest, ServiceResponseEmpty)
{
  Converter converter;

  // Create empty service response
  test_msgs::srv::Empty::Response response;

  // Serialize
  rclcpp::Serialization<test_msgs::srv::Empty::Response> serializer;
  rclcpp::SerializedMessage serialized_msg;
  serializer.serialize_message(&response, &serialized_msg);

  // Convert SerializedMessage to JSON
  nlohmann::json json;
  ASSERT_TRUE(converter.to_json("test_msgs/srv/Empty::Response", serialized_msg, json));
  // Empty message should produce empty JSON object
  EXPECT_TRUE(json.is_object());

  // Convert JSON back to SerializedMessage
  rclcpp::SerializedMessage result_serialized;
  ASSERT_TRUE(converter.to_msg("test_msgs/srv/Empty::Response", json, result_serialized));

  // Deserialize to verify (should not throw)
  test_msgs::srv::Empty::Response result;
  EXPECT_NO_THROW(serializer.deserialize_message(&result_serialized, &result));
}

TEST(ConverterTest, CacheTest)
{
  Converter converter;

  // First call - loads type info via dlopen
  std_msgs::msg::String msg1;
  msg1.data = "first call";
  rclcpp::Serialization<std_msgs::msg::String> serializer;
  rclcpp::SerializedMessage serialized_msg1;
  serializer.serialize_message(&msg1, &serialized_msg1);

  nlohmann::json json1;
  ASSERT_TRUE(converter.to_json("std_msgs/msg/String", serialized_msg1, json1));
  EXPECT_EQ(json1["data"], "first call");

  // Second call - should use cached type info (no dlopen)
  std_msgs::msg::String msg2;
  msg2.data = "second call";
  rclcpp::SerializedMessage serialized_msg2;
  serializer.serialize_message(&msg2, &serialized_msg2);

  nlohmann::json json2;
  ASSERT_TRUE(converter.to_json("std_msgs/msg/String", serialized_msg2, json2));
  EXPECT_EQ(json2["data"], "second call");

  // Third call - different data, same type (cache hit)
  std_msgs::msg::String msg3;
  msg3.data = "third call";
  rclcpp::SerializedMessage serialized_msg3;
  serializer.serialize_message(&msg3, &serialized_msg3);

  nlohmann::json json3;
  ASSERT_TRUE(converter.to_json("std_msgs/msg/String", serialized_msg3, json3));
  EXPECT_EQ(json3["data"], "third call");
}

TEST(ConverterTest, TestMsgsBasicTypes)
{
  Converter converter;

  test_msgs::msg::BasicTypes msg;
  msg.bool_value = true;
  msg.byte_value = 123;
  msg.char_value = 'A';
  msg.float32_value = 3.14f;
  msg.float64_value = 2.718;
  msg.int8_value = -42;
  msg.uint8_value = 200;
  msg.int16_value = -1000;
  msg.uint16_value = 50000;
  msg.int32_value = -123456;
  msg.uint32_value = 987654;
  msg.int64_value = -9876543210;
  msg.uint64_value = 1234567890123;

  nlohmann::json json;
  ASSERT_TRUE(converter.to_json(msg, json));

  EXPECT_EQ(json["bool_value"], true);
  EXPECT_EQ(json["byte_value"], 123);
  EXPECT_EQ(json["char_value"], 'A');
  EXPECT_FLOAT_EQ(json["float32_value"], 3.14f);
  EXPECT_DOUBLE_EQ(json["float64_value"], 2.718);
  EXPECT_EQ(json["int8_value"], -42);
  EXPECT_EQ(json["uint8_value"], 200);
  EXPECT_EQ(json["int16_value"], -1000);
  EXPECT_EQ(json["uint16_value"], 50000);
  EXPECT_EQ(json["int32_value"], -123456);
  EXPECT_EQ(json["uint32_value"], 987654);
  EXPECT_EQ(json["int64_value"], -9876543210);
  EXPECT_EQ(json["uint64_value"], 1234567890123);

  // Convert back
  test_msgs::msg::BasicTypes result;
  ASSERT_TRUE(converter.to_msg(json, result));

  EXPECT_EQ(result.bool_value, msg.bool_value);
  EXPECT_EQ(result.byte_value, msg.byte_value);
  EXPECT_EQ(result.char_value, msg.char_value);
  EXPECT_FLOAT_EQ(result.float32_value, msg.float32_value);
  EXPECT_DOUBLE_EQ(result.float64_value, msg.float64_value);
  EXPECT_EQ(result.int8_value, msg.int8_value);
  EXPECT_EQ(result.uint8_value, msg.uint8_value);
}

TEST(ConverterTest, TestMsgsArrays)
{
  Converter converter;

  test_msgs::msg::Arrays msg;
  msg.bool_values = {true, false, true};
  msg.byte_values = {0, 127, 255};
  msg.char_values = {65, 66, 67};  // A, B, C
  msg.float32_values = {1.1f, 2.2f, 3.3f};
  msg.float64_values = {1.111, 2.222, 3.333};
  msg.int8_values = {-1, 0, 1};
  msg.uint8_values = {0, 100, 255};
  msg.int16_values = {-100, 0, 100};
  msg.uint16_values = {0, 1000, 50000};
  msg.int32_values = {-1000000, 0, 1000000};
  msg.uint32_values = {0, 500000, 4000000};
  msg.int64_values = {-9999999999, 0, 9999999999};
  msg.uint64_values = {0, 123456789, 987654321};
  msg.string_values = {"hello", "world", "test"};

  nlohmann::json json;
  ASSERT_TRUE(converter.to_json(msg, json));

  ASSERT_TRUE(json["bool_values"].is_array());
  EXPECT_EQ(json["bool_values"].size(), 3);
  EXPECT_EQ(json["bool_values"][0], true);
  EXPECT_EQ(json["bool_values"][1], false);
  EXPECT_EQ(json["bool_values"][2], true);

  ASSERT_TRUE(json["byte_values"].is_array());
  EXPECT_EQ(json["byte_values"][0], 0);
  EXPECT_EQ(json["byte_values"][1], 127);
  EXPECT_EQ(json["byte_values"][2], 255);

  ASSERT_TRUE(json["string_values"].is_array());
  EXPECT_EQ(json["string_values"][0], "hello");
  EXPECT_EQ(json["string_values"][1], "world");
  EXPECT_EQ(json["string_values"][2], "test");

  // Convert back
  test_msgs::msg::Arrays result;
  ASSERT_TRUE(converter.to_msg(json, result));

  ASSERT_EQ(result.bool_values.size(), 3);
  EXPECT_EQ(result.bool_values[0], true);
  EXPECT_EQ(result.bool_values[1], false);

  ASSERT_EQ(result.string_values.size(), 3);
  EXPECT_EQ(result.string_values[0], "hello");
  EXPECT_EQ(result.string_values[2], "test");
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
