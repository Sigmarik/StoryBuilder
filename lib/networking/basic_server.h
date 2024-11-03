/**
 * @file basic_server.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Generic TCP/UDP server
 * @version 0.1
 * @date 2024-11-03
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <assert.h>

#include <functional>
#include <map>
#include <thread>

#include "basic_interface.h"

struct NetworkClientInfo {
    int socket = 0;
    sockaddr_in address{};
};

template <NetworkProtocol Protocol>
struct NetworkServer : public NetworkConnection<Protocol> {
    NetworkServer(in_port_t port);
    ~NetworkServer();

    void start_accepting(in_port_t local_port);
    void check_new_connections();
    void stop_accepting();

    using ClientId = int;

    template <class T>
    bool send_to(ClientId client, const T& content) {
        if (!clients_.contains(client)) return {};

        NetworkConnection<Protocol>& client_conn = clients_[client];

        if (client_conn.is_dead()) {
            clients_.erase(client);
            return {};
        }

        return client_conn.template send<T>(content);
    }

    template <class T>
    std::optional<T> receive_from(ClientId client) {
        if (!clients_.contains(client)) return {};

        NetworkConnection<Protocol>& client_conn = clients_[client];

        if (client_conn.is_dead()) {
            clients_.erase(client);
            return {};
        }

        return client_conn.template receive<T>();
    }

    bool is_alive(ClientId client) const {
        return clients_.contains(client) && !clients_[client].is_dead();
    }

    void remove_dead() {
        for (auto& [client, client_conn] : clients_) {
            if (!client_conn.is_dead()) continue;

            clients_.erase(client);
        }
    }

   protected:
    virtual void on_client_connect(ClientId client,
                                   NetworkConnection<Protocol> connection) {}

   private:
    NetworkClientInfo accept_client();
    void setup_client(NetworkConnection<Protocol>& connection);

    std::map<ClientId, NetworkConnection<Protocol>> clients_{};

    std::jthread conn_listener_{};
    int local_sock_ = 0;

    NetworkConnection<Protocol> client_communicator_{};
};

template <NetworkProtocol Protocol>
inline void NetworkServer<Protocol>::start_accepting(in_port_t local_port) {
    auto listen_for_conns = [=, this]() {
        assert(errno != 0);

        int server = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(local_port);
        addr.sin_addr.s_addr = INADDR_LOOPBACK;

        bind(server, (sockaddr*)&addr, sizeof(addr));
        listen(server, 1);

        int loopback = accept(server, nullptr, nullptr);

        while (connect(loopback, (sockaddr*)&addr, sizeof(addr)) < 0);

        for (;;) {
            NetworkClientInfo client = accept_client();
            send(loopback, &client, sizeof(client), 0);

            if (errno != 0) {
                errno = 0;
                break;
            }
        }
    };

    conn_listener_ = std::jthread(listen_for_conns);

    local_sock_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(local_port);
    addr.sin_addr.s_addr = INADDR_LOOPBACK;

    while (connect(local_sock_, (sockaddr*)&addr, sizeof(addr)) < 0);
}

template <NetworkProtocol Protocol>
inline void NetworkServer<Protocol>::check_new_connections() {
    assert(errno != 0);

    while (true) {
        NetworkClientInfo client = 0;
        recv(local_sock_, &client, sizeof(client), 0);

        if (errno != 0) break;

        int client_id = client.socket;

        clients_[client_id] = NetworkConnection<Protocol>();
        NetworkConnection<Protocol> client_conn = clients_[client_id];
        client_conn.sock_ = client.socket;
        client_conn.conn_addr_ = client.address;

        setup_client(client_conn);

        on_client_connect(client, clients_[client_id]);
    }
}

template <NetworkProtocol Protocol>
inline void NetworkServer<Protocol>::stop_accepting() {
    conn_listener_.request_stop();
    close(local_sock_);
    local_sock_ = 0;
}
