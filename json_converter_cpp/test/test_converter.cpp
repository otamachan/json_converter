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

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
