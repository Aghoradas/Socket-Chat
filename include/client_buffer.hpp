#ifndef CLIENT_BUFFER_HPP
#define CLIENT_BUFFER_HPP

#include <string>
#include <vector>
#include <mutex>

namespace receiver {
  class Buffer {
  private:
    std::vector<std::string> buffer_line;
    std::mutex mtx;

  public:

    std::string get_buffer(int line_number) {
      std::lock_guard<std::mutex> lock(mtx);
      return buffer_line.at(line_number);
    }

    void store_to_buffer(std::string& new_line) {
      std::lock_guard<std::mutex> lock(mtx);
      buffer_line.emplace_back(new_line);
    }

    int buffer_size() {
      std::lock_guard<std::mutex> lock(mtx);
      return buffer_line.size();
    }

    void cls() {
     std::lock_guard<std::mutex> lock(mtx);
     buffer_line.clear();
    }

  };

}

#endif
