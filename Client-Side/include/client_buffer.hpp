#ifndef CLIENT_BUFFER_HPP
#define CLIENT_BUFFER_HPP

#include <string>
#include <vector>
#include <mutex>
#include <shared_mutex>


namespace receiver {
  class Buffer {
  private:
    std::vector<std::string> buffer_line;
    std::mutex mtx;
    std::shared_mutex locker;

  public:

    std::string get_buffer(int line_number) {
      std::shared_lock lock(locker);
      std::string chat = buffer_line[line_number];
      return chat;
    }

    void store_to_buffer(std::string& new_line) {
      std::unique_lock lock(mtx);
      buffer_line.emplace_back(new_line);
      return;
    }

    int buffer_size() {
      std::unique_lock lock(locker);
      int size = buffer_line.size();
      return size;
    }

    bool buffer_empty() {
      std::unique_lock lock(locker);
      bool nothing = buffer_line.empty();
      return nothing;
    }

    void cls() {
     std::unique_lock<std::mutex> lock(mtx);
     buffer_line.clear();
     return;
    }

  };

}

#endif
