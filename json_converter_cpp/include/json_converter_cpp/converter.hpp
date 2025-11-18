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

#include <rapidjson/allocators.h>
#include <rapidjson/document.h>

#include <memory>
#include <string>
#include <unordered_map>

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
  Converter(const Converter &) = delete;
  Converter(Converter &&) = delete;
  Converter & operator=(const Converter &) = delete;
  Converter & operator=(Converter &&) = delete;
  virtual ~Converter() = default;

  /**
   * @brief Convert a ROS2 message to JSON
   * @tparam T ROS2 message type
   * @param msg ROS2 message
   * @param json Output JSON value (will be set as object/array/primitive)
   * @param allocator RapidJSON allocator for memory management
   * @return true if conversion succeeded, false otherwise
   */
  template<typename T>
  bool to_json(
    const T & msg, rapidjson::Value & json,
    rapidjson::Document::AllocatorType & allocator);

  /**
   * @brief Convert JSON to a ROS2 message
   * @tparam T ROS2 message type
   * @param json Input JSON value (object/array/primitive)
   * @param msg Output ROS2 message
   * @return true if conversion succeeded, false otherwise
   */
  template<typename T>
  bool to_msg(const rapidjson::Value & json, T & msg);

  /**
   * @brief Convert a SerializedMessage to JSON using dynamic type loading
   * @param type_name Full type name (e.g., "std_msgs/msg/String")
   * @param serialized_msg Serialized ROS2 message
   * @param json Output JSON value (will be set as object)
   * @param allocator RapidJSON allocator for memory management
   * @return true if conversion succeeded, false otherwise
   */
  bool to_json(
    const std::string & type_name, const rclcpp::SerializedMessage & serialized_msg,
    rapidjson::Value & json, rapidjson::Document::AllocatorType & allocator);

  /**
   * @brief Convert JSON to a SerializedMessage using dynamic type loading
   * @param type_name Full type name (e.g., "std_msgs/msg/String")
   * @param json Input JSON value (must be object)
   * @param serialized_msg Output serialized ROS2 message
   * @return true if conversion succeeded, false otherwise
   */
  bool to_msg(
    const std::string & type_name, const rapidjson::Value & json,
    rclcpp::SerializedMessage & serialized_msg);

  /**
   * @brief Convert JSON to a message object (for GenericClient)
   * @param type_name Full type name (e.g., "std_msgs/msg/String")
   * @param json Input JSON value (must be object)
   * @param message_ptr Shared pointer to message object (allocated and set by this function)
   * @return true if conversion succeeded, false otherwise
   */
  bool to_msg(
    const std::string & type_name, const rapidjson::Value & json,
    std::shared_ptr<void> & message_ptr);

  /**
   * @brief Convert a message object to JSON (for GenericClient)
   * @param type_name Full type name (e.g., "std_msgs/msg/String")
   * @param message_ptr Pointer to message object
   * @param json Output JSON value (will be set as object)
   * @param allocator RapidJSON allocator for memory management
   * @return true if conversion succeeded, false otherwise
   */
  bool to_json(
    const std::string & type_name, const void * message_ptr,
    rapidjson::Value & json, rapidjson::Document::AllocatorType & allocator);

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

  static bool primitive_to_json(
    const void * data_ptr, uint8_t type_id, rapidjson::Value & json,
    rapidjson::Document::AllocatorType & allocator);

  static bool message_to_json(
    const void * message_ptr, const MessageMembers * members, rapidjson::Value & json,
    rapidjson::Document::AllocatorType & allocator);

  static bool json_to_primitive(
    const rapidjson::Value & json, void * data_ptr, uint8_t type_id);

  static bool json_to_message(
    const rapidjson::Value & json, void * message_ptr, const MessageMembers * members);

  // Cache for type information
  std::unordered_map<std::string, TypeInfo> type_info_cache_;
};

template<typename T>
bool Converter::to_json(
  const T & msg, rapidjson::Value & json,
  rapidjson::Document::AllocatorType & allocator)
{
  const rosidl_message_type_support_t * type_support =
    rosidl_typesupport_cpp::get_message_type_support_handle<T>();

  if (!type_support) {
    return false;
  }

  const auto * introspection_ts = get_message_typesupport_handle(
    type_support, rosidl_typesupport_introspection_cpp::typesupport_identifier);

  if (!introspection_ts) {
    return false;
  }

  const auto * members = static_cast<const MessageMembers *>(introspection_ts->data);

  return message_to_json(&msg, members, json, allocator);
}

template<typename T>
bool Converter::to_msg(const rapidjson::Value & json, T & msg)
{
  const rosidl_message_type_support_t * type_support =
    rosidl_typesupport_cpp::get_message_type_support_handle<T>();

  if (!type_support) {
    return false;
  }

  const auto * introspection_ts = get_message_typesupport_handle(
    type_support, rosidl_typesupport_introspection_cpp::typesupport_identifier);

  if (!introspection_ts) {
    return false;
  }

  const auto * members = static_cast<const MessageMembers *>(introspection_ts->data);

  return json_to_message(json, &msg, members);
}

}  // namespace json_converter_cpp
