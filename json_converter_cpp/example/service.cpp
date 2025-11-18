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
#include <rapidjson/writer.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp/serialization.hpp>

#include "json_converter_cpp/converter.hpp"

class ServiceCall : public rclcpp::Node
{
public:
  ServiceCall(
    std::string service_name,
    std::string service_type,
    rapidjson::Document && request_doc)
  : Node("service_call"),
    service_name_(std::move(service_name)),
    service_type_(std::move(service_type)),
    request_doc_(std::move(request_doc))
  {
  }

  bool call_service()
  {
    // Convert JSON to request message object
    std::shared_ptr<void> request_msg;
    std::string request_type = service_type_ + "::Request";
    if (!converter_.to_msg(request_type, request_doc_, request_msg)) {
      std::cerr << "Failed to convert JSON to request message\n";
      return false;
    }

    // Create generic client
    auto client = this->create_generic_client(service_name_, service_type_);

    // Wait for service to be available
    if (!client->wait_for_service(std::chrono::seconds(5))) {
      std::cerr << "Service '" << service_name_ << "' not available\n";
      return false;
    }

    // Call service with message object pointer
    auto future = client->async_send_request(request_msg.get());

    // Wait for response
    if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), future) !=
      rclcpp::FutureReturnCode::SUCCESS)
    {
      std::cerr << "Failed to call service\n";
      return false;
    }

    // Convert response to JSON
    auto response_ptr = future.get();
    rapidjson::Document response_doc;
    response_doc.SetObject();
    auto & allocator = response_doc.GetAllocator();
    std::string response_type = service_type_ + "::Response";
    if (!converter_.to_json(response_type, response_ptr.get(), response_doc, allocator)) {
      std::cerr << "Failed to convert response to JSON\n";
      return false;
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    response_doc.Accept(writer);
    std::cout << buffer.GetString() << '\n';
    return true;
  }

private:
  json_converter_cpp::Converter converter_;
  std::string service_name_;
  std::string service_type_;
  rapidjson::Document request_doc_;
};

void print_usage()
{
  std::cout << "Usage:\n";
  std::cout << "  service call <service_name> <service_type> <json_string>\n";
  std::cout << "\n";
  std::cout << "Examples:\n";
  std::cout << "  service call /add_two_ints example_interfaces/srv/AddTwoInts "
            << "'{\"a\":5,\"b\":3}'\n";
}

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  if (argc < 2) {
    print_usage();
    return 1;
  }

  std::string command = argv[1];

  if (command == "call") {
    if (argc != 5) {
      std::cerr << "Error: call requires <service_name> <service_type> <json_string>\n";
      print_usage();
      return 1;
    }

    std::string service_name = argv[2];
    std::string service_type = argv[3];
    std::string json_string = argv[4];

    rapidjson::Document request_doc;
    if (request_doc.Parse(json_string.c_str()).HasParseError()) {
      std::cerr << "Error parsing JSON at offset " << request_doc.GetErrorOffset() << '\n';
      return 1;
    }

    auto node = std::make_shared<ServiceCall>(
      service_name, service_type, std::move(request_doc));
    bool success = node->call_service();

    rclcpp::shutdown();
    return success ? 0 : 1;
  }

  std::cerr << "Error: Unknown command '" << command << "'\n";
  print_usage();
  return 1;
}
