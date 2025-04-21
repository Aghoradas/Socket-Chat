# Socket-Chat

SOCKET-CHAT (CHUI-FACE): 
  A real-time chat application built using C++, socket programming, and a Client-GUI. This project demonstrates
  the client-server architecture, network protocols, and real-time communication with an easy-to-use graphical interface.

  Users can connect to the server, send and receive messages instantly, and interact with a user-friendly GUI
  that makes communication seamless.


FEATURES:

 *  Real-Time Messaging: Clients can send and receive messages instantly over a network.
 *  Multi-Client Support: Multiple clients can connect to the server simultaneously and chat with each other.
 *  User-Friendly GUI: A basic but functional graphical user interface (GUI) built with a simple C++ GUI library.
 *  Command-Line Support: In addition to the GUI, a text-based interface is available for quick testing and debugging.
 *  Server Support: The server can handle multiple connected clients, broadcasting messages to all active participants in real-time.


TECHNOLOGIES USED:

 *  Languages: C++
 *  Libraries:
    	For GUI: WxWidgets
    	For Sockets: Basic C/C++ libraries
 *  Protocols: TCP/IP (for network communication)
 *  Tools:
    	Makefile, CMake, GNU C++ Compiler Collection
    	Terminal/Command Line for testing
 *  Operating Systems: Linux


LIMITATIONS:

 *  No Encryption: The communication between the client and server is not encrypted. For secure communication, consider
      implementing SSL/TLS encryption.
 *  Basic GUI: The graphical interface is simple and designed for educational purposes. A more polished interface with
      additional features could be developed.
 *  Local Network Communication: The current setup assumes all communication happens over local networks or
      localhost. For remote communication, additional setup would be required, such as handling NAT and public IPs.


FUTURE IMPROVEMENTS:

 *  Private Messaging: Allow users to send messages directly to individual clients.
 *  Message History: Implement a feature to view past messages in the chat.
 *  Secure Communication: Add encryption (e.g., using OpenSSL) to secure the messages between clients and the server.
 *  Enhanced GUI: Develop a more feature-rich GUI with custom themes, emojis, and better message formatting.


CONTRIBUTING:
 *  Contributions are welcome! If you want to help improve the project, feel free to open an issue or submit a pull
    request. Suggestions for new features or improvements are always appreciated.

