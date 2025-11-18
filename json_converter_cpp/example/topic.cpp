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

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp/serialization.hpp>

#include "json_converter_cpp/converter.hpp"

class TopicEcho : public rclcpp::Node
{
public:
  TopicEcho(const std::string & topic_name, const std::string & type_name)
  : Node("topic_echo")
  {
    auto callback = [this, type_name](
      std::shared_ptr<rclcpp::SerializedMessage> msg) {  // NOLINT
        nlohmann::json json;
        if (converter_.to_json(type_name, *msg, json)) {
          std::cout << json.dump() << '\n';
        } else {
          std::cerr << "Failed to convert message to JSON\n";
        }
      };

    subscription_ = this->create_generic_subscription(
      topic_name, type_name, rclcpp::QoS(10), callback);
  }

  static std::string get_topic_type(const std::string & topic_name)
  {
    auto temp_node = std::make_shared<rclcpp::Node>("topic_type_lookup");

    // Wait for topic to appear (up to 2 seconds)
    for (int i = 0; i < 20; ++i) {
      auto topics_and_types = temp_node->get_topic_names_and_types();
      auto topic_it = topics_and_types.find(topic_name);
      if (topic_it != topics_and_types.end() && !topic_it->second.empty()) {
        return topic_it->second[0];
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return "";
  }

private:
  json_converter_cpp::Converter converter_;
  rclcpp::GenericSubscription::SharedPtr subscription_;
};

class TopicPub : public rclcpp::Node
{
public:
  TopicPub(
    const std::string & topic_name, const std::string & type_name,
    const std::string & json_str)
  : Node("topic_pub"), type_name_(type_name)
  {
    publisher_ = this->create_generic_publisher(topic_name, type_name, rclcpp::QoS(10));

    // Parse JSON
    try {
      json_ = nlohmann::json::parse(json_str);
    } catch (const nlohmann::json::exception & e) {
      RCLCPP_ERROR(this->get_logger(), "Failed to parse JSON: %s", e.what());
      rclcpp::shutdown();
      return;
    }

    // Wait for subscriber
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(100),
      [this]() {
        if (publisher_->get_subscription_count() > 0) {
          publish_and_exit();
        }
      });
  }

private:
  void publish_and_exit()
  {
    rclcpp::SerializedMessage serialized_msg;
    if (converter_.to_msg(type_name_, json_, serialized_msg)) {
      publisher_->publish(serialized_msg);
      RCLCPP_INFO(this->get_logger(), "Published message");
    } else {
      RCLCPP_ERROR(this->get_logger(), "Failed to convert JSON to message");
    }
    rclcpp::shutdown();
  }

  json_converter_cpp::Converter converter_;
  rclcpp::GenericPublisher::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::string type_name_;
  nlohmann::json json_;
};

void print_usage()
{
  std::cout << "Usage:\n";
  std::cout << "  topic echo <topic_name> [type_name]\n";
  std::cout << "  topic pub <topic_name> <type_name> <json_string>\n";
  std::cout << "\n";
  std::cout << "Examples:\n";
  std::cout << "  topic echo /cmd_vel\n";
  std::cout << "  topic echo /cmd_vel geometry_msgs/msg/Twist\n";
  std::cout << R"(  topic pub /cmd_vel geometry_msgs/msg/Twist )"
            << R"('{"linear":{"x":1.0,"y":0.0,"z":0.0},)"
            << R"("angular":{"x":0.0,"y":0.0,"z":0.0}}')" << '\n';
}

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  if (argc < 2) {
    print_usage();
    return 1;
  }

  std::string command = argv[1];

  if (command == "echo") {
    if (argc != 3 && argc != 4) {
      std::cerr << "Error: echo requires <topic_name> [type_name]\n";
      print_usage();
      return 1;
    }

    std::string topic_name = argv[2];
    std::string type_name;

    if (argc == 4) {
      type_name = argv[3];
    } else {
      // Auto-detect topic type
      type_name = TopicEcho::get_topic_type(topic_name);
      if (type_name.empty()) {
        std::cerr << "Error: Could not find topic '" << topic_name
                  << "'. Make sure the topic exists or specify the type explicitly.\n";
        return 1;
      }
      std::cout << "Detected topic type: " << type_name << '\n';
    }

    auto node = std::make_shared<TopicEcho>(topic_name, type_name);
    rclcpp::spin(node);
  } else if (command == "pub") {
    if (argc != 5) {
      std::cerr << "Error: pub requires <topic_name> <type_name> <json_string>\n";
      print_usage();
      return 1;
    }

    std::string topic_name = argv[2];
    std::string type_name = argv[3];
    std::string json_str = argv[4];

    auto node = std::make_shared<TopicPub>(topic_name, type_name, json_str);
    rclcpp::spin(node);
  } else {
    std::cerr << "Error: Unknown command '" << command << "'\n";
    print_usage();
    return 1;
  }

  rclcpp::shutdown();
  return 0;
}
