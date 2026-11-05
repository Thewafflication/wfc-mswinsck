// Checks the state rules of the Winsock control (wfc/mswinsck/states.hpp).

#include <wfc/mswinsck/states.hpp>

#include <cstdio>
#include <string>

namespace {

int failures = 0;

void expect(const bool condition, const char* what) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", what);
        ++failures;
    }
}

}  // namespace

int main() {
    using namespace wfc::mswinsck;
    using S = SocketState;

    expect(error_number(Result::invalid_operation) == 40020 &&
               error_number(Result::invalid_state) == 40006 &&
               error_number(Result::ok) == 0,
           "the error numbers");
    expect(std::string(error_message(Result::invalid_operation)) ==
                   "Invalid operation at current state" &&
               std::string(error_message(Result::invalid_state))
                       .rfind("Wrong protocol or connection state", 0) == 0,
           "the messages");

    expect(check_listen(S::closed, false) == Result::ok &&
               check_listen(S::open, false) == Result::ok &&
               check_listen(S::listening, false) == Result::invalid_operation &&
               check_listen(S::connected, false) == Result::invalid_operation,
           "Listen from Closed or Open");
    expect(check_listen(S::closed, true) == Result::invalid_state &&
               check_listen(S::connected, true) == Result::invalid_operation,
           "Listen is for TCP; the state is checked first");

    expect(check_closed(S::closed) == Result::ok &&
               check_closed(S::open) == Result::invalid_operation &&
               check_closed(S::connecting) == Result::invalid_operation,
           "Bind, Connect, Accept and Protocol need Closed");

    expect(check_send(S::connected, false) == Result::ok &&
               check_send(S::closed, false) == Result::invalid_state &&
               check_send(S::closed, true) == Result::ok &&
               check_send(S::open, true) == Result::ok,
           "SendData needs a connection, a datagram does not");

    expect(!keeps_host_awake(S::closed) && !keeps_host_awake(S::error) &&
               keeps_host_awake(S::open) && keeps_host_awake(S::connecting) &&
               keeps_host_awake(S::listening),
           "an open socket keeps the host awake");

    if (failures == 0) {
        std::puts("states: ok");
    }
    return failures == 0 ? 0 : 1;
}
