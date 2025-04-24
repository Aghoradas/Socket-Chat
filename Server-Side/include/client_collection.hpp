#ifndef CLIENT_COLLECTION
  #include <mutex>
  #include <vector>
  #include "client_identity.hpp"
#define CLIENT_COLLECTION

  class ClientCollection {
  public:
    ClientCollection() {}
    std::mutex shared_locker;
    std::vector<defined::ClientIdentity> CLIENTS;
    
    void add_client(defined::ClientIdentity& client) {
      std::unique_lock lock(shared_locker); 
      CLIENTS.emplace_back(client);
    } 

    int client_amount() {
      std::unique_lock lock(shared_locker);
      int size = CLIENTS.size();
      return size;
    }

    std::vector<defined::ClientIdentity> get_clients() {
      std::vector<defined::ClientIdentity> called_clients = CLIENTS;
      return called_clients;
    }
  };

#endif
