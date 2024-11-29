#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include "test_runner.h"
using namespace std;

class Buffer
{
   public:
    Buffer(int bufferSize)
        : bufferSize_(bufferSize), buffer_(new char[bufferSize]), bytesReceived_(0)
    {
    }

    ~Buffer()
    {
        delete[] buffer_;
    }

    void recvData(int socket)
    {
        int epfd = epoll_create1(0);
        if (epfd < 0) {
            throw runtime_error("Failed to create epoll instance");
        }

        struct epoll_event event;
        event.events  = EPOLLIN;
        event.data.fd = socket;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, socket, &event) < 0) {
            throw runtime_error("Failed to add socket to epoll");
        }

        while (true) {
            struct epoll_event events[1];
            int                num_events = epoll_wait(epfd, events, 1, 1000); // 1-second timeout
            if (num_events < 0) {
                throw runtime_error("Failed to wait for epoll events");
            }

            if (num_events == 0) {
                continue; // Timeout, no events
            }

            if (events[0].events & EPOLLIN) {
                char temp[bufferSize_];
                int  bytes_received = recv(socket, temp, sizeof(temp), 0);
                if (bytes_received < 0) {
                    throw runtime_error("Failed to receive data");
                }

                if (bytes_received == 0) {
                    break; // Connection closed
                }

                copy(temp, temp + bytes_received, buffer_ + bytesReceived_);
                bytesReceived_ += bytes_received;

                // Process messages
                while (bytesReceived_ >= headerSize_) {
                    int messageSize = getMessageSize();
                    if (bytesReceived_ >= messageSize) {
                        processMessage();
                        bytesReceived_ -= messageSize;
                        memmove(buffer_, buffer_ + messageSize, bytesReceived_);
                    } else {
                        break;
                    }
                }
            }
        }

        close(epfd);
    }

   private:
    int              bufferSize_;
    char*            buffer_;
    int              bytesReceived_;
    static const int headerSize_ = 4; // Example header size

    struct Header {
        int messageType;
        int payloadSize;

        Header(int messageType, int payloadSize)
            : messageType(messageType), payloadSize(payloadSize)
        {
        }

        int getMessageType() const
        {
            return messageType;
        }
        int getPayloadSize() const
        {
            return payloadSize;
        }
    };

    struct Payload {
        string data;

        Payload(const string& data) : data(data) {}

        string getData() const
        {
            return data;
        }
    };

    int getMessageSize()
    {
        if (bytesReceived_ < headerSize_) {
            throw runtime_error("Not enough data to determine message size");
        }
        Header header = parseHeader(buffer_, headerSize_);
        return headerSize_ + header.getPayloadSize();
    }

    void processMessage()
    {
        try {
            // Extract and process header
            Header header = parseHeader(buffer_, headerSize_);

            // Extract and process payload
            Payload payload = parsePayload(buffer_ + headerSize_, header.getPayloadSize());

            // Store or handle processed message
            cout << "Received message: " << header.getMessageType() << " - " << payload.getData()
                 << endl;
        } catch (const exception& e) {
            cerr << "Error processing message: " << e.what() << endl;
        }
    }

    Header parseHeader(char* buffer, int headerSize)
    {
        if (headerSize != 8) {
            throw runtime_error("Invalid header size");
        }
        int messageType;
        int payloadSize;
        memcpy(&messageType, buffer, 4);
        memcpy(&payloadSize, buffer + 4, 4);
        return Header(ntohl(messageType), ntohl(payloadSize));
    }

    Payload parsePayload(char* buffer, int payloadSize)
    {
        return Payload(string(buffer, payloadSize));
    }
};

int createSocket()
{
    // Initialize socket
    int socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
        throw runtime_error("Failed to create socket");
    }

    // Set up server address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port   = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    // Connect to server
    if (connect(socket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        throw runtime_error("Failed to connect to server");
    }

    return socket;
}

void closeSocket(int socket)
{
    if (close(socket) < 0) {
        cerr << "Error closing socket" << endl;
    }
}

int main()
{
    try {
        int socket = createSocket();
        if (socket < 0) {
            throw runtime_error("Error creating socket");
        }

        Buffer buffer(1024);
        buffer.recvData(socket);
        closeSocket(socket);
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}
