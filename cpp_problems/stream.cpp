#include "test_runner.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/event.h>
using namespace std;

class Buffer {
  public:
    Buffer(int bufferSize) : bufferSize_(bufferSize), buffer_(new char[bufferSize]), bytesReceived_(0) {}

    ~Buffer() { delete[] buffer_; }

    void recvData(int socket) {
      int kq = kqueue();
      if (kq < 0) {
        throw std::runtime_error("Failed to create kqueue");
      }

      struct kevent event;
      EV_SET(&event, socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
      if (kevent(kq, &event, 1, NULL, 0, NULL) < 0) {
        throw std::runtime_error("Failed to add socket to kqueue");
      }

      while (true) {
        struct timespec timeout = {1, 0}; // 1-second timeout
        struct kevent events[1];
        int num_events = kevent(kq, NULL, 0, events, 1, &timeout);
        if (num_events < 0) {
          throw std::runtime_error("Failed to wait for kqueue events");
        }

        if (events[0].filter == EVFILT_READ) {
          char temp[bufferSize_];
          int bytes_received = recv(socket, temp, 1024, 0);
          if (bytes_received < 0) {
            throw std::runtime_error("Failed to receive data");
          }

          if (bytes_received == 0) {
            break;
          }

          copy(temp, temp + bytes_received, buffer_ + bytesReceived_);
          bytesReceived_ += bytes_received;

          // Process messages
          while (bytesReceived_ >= headerSize_) {
            int messageSize = getMessageSize();
            if (bytesReceived_ >= messageSize) {
              processMessage();
              bytesReceived_ -= messageSize;
              std::memmove(buffer_, buffer_ + messageSize, bytesReceived_);
            } else {
              break;
            }
          }
        }
      }
      close(kq);
    }

  private:
    const int bufferSize_;
    char* buffer_;
    int bytesReceived_;
    static const int headerSize_ = 4; // Example header size

    struct Header {
      int messageType;
      int payloadSize;

      Header(int messageType, int payloadSize) : messageType(messageType), payloadSize(payloadSize) {}

      int getMessageType() const { return messageType; }
      int getPayloadSize() const { return payloadSize; }
    };

    struct Payload {
      std::string data;

      Payload(const std::string& data) : data(data) {}

      std::string getData() const { return data; }
    };

    int getMessageSize() {
      if (bytesReceived_ < headerSize_) {
        throw std::runtime_error("Not enough data to determine message size");
      }
      Header header = parseHeader(buffer_, headerSize_);
      return headerSize_ + header.getPayloadSize();
    }

    void processMessage() {
      try {
        // Extract and process header
        Header header = parseHeader(buffer_, headerSize_);

        // Extract and process payload
        Payload payload = parsePayload(buffer_ + headerSize_, header.getPayloadSize());

        // Store or handle processed message
        std::cout << "Received message: " << header.getMessageType() << " - " << payload.getData() << std::endl;
      } catch (const std::exception& e) {
        std::cerr << "Error processing message: " << e.what() << std::endl;
      }
    }


    Header parseHeader(char* buffer, int headerSize) {
      if (headerSize != 8) {
        throw std::runtime_error("Invalid header size");
      }
      int messageType;
      int payloadSize;
      std::memcpy(&messageType, buffer, 4);
      std::memcpy(&payloadSize, buffer + 4, 4);
      return Header(ntohl(messageType), ntohl(payloadSize));
    }

    Payload parsePayload(char* buffer, int payloadSize) {
      return Payload(std::string(buffer, payloadSize));
    }

};

int createSocket() {
  // Initialize socket
  int socket = ::socket(AF_INET, SOCK_STREAM, 0);
  if (socket < 0) {
    throw std::runtime_error("Failed to create socket");
  }

  // Set up server address
  sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(8080);
  inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

  // Connect to server
  if (connect(socket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    throw std::runtime_error("Failed to connect to server");
  }

  return socket;

}

void closeSocket(int socket) {
  if (close(socket) < 0) {
    std::cerr << "Error closing socket" << std::endl;
  }
}

int main() {
  try {
    int socket = createSocket();
    if (socket < 0) {
      throw std::runtime_error("Error creating socket");
    }

    Buffer buffer(1024);
    buffer.recvData(socket);
    closeSocket(socket);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;

}
