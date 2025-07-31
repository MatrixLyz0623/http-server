#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <functional>
#include <algorithm>
#include <thread>
#include <fstream>

#include "http_parser.h"
#include "http_response.h"
#include "dispatcher.h"
#include "compress.h"

void handle_client(int client_socket, const Dispatcher& dispatcher) {
    char buffer[4096] = {};
    HttpRequestParser parser;

    ssize_t byteReads = read(client_socket, buffer, sizeof(buffer));
    if (byteReads < 0) {
        std::cerr << "Read Error From Client\n";
        close(client_socket);
        return;
    }
    std::cout << "Read " << byteReads << " bytes\n";
    std::cout << "Raw request:\n" << std::string(buffer, byteReads) << std::endl;
    parser.parse(buffer);
    std::cout << "method: " << parser.get_method() << std::endl;
    HttpResponse response = dispatcher.dispatch(parser);
    std::string responseStr = response.to_string();

    send(client_socket, responseStr.c_str(), responseStr.size(), 0);
    shutdown(client_socket, SHUT_WR);
    close(client_socket);
}

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  
  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }
  //
  // // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }
  //
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);
  //
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }
  //
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }


  //register
  Dispatcher dispatcher;

  dispatcher.registerHandler("GET", "/", [](const HttpRequestParser& req) {
      return HttpResponse::ok("Welcome to the homepage!");
  });

  dispatcher.registerHandlerPrefix("GET", "/echo/" ,[](const HttpRequestParser& req) {
      std::string body = req.get_path().substr(6); // 去掉 "/echo/"
      return HttpResponse::ok(body);
  });

  dispatcher.registerHandler("GET", "/user-agent", [](const HttpRequestParser& req) {
     std::string body = req.get_header("User-Agent");
     return HttpResponse::ok(body);
  });

  dispatcher.registerHandlerPrefix("GET", "/files/", [](const HttpRequestParser& req) {
    std::string filename = req.get_path().substr(7);
    std::string fullPath = "/tmp/" + filename;
    std::ifstream file(fullPath, std::ios::binary);
    std::cout << "Attempting to open: " << fullPath << std::endl;

    if (file) {
      std::ostringstream oos;
      oos << file.rdbuf();
      std::string body = oos.str();
      CompressionType compressType = select_compression(req.get_header("Accept-Encoding"));
      if (compressType != CompressionType::None) {
        switch (compressType) {
            case CompressionType::Gzip: {
                std::string compressed = gzip_compress(body);
                return HttpResponse(200, "OK", "text/plain", compressed);
                break;
            }
            case CompressionType::None:
            default:
                return HttpResponse(200, "OK", "text/plain", body);
                break;
        }
      } else {
        return HttpResponse(200, "OK", "application/octet-stream", body);
      }
    } else {
      return HttpResponse::notFound();
    }
  });

    dispatcher.registerHandlerPrefix("POST", "/files/", [](const HttpRequestParser& req) {
    std::string filename = req.get_path().substr(7);
    std::string fullPath = "/tmp/" + filename;
    std::ofstream file(fullPath, std::ios::binary);
    std::cout << "Attempting to open: " << fullPath << std::endl;
    if (file) {
      file << req.get_body();
      return HttpResponse(201, "Created", "", "File created");
    } else {
        return HttpResponse(500, "Internal Server Error", "text/plain", "Could not create file");
    }
  });

  while (true) {
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    std::cout << "Waiting for a client to connect...\n";
    
    int client_socket  = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
    if (client_socket < 0) {
        std::cerr << "Failed to accept connection";
        continue;
    }
    std::thread t(handle_client, client_socket, std::ref(dispatcher));
    t.detach();
  }

  close(server_fd);


  return 0;
}
