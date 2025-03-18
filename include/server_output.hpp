#ifndef SERVER_OUTPUT_HPP
#define SERVER_OUTPUT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <netinet/in.h>

namespace sending {
  class Output {
  public:
    int bytes_sent;
    char* data_packet;

    void buffer_send(int client_connection, std::vector<std::string> buffer_messages) {
      for (const std::string& line : buffer_messages) {
        bytes_sent = send(client_connection, line.c_str(), line.size(), 0);
        if (bytes_sent <= 0) {
          std::cerr << "-error sending data: " << line.data() << std::endl;
        }
      }
    }
  };
}
#endif 
