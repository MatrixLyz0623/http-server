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


class HttpRequestParser {
public:
    bool parse(const std::string& buffer);
    const std::string& get_method() const;
    const std::string& get_path() const;

private:
    std::string method;
    std::string path;
};

bool HttpRequestParser::parse(const std::string& buffer) {
    std::istringstream stream(buffer);
    std::string line;

    if (std::getline(stream, line)) {
        std::istringstream linestream(line);
        linestream >> method >> path;
        return !method.empty() && !path.empty(); 
    }
    return false;
}

const std::string& HttpRequestParser::get_method() const {
    return method;
}

const std::string& HttpRequestParser::get_path() const {
    return path;
}

class HttpResponse {
public:
    static HttpResponse ok(const std::string& body);
    static HttpResponse notFound();

    HttpResponse(const HttpResponse& other)
        : status_code_(other.status_code_),
          status_text_(other.status_text_),
          content_type_(other.content_type_),
          body_(other.body_) {
        std::cout << "copy construct" << std::endl;
    }

    HttpResponse(HttpResponse&& other)
        : status_code_(other.status_code_),
          status_text_(std::move(other.status_text_)),
          content_type_(std::move(other.content_type_)),
          body_(std::move(other.body_)) {
        std::cout << "move construct" << std::endl;
    }

    HttpResponse(int status_code,
                 std::string status_text,
                 std::string content_type,
                 std::string body)
        : status_code_(status_code),
          status_text_(std::move(status_text)),
          content_type_(std::move(content_type)),
          body_(std::move(body)) {
            std::cout<< "construct" << std::endl;
          }

    std::string to_string() const;

private:
    int status_code_;
    std::string status_text_;
    std::string content_type_;
    std::string body_;
};


HttpResponse HttpResponse::ok(const std::string& body = "") {
    std::cout<<"call OK"<< std::endl;
    return HttpResponse(200, "OK", "text/plain", body);
}

HttpResponse HttpResponse::notFound() {
    return HttpResponse(404, "NotFound", "text/plain", "404 Not Found");
}

std::string HttpResponse::to_string() const {
    std::ostringstream os;
    os << "HTTP/1.1 " << status_code_ << " " << status_text_ << "\r\n";
    if (!content_type_.empty()) {
        os << "Content-Type: " << content_type_ << "\r\n";
    }
    os << "Content-Length: " << body_.size() << "\r\n";
    os << "\r\n";
    os << body_;
    return os.str();
}

class Dispatcher {
public:
    using Handler = std::function<HttpResponse(const std::string& path)>;

    void registerHandler(const std::string& path, Handler handler);
    void registerHandlerPrefix(const std::string& prefix, Handler handler);
    HttpResponse dispatch(const std::string& path) const;

private:
    std::unordered_map<std::string, Handler> exactRoutes_;
    std::vector<std::pair<std::string, Handler>> prefixRoutes_;
};

void Dispatcher::registerHandler(const std::string& path, Handler handler) {
    exactRoutes_[path] = std::move(handler);
}

void Dispatcher::registerHandlerPrefix(const std::string& prefix, Handler handler) {
    prefixRoutes_.emplace_back(prefix, std::move(handler));
}

HttpResponse Dispatcher::dispatch(const std::string& path) const {
    // 精确匹配
    auto it = exactRoutes_.find(path);
    if (it != exactRoutes_.end()) {
        return it->second(path);
    }

    // 前缀匹配
    for (auto it = prefixRoutes_.begin(); it != prefixRoutes_.end(); ++it) {
        if (path.rfind(it->first, 0) == 0) {
            return it->second(path);
        }
    }

    // 未匹配
    return HttpResponse::notFound();
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

  dispatcher.registerHandler("/", [](const std::string& path) {
      return HttpResponse::ok("Welcome to the homepage!");
  });

  dispatcher.registerHandlerPrefix("/echo/", [](const std::string& path) {
      std::string body = path.substr(6); // 去掉 "/echo/"
      return HttpResponse::ok(body);
  });

  while (true) {
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    std::cout << "Waiting for a client to connect...\n";
    
    int client_scoket = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
    std::cout << "Client connected\n";
  
    char buffer[4096] = {};
    HttpRequestParser parser;
    ssize_t byteReads = read(client_scoket, buffer, sizeof(buffer));
    if (byteReads < 0 ) {
      std::cerr << "Read Error From Client";
      return 1;
    }
    parser.parse(buffer);
    std::string path = parser.get_path();
    HttpResponse response = dispatcher.dispatch(path);
    std::string responseStr = response.to_string();

    send(client_scoket, responseStr.c_str(), responseStr.size(), 0);

    shutdown(client_scoket, SHUT_WR); 
  }

    close(server_fd);


  return 0;
}
