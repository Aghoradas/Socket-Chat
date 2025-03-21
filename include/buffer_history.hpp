#ifndef BUFFER_HISTORY_HPP
#define BUFFER_HISTORY_HPP

#include <condition_variable>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <mutex>



namespace buffer {
  class History {
  private:
    std::mutex datalines_mutex;
    std::vector<std::string> data_lines;
    std::condition_variable cv;

  public:

    void store_data(std::string& message_data) {
      std::lock_guard<std::mutex> guard(datalines_mutex);
      data_lines.emplace_back(message_data);
      std::cout << "\ndata stored: " << message_data << std::endl;
    }
    
    std::vector<std::string> get_data() {
      std::lock_guard<std::mutex> guard(datalines_mutex);
      return data_lines;
    }

    std::vector<std::string> message_parser(std::string &to_be_parsed) {
      std::lock_guard<std::mutex> guard(datalines_mutex);
      std::vector<std::string> member_message;
      std::stringstream ss(to_be_parsed);
      std::string part;
      while (std::getline(ss, part, '|')) {
        member_message.emplace_back(part);
      }
      return member_message;
    }

    void print_buffer(std::string current_time) {
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
