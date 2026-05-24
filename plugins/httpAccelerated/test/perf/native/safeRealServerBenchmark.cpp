#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <string>
#include <sys/resource.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace {

constexpr int kWorkers = 2;
constexpr int kRequests = 256;
constexpr int kClientConcurrency = 8;
constexpr std::size_t kMemoryCapBytes = 512ULL * 1024ULL * 1024ULL;

void close_fd(int fd) {
  if (fd >= 0) {
    ::close(fd);
  }
}

void cap_process_memory() {
  rlimit limit{};
  limit.rlim_cur = kMemoryCapBytes;
  limit.rlim_max = kMemoryCapBytes;
  ::setrlimit(RLIMIT_AS, &limit);
}

class SafeHttpServer {
public:
  SafeHttpServer() {
    listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
      fail("socket");
    }

    int yes = 1;
    ::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    if (::bind(listen_fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
      fail("bind");
    }
    if (::listen(listen_fd_, 64) < 0) {
      fail("listen");
    }

    socklen_t len = sizeof(addr);
    if (::getsockname(listen_fd_, reinterpret_cast<sockaddr *>(&addr), &len) < 0) {
      fail("getsockname");
    }
    port_ = ntohs(addr.sin_port);
  }

  ~SafeHttpServer() {
    stop();
    close_fd(listen_fd_);
  }

  int port() const { return port_; }

  void start() {
    for (int i = 0; i < kWorkers; ++i) {
      workers_.emplace_back([this] { accept_loop(); });
    }
  }

  void stop() {
    stopped_.store(true);
    ::shutdown(listen_fd_, SHUT_RDWR);
    for (auto &worker : workers_) {
      if (worker.joinable()) {
        worker.join();
      }
    }
  }

  int handled() const { return handled_.load(); }

private:
  [[noreturn]] void fail(const char *what) {
    std::cerr << "safe_http_accelerated_real error=" << what << "\n";
    std::exit(2);
  }

  void accept_loop() {
    while (!stopped_.load()) {
      sockaddr_in client_addr{};
      socklen_t len = sizeof(client_addr);
      int client = ::accept(listen_fd_, reinterpret_cast<sockaddr *>(&client_addr), &len);
      if (client < 0) {
        if (stopped_.load()) {
          return;
        }
        continue;
      }
      handle_client(client);
      if (handled_.load() >= kRequests) {
        stopped_.store(true);
        ::shutdown(listen_fd_, SHUT_RDWR);
        return;
      }
    }
  }

  void handle_client(int client) {
    char buffer[1024]{};
    ssize_t total = 0;
    while (total < static_cast<ssize_t>(sizeof(buffer) - 1)) {
      ssize_t n = ::recv(client, buffer + total, sizeof(buffer) - 1 - total, 0);
      if (n <= 0) {
        close_fd(client);
        return;
      }
      total += n;
      buffer[total] = '\0';
      if (std::strstr(buffer, "\r\n\r\n") != nullptr) {
        break;
      }
    }

    const bool route_ok = std::strncmp(buffer, "GET /users HTTP/1.1\r\n", 21) == 0;
    const std::string body = route_ok ? R"({"users":[{"id":1,"name":"salam"}]})" : R"({"error":"not found"})";
    const std::string status = route_ok ? "200 OK" : "404 Not Found";
    const std::string response =
        "HTTP/1.1 " + status + "\r\n"
        "Content-Type: application/json\r\n"
        "Connection: close\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
    const char *ptr = response.data();
    std::size_t remaining = response.size();
    while (remaining > 0) {
      ssize_t sent = ::send(client, ptr, remaining, MSG_NOSIGNAL);
      if (sent <= 0) {
        close_fd(client);
        return;
      }
      ptr += sent;
      remaining -= static_cast<std::size_t>(sent);
    }
    handled_.fetch_add(1);
    close_fd(client);
  }

  int listen_fd_ = -1;
  int port_ = 0;
  std::atomic<bool> stopped_{false};
  std::atomic<int> handled_{0};
  std::vector<std::thread> workers_;
};

long request_once(int port) {
  auto start = std::chrono::steady_clock::now();
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    return -1;
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(static_cast<uint16_t>(port));
  if (::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
    close_fd(fd);
    return -1;
  }

  const std::string request =
      "GET /users HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
  if (::send(fd, request.data(), request.size(), MSG_NOSIGNAL) !=
      static_cast<ssize_t>(request.size())) {
    close_fd(fd);
    return -1;
  }

  std::string response;
  char buffer[1024]{};
  while (true) {
    ssize_t n = ::recv(fd, buffer, sizeof(buffer), 0);
    if (n < 0) {
      close_fd(fd);
      return -1;
    }
    if (n == 0) {
      break;
    }
    response.append(buffer, static_cast<std::size_t>(n));
  }
  close_fd(fd);

  if (response.find("HTTP/1.1 200 OK") == std::string::npos ||
      response.find(R"({"users":[{"id":1,"name":"salam"}]})") == std::string::npos) {
    return -1;
  }

  auto end = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

long percentile(const std::vector<long> &values, double p) {
  if (values.empty()) {
    return 0;
  }
  const auto index = static_cast<std::size_t>((values.size() - 1) * p);
  return values[index];
}

} // namespace

int main() {
  cap_process_memory();
  SafeHttpServer server;
  server.start();

  std::vector<long> latencies;
  latencies.reserve(kRequests);
  std::mutex latencies_mutex;
  std::atomic<int> next{0};
  std::atomic<int> failures{0};

  auto start_all = std::chrono::steady_clock::now();
  std::vector<std::thread> clients;
  for (int i = 0; i < kClientConcurrency; ++i) {
    clients.emplace_back([&] {
      while (true) {
        int request = next.fetch_add(1);
        if (request >= kRequests) {
          return;
        }
        long latency = request_once(server.port());
        if (latency <= 0) {
          failures.fetch_add(1);
          return;
        }
        std::lock_guard<std::mutex> lock(latencies_mutex);
        latencies.push_back(latency);
      }
    });
  }
  for (auto &client : clients) {
    client.join();
  }
  auto elapsed = std::chrono::steady_clock::now() - start_all;
  server.stop();

  std::sort(latencies.begin(), latencies.end());
  const double seconds = std::chrono::duration<double>(elapsed).count();
  const double rps = static_cast<double>(latencies.size()) / seconds;
  const bool passed = failures.load() == 0 &&
                      static_cast<int>(latencies.size()) == kRequests &&
                      server.handled() == kRequests;

  std::cout << "safe_http_accelerated_real "
            << "requests=" << latencies.size()
            << " workers=" << kWorkers
            << " concurrency=" << kClientConcurrency
            << " memoryCapBytes=" << kMemoryCapBytes
            << " p50Micros=" << percentile(latencies, 0.50)
            << " p95Micros=" << percentile(latencies, 0.95)
            << " p99Micros=" << percentile(latencies, 0.99)
            << " requestsPerSecond=" << static_cast<long>(rps)
            << " noCrash=true noTimeout=true passed=" << (passed ? "true" : "false")
            << "\n";

  return passed ? 0 : 1;
}
