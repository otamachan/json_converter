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

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>

#include <gtest/gtest.h>

#include <geometry_msgs/msg/point.hpp>
#include <geometry_msgs/msg/polygon.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <rclcpp/serialization.hpp>
#include <std_msgs/msg/int32.hpp>
#include <std_msgs/msg/int32_multi_array.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_srvs/srv/trigger.hpp>
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

  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  ASSERT_TRUE(converter.to_json(msg, doc, allocator));
  EXPECT_EQ(std::string(doc["data"].GetString()), "hello world");
}

TEST(ConverterTest, JsonToStringMessage)
{
  Converter converter;
  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  doc.AddMember("data", "test message", allocator);

  std_msgs::msg::String msg;
  ASSERT_TRUE(converter.to_msg(doc, msg));
  EXPECT_EQ(msg.data, "test message");
}

TEST(ConverterTest, Int32ArrayToJson)
{
  Converter converter;
  std_msgs::msg::Int32MultiArray msg;
  msg.data = {1, 2, 3, 4, 5};

  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  ASSERT_TRUE(converter.to_json(msg, doc, allocator));
  ASSERT_TRUE(doc["data"].IsArray());
  EXPECT_EQ(doc["data"].Size(), 5);
  EXPECT_EQ(doc["data"][0].GetInt(), 1);
  EXPECT_EQ(doc["data"][4].GetInt(), 5);
}

TEST(ConverterTest, JsonToInt32Array)
{
  Converter converter;
  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  rapidjson::Value array(rapidjson::kArrayType);
  array.PushBack(10, allocator);
  array.PushBack(20, allocator);
  array.PushBack(30, allocator);
  doc.AddMember("data", array, allocator);

  std_msgs::msg::Int32MultiArray msg;
  ASSERT_TRUE(converter.to_msg(doc, msg));
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

  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  ASSERT_TRUE(converter.to_json(msg, doc, allocator));
  EXPECT_DOUBLE_EQ(doc["position"]["x"].GetDouble(), 1.0);
  EXPECT_DOUBLE_EQ(doc["position"]["y"].GetDouble(), 2.0);
  EXPECT_DOUBLE_EQ(doc["position"]["z"].GetDouble(), 3.0);
  EXPECT_DOUBLE_EQ(doc["orientation"]["w"].GetDouble(), 1.0);
}

TEST(ConverterTest, JsonToNestedMessage)
{
  Converter converter;
  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();

  rapidjson::Value position(rapidjson::kObjectType);
  position.AddMember("x", 5.0, allocator);
  position.AddMember("y", 6.0, allocator);
  position.AddMember("z", 7.0, allocator);
  doc.AddMember("position", position, allocator);

  rapidjson::Value orientation(rapidjson::kObjectType);
  orientation.AddMember("x", 0.0, allocator);
  orientation.AddMember("y", 0.0, allocator);
  orientation.AddMember("z", 0.0, allocator);
  orientation.AddMember("w", 1.0, allocator);
  doc.AddMember("orientation", orientation, allocator);

  geometry_msgs::msg::Pose msg;
  ASSERT_TRUE(converter.to_msg(doc, msg));
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

  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  ASSERT_TRUE(converter.to_json(msg, doc, allocator));
  ASSERT_TRUE(doc["points"].IsArray());
  EXPECT_EQ(doc["points"].Size(), 3);
  EXPECT_FLOAT_EQ(doc["points"][0]["x"].GetFloat(), 1.0);
  EXPECT_FLOAT_EQ(doc["points"][1]["x"].GetFloat(), 3.0);
  EXPECT_FLOAT_EQ(doc["points"][2]["x"].GetFloat(), 5.0);
}

TEST(ConverterTest, JsonToArrayOfNestedMessages)
{
  Converter converter;
  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();

  rapidjson::Value points(rapidjson::kArrayType);

  rapidjson::Value p1(rapidjson::kObjectType);
  p1.AddMember("x", 10.0, allocator);
  p1.AddMember("y", 20.0, allocator);
  p1.AddMember("z", 0.0, allocator);
  points.PushBack(p1, allocator);

  rapidjson::Value p2(rapidjson::kObjectType);
  p2.AddMember("x", 30.0, allocator);
  p2.AddMember("y", 40.0, allocator);
  p2.AddMember("z", 0.0, allocator);
  points.PushBack(p2, allocator);

  doc.AddMember("points", points, allocator);

  geometry_msgs::msg::Polygon msg;
  ASSERT_TRUE(converter.to_msg(doc, msg));
  ASSERT_EQ(msg.points.size(), 2);
  EXPECT_FLOAT_EQ(msg.points[0].x, 10.0);
  EXPECT_FLOAT_EQ(msg.points[0].y, 20.0);
  EXPECT_FLOAT_EQ(msg.points[1].x, 30.0);
  EXPECT_FLOAT_EQ(msg.points[1].y, 40.0);
}

TEST(ConverterTest, JsonWithMissingFieldsUsesDefaults)
{
  Converter converter;
  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  doc.AddMember("x", 10.0, allocator);
  // y and z fields are missing

  geometry_msgs::msg::Point msg;
  msg.y = 99.0;  // Set non-default value
  msg.z = 88.0;  // Set non-default value

  ASSERT_TRUE(converter.to_msg(doc, msg));
  EXPECT_DOUBLE_EQ(msg.x, 10.0);
  // Missing fields should keep their existing values
  EXPECT_DOUBLE_EQ(msg.y, 99.0);
  EXPECT_DOUBLE_EQ(msg.z, 88.0);
}

TEST(ConverterTest, JsonWithExtraFieldsAreIgnored)
{
  Converter converter;
  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  doc.AddMember("x", 1.0, allocator);
  doc.AddMember("y", 2.0, allocator);
  doc.AddMember("z", 3.0, allocator);
  doc.AddMember("unknown_field", "should be ignored", allocator);
  doc.AddMember("extra_number", 123, allocator);

  geometry_msgs::msg::Point msg;
  ASSERT_TRUE(converter.to_msg(doc, msg));
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
  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  ASSERT_TRUE(converter.to_json(original, doc, allocator));

  // Convert back to message
  geometry_msgs::msg::Polygon result;
  ASSERT_TRUE(converter.to_msg(doc, result));

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
  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  ASSERT_TRUE(converter.to_json("std_msgs/msg/String", serialized_msg, doc, allocator));
  EXPECT_EQ(std::string(doc["data"].GetString()), "serialized message test");

  // Convert JSON back to SerializedMessage
  rclcpp::SerializedMessage result_serialized;
  ASSERT_TRUE(converter.to_msg("std_msgs/msg/String", doc, result_serialized));

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
  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  ASSERT_TRUE(converter.to_json("geometry_msgs/msg/Pose", serialized_msg, doc, allocator));
  EXPECT_FLOAT_EQ(doc["position"]["x"].GetFloat(), 1.5);
  EXPECT_FLOAT_EQ(doc["position"]["y"].GetFloat(), 2.5);
  EXPECT_FLOAT_EQ(doc["position"]["z"].GetFloat(), 3.5);
  EXPECT_FLOAT_EQ(doc["orientation"]["w"].GetFloat(), 1.0);

  // Convert JSON back to SerializedMessage
  rclcpp::SerializedMessage result_serialized;
  ASSERT_TRUE(converter.to_msg("geometry_msgs/msg/Pose", doc, result_serialized));

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
  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  ASSERT_TRUE(converter.to_json("test_msgs/srv/BasicTypes::Request", serialized_msg, doc,
    allocator));
  EXPECT_EQ(doc["bool_value"].GetBool(), true);
  EXPECT_EQ(doc["byte_value"].GetInt(), 123);
  EXPECT_EQ(doc["char_value"].GetInt(), 'A');
  EXPECT_FLOAT_EQ(doc["float32_value"].GetFloat(), 1.5f);
  EXPECT_DOUBLE_EQ(doc["float64_value"].GetDouble(), 2.5);
  EXPECT_EQ(doc["int8_value"].GetInt(), -8);
  EXPECT_EQ(doc["uint8_value"].GetInt(), 8);
  EXPECT_EQ(doc["int16_value"].GetInt(), -16);
  EXPECT_EQ(doc["uint16_value"].GetInt(), 16);
  EXPECT_EQ(doc["int32_value"].GetInt(), -32);
  EXPECT_EQ(doc["uint32_value"].GetUint(), 32);
  EXPECT_EQ(doc["int64_value"].GetInt64(), -64);
  EXPECT_EQ(doc["uint64_value"].GetUint64(), 64);
  EXPECT_EQ(std::string(doc["string_value"].GetString()), "test string");

  // Convert JSON back to SerializedMessage
  rclcpp::SerializedMessage result_serialized;
  ASSERT_TRUE(converter.to_msg("test_msgs/srv/BasicTypes::Request", doc, result_serialized));

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
  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  ASSERT_TRUE(converter.to_json("test_msgs/srv/BasicTypes::Response", serialized_msg, doc,
    allocator));
  EXPECT_EQ(doc["bool_value"].GetBool(), false);
  EXPECT_EQ(doc["byte_value"].GetInt(), 255);
  EXPECT_EQ(doc["char_value"].GetInt(), 'Z');
  EXPECT_FLOAT_EQ(doc["float32_value"].GetFloat(), 3.14f);
  EXPECT_DOUBLE_EQ(doc["float64_value"].GetDouble(), 2.718);
  EXPECT_EQ(std::string(doc["string_value"].GetString()), "response data");

  // Convert JSON back to SerializedMessage
  rclcpp::SerializedMessage result_serialized;
  ASSERT_TRUE(converter.to_msg("test_msgs/srv/BasicTypes::Response", doc, result_serialized));

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
  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  ASSERT_TRUE(converter.to_json("test_msgs/srv/Empty::Request", serialized_msg, doc, allocator));
  // Empty message should produce empty JSON object
  EXPECT_TRUE(doc.IsObject());

  // Convert JSON back to SerializedMessage
  rclcpp::SerializedMessage result_serialized;
  ASSERT_TRUE(converter.to_msg("test_msgs/srv/Empty::Request", doc, result_serialized));

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
  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  ASSERT_TRUE(converter.to_json("test_msgs/srv/Empty::Response", serialized_msg, doc, allocator));
  // Empty message should produce empty JSON object
  EXPECT_TRUE(doc.IsObject());

  // Convert JSON back to SerializedMessage
  rclcpp::SerializedMessage result_serialized;
  ASSERT_TRUE(converter.to_msg("test_msgs/srv/Empty::Response", doc, result_serialized));

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

  rapidjson::Document doc1;
  doc1.SetObject();
  auto & allocator1 = doc1.GetAllocator();
  ASSERT_TRUE(converter.to_json("std_msgs/msg/String", serialized_msg1, doc1, allocator1));
  EXPECT_EQ(std::string(doc1["data"].GetString()), "first call");

  // Second call - should use cached type info (no dlopen)
  std_msgs::msg::String msg2;
  msg2.data = "second call";
  rclcpp::SerializedMessage serialized_msg2;
  serializer.serialize_message(&msg2, &serialized_msg2);

  rapidjson::Document doc2;
  doc2.SetObject();
  auto & allocator2 = doc2.GetAllocator();
  ASSERT_TRUE(converter.to_json("std_msgs/msg/String", serialized_msg2, doc2, allocator2));
  EXPECT_EQ(std::string(doc2["data"].GetString()), "second call");

  // Third call - different data, same type (cache hit)
  std_msgs::msg::String msg3;
  msg3.data = "third call";
  rclcpp::SerializedMessage serialized_msg3;
  serializer.serialize_message(&msg3, &serialized_msg3);

  rapidjson::Document doc3;
  doc3.SetObject();
  auto & allocator3 = doc3.GetAllocator();
  ASSERT_TRUE(converter.to_json("std_msgs/msg/String", serialized_msg3, doc3, allocator3));
  EXPECT_EQ(std::string(doc3["data"].GetString()), "third call");
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

  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  ASSERT_TRUE(converter.to_json(msg, doc, allocator));

  EXPECT_EQ(doc["bool_value"].GetBool(), true);
  EXPECT_EQ(doc["byte_value"].GetInt(), 123);
  EXPECT_EQ(doc["char_value"].GetInt(), 'A');
  EXPECT_FLOAT_EQ(doc["float32_value"].GetFloat(), 3.14f);
  EXPECT_DOUBLE_EQ(doc["float64_value"].GetDouble(), 2.718);
  EXPECT_EQ(doc["int8_value"].GetInt(), -42);
  EXPECT_EQ(doc["uint8_value"].GetInt(), 200);
  EXPECT_EQ(doc["int16_value"].GetInt(), -1000);
  EXPECT_EQ(doc["uint16_value"].GetInt(), 50000);
  EXPECT_EQ(doc["int32_value"].GetInt(), -123456);
  EXPECT_EQ(doc["uint32_value"].GetUint(), 987654);
  EXPECT_EQ(doc["int64_value"].GetInt64(), -9876543210);
  EXPECT_EQ(doc["uint64_value"].GetUint64(), 1234567890123);

  // Convert back
  test_msgs::msg::BasicTypes result;
  ASSERT_TRUE(converter.to_msg(doc, result));

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

  rapidjson::Document doc;
  doc.SetObject();
  auto & allocator = doc.GetAllocator();
  ASSERT_TRUE(converter.to_json(msg, doc, allocator));

  ASSERT_TRUE(doc["bool_values"].IsArray());
  EXPECT_EQ(doc["bool_values"].Size(), 3);
  EXPECT_EQ(doc["bool_values"][0].GetBool(), true);
  EXPECT_EQ(doc["bool_values"][1].GetBool(), false);
  EXPECT_EQ(doc["bool_values"][2].GetBool(), true);

  ASSERT_TRUE(doc["byte_values"].IsArray());
  EXPECT_EQ(doc["byte_values"][0].GetInt(), 0);
  EXPECT_EQ(doc["byte_values"][1].GetInt(), 127);
  EXPECT_EQ(doc["byte_values"][2].GetInt(), 255);

  ASSERT_TRUE(doc["string_values"].IsArray());
  EXPECT_EQ(std::string(doc["string_values"][0].GetString()), "hello");
  EXPECT_EQ(std::string(doc["string_values"][1].GetString()), "world");
  EXPECT_EQ(std::string(doc["string_values"][2].GetString()), "test");

  // Convert back
  test_msgs::msg::Arrays result;
  ASSERT_TRUE(converter.to_msg(doc, result));

  ASSERT_EQ(result.bool_values.size(), 3);
  EXPECT_EQ(result.bool_values[0], true);
  EXPECT_EQ(result.bool_values[1], false);

  ASSERT_EQ(result.string_values.size(), 3);
  EXPECT_EQ(result.string_values[0], "hello");
  EXPECT_EQ(result.string_values[2], "test");
}

TEST(ConverterTest, TriggerServiceRequest)
{
  Converter converter;

  // Empty JSON to Trigger request (using void pointer API)
  // Trigger Request has no fields, but the structure still exists
  rapidjson::Document request_doc;
  request_doc.Parse("{}");

  std::shared_ptr<void> request_msg;
  ASSERT_TRUE(converter.to_msg("std_srvs/srv/Trigger::Request", request_doc, request_msg));
  EXPECT_NE(request_msg, nullptr);

  // Convert back to JSON
  rapidjson::Document result_doc;
  result_doc.SetObject();
  auto & allocator = result_doc.GetAllocator();
  ASSERT_TRUE(converter.to_json("std_srvs/srv/Trigger::Request", request_msg.get(),
    result_doc, allocator));

  // Trigger::Request is empty (structure_needs_at_least_one_member is skipped)
  EXPECT_TRUE(result_doc.IsObject());
  EXPECT_EQ(result_doc.MemberCount(), 0);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
