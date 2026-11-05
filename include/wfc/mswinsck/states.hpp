// The rules of the Winsock control that depend only on its state, apart from
// any socket, control or evaluator: which methods the state allows, and the
// Visual Basic error numbers of a refusal. Header only, with no dependency on
// the WFC evaluator or on Windows; the evaluator and the tests use it.
// Internal to the WFC libraries.

#ifndef WFC_MSWINSCK_STATES_HPP
#define WFC_MSWINSCK_STATES_HPP

namespace wfc::mswinsck {

// Winsock.State (sckClosed ... sckError).
enum class SocketState : int {
    closed = 0,
    open = 1,
    listening = 2,
    connection_pending = 3,
    connecting = 6,
    connected = 7,
    closing = 8,
    error = 9,
};

// How a request ended.
enum class Result {
    ok,
    invalid_operation,  // 40020: not allowed in the current state
    invalid_state,      // 40006: wrong protocol or state for the transaction
};

[[nodiscard]] constexpr long error_number(const Result result) {
    switch (result) {
        case Result::ok:
            return 0;
        case Result::invalid_operation:
            return 40020;
        case Result::invalid_state:
            return 40006;
    }
    return 0;
}

[[nodiscard]] constexpr const char* error_message(const Result result) {
    switch (result) {
        case Result::ok:
            return "";
        case Result::invalid_operation:
            return "Invalid operation at current state";
        case Result::invalid_state:
            return "Wrong protocol or connection state for the requested "
                   "transaction or request";
    }
    return "";
}

// Listen: from Closed or Open, and only for TCP.
[[nodiscard]] constexpr Result check_listen(const SocketState state,
                                            const bool udp) {
    if (state != SocketState::closed && state != SocketState::open) {
        return Result::invalid_operation;
    }
    return udp ? Result::invalid_state : Result::ok;
}

// Bind, Connect and Accept need a closed control; so does changing Protocol.
[[nodiscard]] constexpr Result check_closed(const SocketState state) {
    return state == SocketState::closed ? Result::ok
                                        : Result::invalid_operation;
}

// SendData on a stream needs a connection (a datagram needs none).
[[nodiscard]] constexpr Result check_send(const SocketState state,
                                          const bool udp) {
    return udp || state == SocketState::connected ? Result::ok
                                                  : Result::invalid_state;
}

// A socket that is not closed or in error keeps the host awake to poll it.
[[nodiscard]] constexpr bool keeps_host_awake(const SocketState state) {
    return state != SocketState::closed && state != SocketState::error;
}

}  // namespace wfc::mswinsck

#endif  // WFC_MSWINSCK_STATES_HPP
