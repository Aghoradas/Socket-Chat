#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <cstring>
#include <sstream>
#include <iostream>
#include <vector>
#include <thread>

#include "../include/client_buffer.hpp"

int G_PORT = 8080;
int G_HEARTBEAT_PORT = 8081;


void cls() {
  system("clear");
}


// Listens for a heartbeat connection from server
void heartbeat_listen(const int heartbeat_connection, const int client_connection) {
  char buffer[10];
  int time_out = 0;
  while(true) {
    time_out++;

    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(heartbeat_connection, buffer, sizeof(buffer), 0);

    if (bytes_received > 0 && strncmp(buffer, "PING", 4) == 0) {
      time_out = 0;
      const char* pong_message = "PONG";
      send(heartbeat_connection, pong_message, strlen(pong_message), 0);
    }

    if (time_out == 10) {
      shutdown(heartbeat_connection, SHUT_RDWR);
      close(heartbeat_connection);
      break;
    }
  }
  shutdown(client_connection, SHUT_RDWR);
  close(client_connection);
  exit(0);
}


// Takes in received message that came from a client to prepare it to be printed
// to the screen.
std::vector<std::string> message_parser(std::string &to_be_parsed) {
  std::vector<std::string> member_message;
  std::string tdp_string = std::string(to_be_parsed);
  std::stringstream ss(tdp_string);
  std::string part;
  while (std::getline(ss, part, '|')) {
    member_message.emplace_back(part);
  }

  return member_message;
}


void print_buffer(receiver::Buffer& buffer_package) {
  // Each line of the buffer_line vector contains first, who posted the message,
  // then second, what was posted.
  std::cout << "==========================================" << std::endl;
  if (buffer_package.buffer_size() == 0) {
    std::cout << "  -nothing in buffer\n";
    std::cout << "==========================================" << std::endl;
    return;
  }
  for (int i = 0; i < buffer_package.buffer_size(); i++) {  // Iterating through maps of the vector
      std::cout << " - " << buffer_package.get_buffer(i) << std::endl;
  }
  std::cout << "==========================================" << std::endl;
}


void receiving_from_server(const int& client_socket, const std::string& message, receiver::Buffer& buffer_package) {
  printf("\n-receiving_from_server: started...\n");
  // *** HERE - information is getting through, though its reception
  //            is chopped and needs to be reworked and heavily perfected.data_packet


  // SEND INITIAL CONNECTION MESSAGE TO SERVER
  if (send(client_socket, message.c_str(), message.length(), 0) < 0) {
    std::cerr << "-error sending INITIAL message to server\n" << std::endl;
    return;
  }
  std::cout << "\n-connection accepted" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  int total_received = 0;

  // Initializing data_packet and data_str
  char data_packet[1025];
  memset(data_packet, 0, sizeof(data_packet));
  std::string data_str;
  uint32_t buffer_size;
  uint32_t bytes_data;

  while (true) {
    buffer_size = 0;
    bytes_data = 0;
    memset(data_packet, 0, sizeof(data_packet));
    recv(client_socket, &buffer_size, sizeof(buffer_size), 0);
    buffer_size = ntohl(buffer_size);
    while (bytes_data < buffer_size) {
      bytes_data = (recv(client_socket, data_packet, sizeof(data_packet), 0));
      if (bytes_data > 0) {
        data_packet[bytes_data] = '\0';

        data_str = std::string(data_packet);
        buffer_package.store_to_buffer(data_str);
      }
    }
  }
}


int main() {
  cls();
  std::string client_name;
  std::cout << "      Chat Client" << std::endl;
  std::cout << "-----------------------" << std::endl;
  std::cout << "username: ";
  getline(std::cin, client_name);

  std::vector<std::string> message_data;

  // Creating sockets
  int client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket < 0) {
    std::cerr << "-socket failed to initiate\n" << std::endl;
    return -1;
  }
  std::cout << "\n-client connected";
  std::this_thread::sleep_for(std::chrono::seconds(1));
  int heartbeat_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (heartbeat_socket < 0) {
    std::cerr << "\n-socket failed to initiate" << std::endl;
    return -1;
  }
  std::cout << "\n-heartbeat established";
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Socket address
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(G_PORT);
  if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
    std::cerr << "-invalid address: address not supported\n" << std::endl;
    return -1;
  }
  struct sockaddr_in heartbeat_address;
  heartbeat_address.sin_family = AF_INET;
  heartbeat_address.sin_port = htons(G_HEARTBEAT_PORT);
  if (inet_pton(AF_INET, "127.0.0.1", &heartbeat_address.sin_addr) <= 0) {
    std::cerr << "-invalid address: address not supported\n" << std::endl;
    return -1;
  }

  // CLIENT CONNECT
  if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
    std::cerr << "-error connecting\n" << std::endl;
    return -1;
  }
  if (connect(heartbeat_socket, (struct sockaddr*)&heartbeat_address, sizeof(heartbeat_address)) < 0) {
    std::cerr << "-error connecting heartbeat\n" << std::endl;
    return -1;
  }
  std::cout << "\n-connected to server and heartbeat\n";
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // STARTING HEARTBEAT THREAD
  std::thread heartbeat_thread(heartbeat_listen, heartbeat_socket, client_socket);
  heartbeat_thread.detach();

  // Loop communication session with server
  std::string client_input;
  std::string client_to_server;
  receiver::Buffer buffer_package;
  const std::string message = "!username|" + client_name + "|Client connected...";
  std::thread([&](){ receiving_from_server(client_socket, message, buffer_package); }).detach();

  // Initializing data_packet and data_str
  char data_packet[1025] = {0};
  memset(data_packet, 0, sizeof(data_packet));
  std::string data_str;

  
  while(true) {
    cls();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    print_buffer(buffer_package);
    do {
      std::cout << "\nEnter message: ";
      std::getline(std::cin, client_input);
    } while (client_input.size() == 0);
    client_to_server = "*" + client_name + "|" + client_input;
    if (send(client_socket, client_to_server.c_str(), client_to_server.size(), 0) < 0) {
          std::cerr << "-error sending message to server" << std::endl;
    }
    if (client_input == "!close") {
      std::cout << "\ngood bye" << std::endl;
      break;

    }

    std::cout << "\n-end of recv function" << std::endl;

  }
  shutdown(client_socket, SHUT_RDWR);
  close(client_socket);
  std::cout << "-server disconnected" << std::endl;
  sleep(1);
  std::cout << "-goodbye\n" << std::endl;
}
