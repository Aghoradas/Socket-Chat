#ifndef SERVER_OUTPUT_HPP
#define SERVER_OUTPUT_HPP

#include <iostream>
#include <string>
#include <netinet/in.h>

#include "buffer_history.hpp"

namespace sending {
  class Output {
  public:
    uint32_t bytes_sent;
    std::string data_packet;

    void buffer_send(const int& client_connection, buffer::History& buffer_messages) {
      printf("\n-buffer_send has started...\n");
      while (true) {
        if (buffer_messages.queue_size() > 0) {

          bytes_sent = 0;
          data_packet = buffer_messages.get_data();
          uint32_t size = data_packet.length();
          size = htonl(size);
          send(client_connection, &size, sizeof(size), 0);
          size = ntohl(size);
          while (bytes_sent < size) {
            bytes_sent = send(client_connection, data_packet.c_str(), data_packet.length(), 0);
            if (bytes_sent < 0) {
              std::cerr << "-error sending data: " << data_packet << std::endl;
            }
          }
        }
      }
    }
  };
}
#endif
