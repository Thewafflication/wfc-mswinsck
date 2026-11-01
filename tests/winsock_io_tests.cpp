// Loopback checks of the socket layer behind the Winsock control: a TCP
// connection (listen, connect, accept, send, receive, peer, close) and a UDP
// datagram, all on 127.0.0.1 with ports the system picks.

#include <wfc/mswinsck/winsock_io.hpp>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>

namespace {

namespace net = wfc::net;

int failures = 0;

void expect(const bool condition, const char* what) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", what);
        ++failures;
    }
}

// Runs `step` until it returns true or about five seconds pass.
template <typename Step>
bool wait_for(Step step) {
    for (int attempt = 0; attempt < 500; ++attempt) {
        if (step()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return false;
}

void tcp_round_trip() {
    int error = 0;
    const net::Handle server = net::open(false, error);
    expect(server != net::kInvalid, "open a TCP socket");
    expect(net::bind(server, "127.0.0.1", 0, error), "bind the server");
    expect(net::listen(server, error), "listen");
    const int port = net::local_port(server);
    expect(port > 0, "the server has a port");

    const net::Handle client = net::open(false, error);
    std::string address;
    net::Progress progress =
        net::connect(client, "127.0.0.1", port, address, error);
    expect(progress != net::Progress::Failed, "start the connection");
    expect(address == "127.0.0.1", "the resolved address");
    expect(wait_for([&] {
               progress = net::connect_status(client, error);
               return progress != net::Progress::Pending;
           }) &&
               progress == net::Progress::Done,
           "the connection completes");

    net::Handle accepted = net::kInvalid;
    expect(wait_for([&] {
               accepted = net::accept(server, error);
               return accepted != net::kInvalid;
           }),
           "the server accepts the connection");

    std::string peer_address;
    int peer_port = 0;
    expect(net::peer(accepted, peer_address, peer_port) &&
               peer_address == "127.0.0.1",
           "the peer of the accepted socket");

    expect(net::send(client, "hello", error), "send");
    std::string received;
    expect(wait_for([&] {
               return net::receive(accepted, received, error) > 0 &&
                      received.size() >= 5U;
           }) &&
               received == "hello",
           "the server receives what was sent");

    net::close(client);
    long closed = -1;
    expect(wait_for([&] {
               std::string rest;
               closed = net::receive(accepted, rest, error);
               return closed == 0;
           }),
           "the server sees the connection close");

    net::close(accepted);
    net::close(server);
}

void udp_round_trip() {
    int error = 0;
    const net::Handle left = net::open(true, error);
    const net::Handle right = net::open(true, error);
    expect(left != net::kInvalid && right != net::kInvalid, "open UDP sockets");
    expect(net::bind(left, "127.0.0.1", 0, error), "bind the first socket");
    expect(net::bind(right, "127.0.0.1", 0, error), "bind the second socket");
    expect(net::send_to(left, "datagram", "127.0.0.1", net::local_port(right),
                        error),
           "send a datagram");
    std::string data;
    std::string from;
    int from_port = 0;
    expect(wait_for([&] {
               return net::receive_from(right, data, from, from_port, error) >
                      0;
           }) &&
               data == "datagram",
           "the datagram arrives");
    expect(from == "127.0.0.1" && from_port == net::local_port(left),
           "the datagram names its sender");
    net::close(left);
    net::close(right);
}

void names() {
    expect(!net::host_name().empty(), "a host name");
    expect(!net::error_text(10061).empty(), "the text of a socket error");
}

}  // namespace

int main() {
    tcp_round_trip();
    udp_round_trip();
    names();
    if (failures == 0) {
        std::puts("winsock io: tcp, udp and names are right");
    }
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
