#include "basic_client.h"

template <>
inline NetworkClient<NetworkProtocol::TCP>::
    NetworkClient(in_addr_t server_addr, in_port_t port) {
    sock_ = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_ < 0) {
        die();
        return;
    }

    conn_addr_.sin_family = AF_INET;
    conn_addr_.sin_port = htons(port);
    conn_addr_.sin_addr.s_addr = server_addr;

    int status = connect(sock_, (sockaddr*)&conn_addr_, sizeof(conn_addr_));
    if (status < 0) die();
}

template <>
inline NetworkClient<NetworkProtocol::UDP>::
    NetworkClient(in_addr_t server_addr, in_port_t port) {
    sock_ = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock_ < 0) {
        die();
        return;
    }

    conn_addr_.sin_family = AF_INET;
    conn_addr_.sin_port = htons(port);
    conn_addr_.sin_addr.s_addr = server_addr;

    if (!send<uint16_t>(0)) {
        die();
        return;
    }

    auto new_port = receive<uint16_t>();
    if (!new_port) {
        die();
        return;
    }

    conn_addr_.sin_port = htons(*new_port);
}
