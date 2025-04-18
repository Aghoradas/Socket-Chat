#ifndef BUFFER_HISTORY_HPP
#define BUFFER_HISTORY_HPP

#include <queue>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <shared_mutex>

namespace buffer {
  class History {
  private:
    std::mutex datalines_mutex;
    std::shared_mutex shared_locker;
    std::vector<std::string> data_lines;
    std::queue<std::string> queue_data; // Only being used currently to queue the next line to send to client

  public:

    void store_data(const std::string& message_data) {
      std::unique_lock lock(datalines_mutex);
      queue_data.push(message_data);
      data_lines.emplace_back(message_data);
      std::cout << "\n-data stored: " << message_data << std::endl;
      return;
    }

    int queue_size() {
      std::unique_lock lock(datalines_mutex);
      int size = queue_data.size();
      return size;
    }
    
    std::string get_data() {
      std::shared_lock lock(shared_locker);
      std::string next_up = queue_data.front();
      queue_data.pop();
      return next_up;
    }

    std::vector<std::string> message_parser(std::string& to_be_parsed) {
      std::unique_lock lock(datalines_mutex);
      std::vector<std::string> member_message;
      std::stringstream ss(to_be_parsed);
      std::string part;
      while (std::getline(ss, part, '|')) {
        member_message.emplace_back(part);
      }
      return member_message;
    }

    void print_buffer(std::string& current_time) {
      std::lock_guard<std::mutex> guard(datalines_mutex);
      std::cout << " Time: " << current_time << std::endl;
      std::cout << "------------------------------------------------" << std::endl;
      for(const std::string &line : data_lines) {
        if (!data_lines.empty()) {
          std::cout << " -" << line << std::endl;
        } else {
          std::cout << "-none";
        }
      }
    }

  };

}
#endif
