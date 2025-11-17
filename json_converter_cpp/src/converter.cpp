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

namespace json_converter_cpp
{

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
bool Converter::primitive_to_json(const void * data_ptr, uint8_t type_id, nlohmann::json & json)
{
  switch (type_id) {
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL:
      json = *reinterpret_cast<const bool *>(data_ptr);
      break;
    case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
      json = *reinterpret_cast<const uint8_t *>(data_ptr);
      break;
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
  if (!message_ptr || !members) {
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
          const MessageMembers * nested_members =
            static_cast<const MessageMembers *>(member.members_->data);
          if (!message_to_json(element_ptr, nested_members, element_json)) {
            return false;
          }
        } else {
          if (!primitive_to_json(element_ptr, member.type_id_, element_json)) {
            return false;
          }
        }
        array_json.push_back(element_json);
      }

      json[member.name_] = array_json;
    } else {
      if (member.type_id_ == rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE) {
        nlohmann::json nested_json;
        const MessageMembers * nested_members =
          static_cast<const MessageMembers *>(member.members_->data);
        if (!message_to_json(member_ptr, nested_members, nested_json)) {
          return false;
        }
        json[member.name_] = nested_json;
      } else {
        nlohmann::json field_json;
        if (!primitive_to_json(member_ptr, member.type_id_, field_json)) {
          return false;
        }
        json[member.name_] = field_json;
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
      case rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
        *reinterpret_cast<uint8_t *>(data_ptr) = json.get<uint8_t>();
        break;
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
  if (!message_ptr || !members) {
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
          const MessageMembers * nested_members =
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
        const MessageMembers * nested_members =
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

}  // namespace json_converter_cpp
