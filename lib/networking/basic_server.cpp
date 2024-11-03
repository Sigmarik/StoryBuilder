#include "basic_server.h"

template <>
inline NetworkServer<NetworkProtocol::TCP>::NetworkServer(in_port_t port) {
    sock_ = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sock_, (sockaddr *)&addr, sizeof(addr));

    listen(sock_, 16);
}

template <>
inline NetworkServer<NetworkProtocol::UDP>::NetworkServer(in_port_t port) {
    sock_ = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sock_, (sockaddr *)&addr, sizeof(addr));

    client_communicator_.sock_ = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in &client_comm = client_communicator_.conn_addr_;

    client_comm.sin_family = AF_INET;
    client_comm.sin_port = 0;
    client_comm.sin_addr.s_addr = INADDR_ANY;

    bind(client_communicator_.sock_, (sockaddr *)&client_comm,
         sizeof(client_comm));
}

template <>
NetworkServer<NetworkProtocol::UDP>::~NetworkServer() {
    close(client_communicator_.sock_);
}

template <>
NetworkServer<NetworkProtocol::TCP>::~NetworkServer() {}

template <>
inline NetworkClientInfo NetworkServer<NetworkProtocol::TCP>::accept_client() {
    return (NetworkClientInfo){
        .socket = accept(sock_, nullptr, nullptr),
        .address = {},
    };
}

template <>
void NetworkServer<NetworkProtocol::TCP>::
    setup_client(NetworkConnection<NetworkProtocol::TCP> &connection) {}

template <>
void NetworkServer<NetworkProtocol::UDP>::
    setup_client(NetworkConnection<NetworkProtocol::UDP> &connection) {
    connection.set_close_on_destroy(false);
}

template <>
inline NetworkClientInfo NetworkServer<NetworkProtocol::UDP>::accept_client() {
    NetworkClientInfo client{};

    receive<uint16_t>();

    client.socket = client_communicator_.sock_;
    client.address = conn_addr_;

    send<uint16_t>(client_communicator_.conn_addr_.sin_port);

    conn_addr_ = (sockaddr_in){};

    return client;
}
