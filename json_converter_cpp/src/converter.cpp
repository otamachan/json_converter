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

#include "json_converter_cpp/converter.hpp"

#include <dlfcn.h>

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <utility>

#include <rclcpp/serialization.hpp>
#include <rosidl_runtime_cpp/message_initialization.hpp>

namespace json_converter_cpp
{

namespace
{

using GetMessageTypeSupportFunc = const rosidl_message_type_support_t * (*)();

}  // namespace

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
bool Converter::primitive_to_json(const void * data_ptr, uint8_t type_id, nlohmann::json & json)
{
  switch (type_id) {
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL:
      json = *reinterpret_cast<const bool *>(data_ptr);
      break;
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_OCTET:  // Also ROS_TYPE_BYTE
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
      json = *reinterpret_cast<const uint8_t *>(data_ptr);
      break;
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
      json = *reinterpret_cast<const int8_t *>(data_ptr);
      break;
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
      json = *reinterpret_cast<const uint16_t *>(data_ptr);
      break;
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
      json = *reinterpret_cast<const int16_t *>(data_ptr);
      break;
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
      json = *reinterpret_cast<const uint32_t *>(data_ptr);
      break;
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
      json = *reinterpret_cast<const int32_t *>(data_ptr);
      break;
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
      json = *reinterpret_cast<const uint64_t *>(data_ptr);
      break;
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
      json = *reinterpret_cast<const int64_t *>(data_ptr);
      break;
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT:
      json = *reinterpret_cast<const float *>(data_ptr);
      break;
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE:
      json = *reinterpret_cast<const double *>(data_ptr);
      break;
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
      json = *reinterpret_cast<const std::string *>(data_ptr);
      break;
    default:
      return false;
  }
  return true;
}
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

bool Converter::message_to_json(
  const void * message_ptr, const MessageMembers * members, nlohmann::json & json)
{
  if (message_ptr == nullptr || members == nullptr) {
    return false;
  }

  for (size_t i = 0; i < members->member_count_; ++i) {
    const MessageMember & member = members->members_[i];
    const uint8_t * member_ptr = static_cast<const uint8_t *>(message_ptr) + member.offset_;

    if (member.is_array_) {
      nlohmann::json array_json = nlohmann::json::array();
      size_t array_size = 0;

      if (member.array_size_ > 0 && !member.is_upper_bound_) {
        array_size = member.array_size_;
      } else {
        array_size = member.size_function(member_ptr);
      }

      for (size_t j = 0; j < array_size; ++j) {
        const void * element_ptr = member.get_const_function(member_ptr, j);
        nlohmann::json element_json;

        if (member.type_id_ == rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE) {
          auto const * nested_members =
            static_cast<const MessageMembers *>(member.members_->data);
          if (!message_to_json(element_ptr, nested_members, element_json)) {
            return false;
          }
        } else {
          if (!primitive_to_json(element_ptr, member.type_id_, element_json)) {
            return false;
          }
        }
        array_json.push_back(std::move(element_json));
      }

      json[member.name_] = std::move(array_json);
    } else {
      if (member.type_id_ == rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE) {
        nlohmann::json nested_json;
        auto const * nested_members =
          static_cast<const MessageMembers *>(member.members_->data);
        if (!message_to_json(member_ptr, nested_members, nested_json)) {
          return false;
        }
        json[member.name_] = std::move(nested_json);
      } else {
        nlohmann::json field_json;
        if (!primitive_to_json(member_ptr, member.type_id_, field_json)) {
          return false;
        }
        json[member.name_] = std::move(field_json);
      }
    }
  }

  return true;
}

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
bool Converter::json_to_primitive(const nlohmann::json & json, void * data_ptr, uint8_t type_id)
{
  try {
    switch (type_id) {
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL:
        *reinterpret_cast<bool *>(data_ptr) = json.get<bool>();
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_OCTET:  // Also ROS_TYPE_BYTE
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
        *reinterpret_cast<uint8_t *>(data_ptr) = json.get<uint8_t>();
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
        *reinterpret_cast<int8_t *>(data_ptr) = json.get<int8_t>();
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
        *reinterpret_cast<uint16_t *>(data_ptr) = json.get<uint16_t>();
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
        *reinterpret_cast<int16_t *>(data_ptr) = json.get<int16_t>();
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
        *reinterpret_cast<uint32_t *>(data_ptr) = json.get<uint32_t>();
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
        *reinterpret_cast<int32_t *>(data_ptr) = json.get<int32_t>();
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
        *reinterpret_cast<uint64_t *>(data_ptr) = json.get<uint64_t>();
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
        *reinterpret_cast<int64_t *>(data_ptr) = json.get<int64_t>();
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT:
        *reinterpret_cast<float *>(data_ptr) = json.get<float>();
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE:
        *reinterpret_cast<double *>(data_ptr) = json.get<double>();
        break;
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
        *reinterpret_cast<std::string *>(data_ptr) = json.get<std::string>();
        break;
      default:
        return false;
    }
  } catch (const nlohmann::json::exception &) {
    return false;
  }
  return true;
}
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

bool Converter::json_to_message(
  const nlohmann::json & json, void * message_ptr, const MessageMembers * members)
{
  if (message_ptr == nullptr || members == nullptr) {
    return false;
  }

  for (size_t i = 0; i < members->member_count_; ++i) {
    const MessageMember & member = members->members_[i];
    uint8_t * member_ptr = static_cast<uint8_t *>(message_ptr) + member.offset_;

    if (!json.contains(member.name_)) {
      continue;
    }

    const auto & json_value = json[member.name_];

    if (member.is_array_) {
      if (!json_value.is_array()) {
        return false;
      }

      size_t array_size = json_value.size();

      if (member.array_size_ > 0 && !member.is_upper_bound_) {
        if (array_size != member.array_size_) {
          return false;
        }
      } else {
        member.resize_function(member_ptr, array_size);
      }

      for (size_t j = 0; j < array_size; ++j) {
        void * element_ptr = member.get_function(member_ptr, j);

        if (member.type_id_ == rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE) {
          auto const * nested_members =
            static_cast<const MessageMembers *>(member.members_->data);
          if (!json_to_message(json_value[j], element_ptr, nested_members)) {
            return false;
          }
        } else {
          if (!json_to_primitive(json_value[j], element_ptr, member.type_id_)) {
            return false;
          }
        }
      }
    } else {
      if (member.type_id_ == rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE) {
        auto const * nested_members =
          static_cast<const MessageMembers *>(member.members_->data);
        if (!json_to_message(json_value, member_ptr, nested_members)) {
          return false;
        }
      } else {
        if (!json_to_primitive(json_value, member_ptr, member.type_id_)) {
          return false;
        }
      }
    }
  }

  return true;
}

const rosidl_message_type_support_t * Converter::load_introspection_type_support(
  const std::string & package_name, const std::string & type_name_with_prefix)
{
  std::stringstream lib_name;
  lib_name << "lib" << package_name << "__rosidl_typesupport_introspection_cpp.so";

  void * library = dlopen(lib_name.str().c_str(), RTLD_LAZY | RTLD_GLOBAL);
  if (library == nullptr) {
    return nullptr;
  }

  std::stringstream func_name;
  func_name << "rosidl_typesupport_introspection_cpp__get_message_type_support_handle__"
            << package_name << type_name_with_prefix;

  // reinterpret_cast needed for dlsym return value
  GetMessageTypeSupportFunc func =
    reinterpret_cast<GetMessageTypeSupportFunc>(dlsym(library, func_name.str().c_str()));
  if (func == nullptr) {
    return nullptr;
  }

  return func();
}

const rosidl_message_type_support_t * Converter::load_type_support_cpp(
  const std::string & package_name, const std::string & type_name_with_prefix)
{
  std::stringstream lib_name;
  lib_name << "lib" << package_name << "__rosidl_typesupport_cpp.so";

  void * library = dlopen(lib_name.str().c_str(), RTLD_LAZY | RTLD_GLOBAL);
  if (library == nullptr) {
    return nullptr;
  }

  std::stringstream func_name;
  func_name << "rosidl_typesupport_cpp__get_message_type_support_handle__"
            << package_name << type_name_with_prefix;

  // reinterpret_cast needed for dlsym return value
  GetMessageTypeSupportFunc func =
    reinterpret_cast<GetMessageTypeSupportFunc>(dlsym(library, func_name.str().c_str()));
  if (func == nullptr) {
    return nullptr;
  }

  return func();
}

Converter::TypeInfo Converter::load_type_info(const std::string & type_name)
{
  // Check cache first
  auto cache_it = type_info_cache_.find(type_name);
  if (cache_it != type_info_cache_.end()) {
    return cache_it->second;
  }

  // Parse "package_name/interface_type/message_type" or
  // "package_name/interface_type/ServiceType::Request" format
  size_t first_slash = type_name.find('/');
  size_t second_slash = type_name.find('/', first_slash + 1);

  if (first_slash == std::string::npos || second_slash == std::string::npos) {
    return {nullptr, nullptr};
  }

  std::string package_name = type_name.substr(0, first_slash);
  std::string interface_type =
    type_name.substr(first_slash + 1, second_slash - first_slash - 1);
  std::string message_type = type_name.substr(second_slash + 1);

  // Replace :: with _ for service Request/Response types
  size_t double_colon = message_type.find("::");
  if (double_colon != std::string::npos) {
    message_type = message_type.substr(0, double_colon) + "_" +
      message_type.substr(double_colon + 2);
  }

  std::string type_name_with_prefix = "__" + interface_type + "__" + message_type;

  const rosidl_message_type_support_t * type_support =
    load_type_support_cpp(package_name, type_name_with_prefix);
  const rosidl_message_type_support_t * introspection_type_support =
    load_introspection_type_support(package_name, type_name_with_prefix);

  if (type_support == nullptr || introspection_type_support == nullptr) {
    return {nullptr, nullptr};
  }

  auto const * members =
    static_cast<const MessageMembers *>(introspection_type_support->data);

  if (members == nullptr) {
    return {nullptr, nullptr};
  }

  TypeInfo type_info = {type_support, members};

  // Cache the result
  type_info_cache_[type_name] = type_info;

  return type_info;
}

// SerializedMessage API
bool Converter::to_json(
  const std::string & type_name,
  const rclcpp::SerializedMessage & serialized_msg,
  nlohmann::json & json)
{
  TypeInfo type_info = load_type_info(type_name);
  if (type_info.type_support == nullptr || type_info.members == nullptr) {
    return false;
  }

  // Allocate memory for the C++ object
  std::vector<uint8_t> object_data(type_info.members->size_of_);

  // Initialize the object
  if (type_info.members->init_function != nullptr) {
    type_info.members->init_function(
      object_data.data(), rosidl_runtime_cpp::MessageInitialization::ALL);
  }

  // Deserialize using rclcpp::SerializationBase
  rclcpp::SerializationBase serializer(type_info.type_support);
  serializer.deserialize_message(&serialized_msg, object_data.data());

  // Convert to JSON
  bool result = message_to_json(object_data.data(), type_info.members, json);

  // Cleanup
  if (type_info.members->fini_function != nullptr) {
    type_info.members->fini_function(object_data.data());
  }

  return result;
}

bool Converter::to_msg(
  const std::string & type_name,
  const nlohmann::json & json,
  rclcpp::SerializedMessage & serialized_msg)
{
  TypeInfo type_info = load_type_info(type_name);
  if (type_info.type_support == nullptr || type_info.members == nullptr) {
    return false;
  }

  // Allocate memory for the C++ object
  std::vector<uint8_t> object_data(type_info.members->size_of_);

  // Initialize the object
  if (type_info.members->init_function != nullptr) {
    type_info.members->init_function(
      object_data.data(), rosidl_runtime_cpp::MessageInitialization::ALL);
  }

  // Convert from JSON
  bool convert_result = json_to_message(json, object_data.data(), type_info.members);

  if (!convert_result) {
    // Cleanup on failure
    if (type_info.members->fini_function != nullptr) {
      type_info.members->fini_function(object_data.data());
    }
    return false;
  }

  // Serialize using rclcpp::SerializationBase
  rclcpp::SerializationBase serializer(type_info.type_support);
  serializer.serialize_message(object_data.data(), &serialized_msg);

  // Cleanup
  if (type_info.members->fini_function != nullptr) {
    type_info.members->fini_function(object_data.data());
  }

  return true;
}

bool Converter::to_msg(
  const std::string & type_name, const nlohmann::json & json,
  std::shared_ptr<void> & message_ptr)
{
  TypeInfo type_info = load_type_info(type_name);
  if (type_info.members == nullptr) {
    return false;
  }

  // Allocate message with custom deleter
  void * raw_ptr = std::malloc(type_info.members->size_of_);  // NOLINT
  if (raw_ptr == nullptr) {
    return false;
  }

  message_ptr = std::shared_ptr<void>(
    raw_ptr,
    [type_info](void * ptr) {
      if (type_info.members->fini_function != nullptr) {
        type_info.members->fini_function(ptr);
      }
      std::free(ptr);  // NOLINT
    });

  // Initialize message
  if (type_info.members->init_function != nullptr) {
    type_info.members->init_function(
      raw_ptr, rosidl_runtime_cpp::MessageInitialization::ZERO);
  } else {
    std::memset(raw_ptr, 0, type_info.members->size_of_);
  }

  // Convert JSON to message
  if (!json_to_message(json, raw_ptr, type_info.members)) {
    message_ptr.reset();
    return false;
  }

  return true;
}

bool Converter::to_json(
  const std::string & type_name, const void * message_ptr, nlohmann::json & json)
{
  TypeInfo type_info = load_type_info(type_name);
  if (type_info.members == nullptr) {
    return false;
  }

  return message_to_json(message_ptr, type_info.members, json);
}

}  // namespace json_converter_cpp
