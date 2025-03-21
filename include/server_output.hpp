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
    std::string end_packet = "...";
    uint32_t line_size;

    void buffer_send(int client_connection, std::vector<std::string> buffer_messages) {
      uint32_t buffer_size = htonl(buffer_messages.size());
      send(client_connection, &buffer_size, sizeof(buffer_size), 0);
      for (const std::string& line : buffer_messages) {
        line_size = htonl(line.size());
        send(client_connection, &line_size, sizeof(line_size), 0);
        bytes_sent = send(client_connection, line.c_str(), line.size(), 0);
        if (bytes_sent <= 0) {
          std::cerr << "-error sending data: " << line.data() << std::endl;
        }
      }

      send(client_connection, end_packet.c_str(), end_packet.size(), 0);
    }
  };
}
#endif 
