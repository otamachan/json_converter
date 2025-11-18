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

#pragma once

#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>
#include <rclcpp/serialized_message.hpp>
#include <rosidl_typesupport_cpp/message_type_support.hpp>
#include <rosidl_typesupport_introspection_cpp/field_types.hpp>
#include <rosidl_typesupport_introspection_cpp/identifier.hpp>
#include <rosidl_typesupport_introspection_cpp/message_introspection.hpp>

namespace json_converter_cpp
{

class Converter
{
public:
  Converter() = default;
  virtual ~Converter() = default;

  /**
   * @brief Convert a ROS2 message to JSON
   * @tparam T ROS2 message type
   * @param msg ROS2 message
   * @param json Output JSON object
   * @return true if conversion succeeded, false otherwise
   */
  template<typename T>
  bool to_json(const T & msg, nlohmann::json & json);

  /**
   * @brief Convert JSON to a ROS2 message
   * @tparam T ROS2 message type
   * @param json Input JSON object
   * @param msg Output ROS2 message
   * @return true if conversion succeeded, false otherwise
   */
  template<typename T>
  bool to_msg(const nlohmann::json & json, T & msg);

  /**
   * @brief Convert a SerializedMessage to JSON using dynamic type loading
   * @param type_name Full type name (e.g., "std_msgs/msg/String")
   * @param serialized_msg Serialized ROS2 message
   * @param json Output JSON object
   * @return true if conversion succeeded, false otherwise
   */
  bool to_json(
    const std::string & type_name, const rclcpp::SerializedMessage & serialized_msg,
    nlohmann::json & json);

  /**
   * @brief Convert JSON to a SerializedMessage using dynamic type loading
   * @param type_name Full type name (e.g., "std_msgs/msg/String")
   * @param json Input JSON object
   * @param serialized_msg Output serialized ROS2 message
   * @return true if conversion succeeded, false otherwise
   */
  bool to_msg(
    const std::string & type_name, const nlohmann::json & json,
    rclcpp::SerializedMessage & serialized_msg);

private:
  using MessageMembers = rosidl_typesupport_introspection_cpp::MessageMembers;
  using MessageMember = rosidl_typesupport_introspection_cpp::MessageMember;

  struct TypeInfo
  {
    const rosidl_message_type_support_t * type_support;
    const MessageMembers * members;
  };

  TypeInfo load_type_info(const std::string & type_name);
  static const rosidl_message_type_support_t * load_introspection_type_support(
    const std::string & package_name, const std::string & type_name_with_prefix);
  static const rosidl_message_type_support_t * load_type_support_cpp(
    const std::string & package_name, const std::string & type_name_with_prefix);

  static bool primitive_to_json(const void * data_ptr, uint8_t type_id, nlohmann::json & json);

  static bool message_to_json(
    const void * message_ptr, const MessageMembers * members, nlohmann::json & json);

  static bool json_to_primitive(const nlohmann::json & json, void * data_ptr, uint8_t type_id);

  static bool json_to_message(
    const nlohmann::json & json, void * message_ptr, const MessageMembers * members);

  // Cache for type information
  std::unordered_map<std::string, TypeInfo> type_info_cache_;
};

template<typename T>
bool Converter::to_json(const T & msg, nlohmann::json & json)
{
  const rosidl_message_type_support_t * type_support =
    rosidl_typesupport_cpp::get_message_type_support_handle<T>();

  if (!type_support) {
    return false;
  }

  auto introspection_ts = get_message_typesupport_handle(
    type_support, rosidl_typesupport_introspection_cpp::typesupport_identifier);

  if (!introspection_ts) {
    return false;
  }

  const MessageMembers * members = static_cast<const MessageMembers *>(introspection_ts->data);

  return message_to_json(&msg, members, json);
}

template<typename T>
bool Converter::to_msg(const nlohmann::json & json, T & msg)
{
  const rosidl_message_type_support_t * type_support =
    rosidl_typesupport_cpp::get_message_type_support_handle<T>();

  if (!type_support) {
    return false;
  }

  auto introspection_ts = get_message_typesupport_handle(
    type_support, rosidl_typesupport_introspection_cpp::typesupport_identifier);

  if (!introspection_ts) {
    return false;
  }

  const MessageMembers * members = static_cast<const MessageMembers *>(introspection_ts->data);

  return json_to_message(json, &msg, members);
}

}  // namespace json_converter_cpp
