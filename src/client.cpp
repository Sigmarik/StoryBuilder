#include "client.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "config.h"
#include "logger/debug.h"
#include "logger/logger.h"
#include "networking/basic_client.h"

static in_addr_t get_address();

template <NetworkProtocol Protocol>
struct GameClient : public NetworkClient<Protocol> {
    GameClient();

    void provide_credentials(const std::string& name);

    void make_turn();

    void display_story();
};

template <NetworkProtocol Protocol>
int as_client() {
    std::string name;
    std::cout << "Your display name:" << std::endl << INPUT_PREFIX;
    std::cin >> name;

    GameClient<Protocol> client;

    client.provide_credentials(name);

    std::cout << "Waiting for other players..." << std::endl;

    client.make_turn();

    client.display_story();

    return EXIT_SUCCESS;
}

template int as_client<NetworkProtocol::TCP>();

template int as_client<NetworkProtocol::UDP>();

static in_addr_t get_address() {
    in_addr_t address = 0;

    do {
        std::string server_addr_string;

        std::cout << "Server address:\n" << INPUT_PREFIX;
        std::cin >> server_addr_string;

        address = inet_addr(server_addr_string.c_str());
        if (address == (in_addr_t)(-1)) {
            address = 0;
            std::cout << "Cannot resolve IPv4 address from the input provided. "
                         "Please use standard IPv4 notation (P1.P2.P3.P4)."
                      << std::endl;
        } else if (address == 0) {
            std::cout << "Server address cannot be 0.0.0.0." << std::endl;
        }
    } while (address == 0);

    return address;
}

template <NetworkProtocol Protocol>
GameClient<Protocol>::GameClient()
    : NetworkClient<Protocol>(get_address(), CONN_PORT) {}

template <NetworkProtocol Protocol>
void GameClient<Protocol>::provide_credentials(const std::string& name) {
    GameClient<Protocol>::send(name);
}

template <NetworkProtocol Protocol>
void GameClient<Protocol>::make_turn() {
    auto first_word = GameClient<Protocol>::template receive<std::string>();
    if (!first_word) return;

    auto second_word = GameClient<Protocol>::template receive<std::string>();
    if (!second_word) return;

    std::cout << "Story prefix:\n"
              << *first_word << " " << *second_word << std::endl
              << INPUT_PREFIX;

    std::string reply;

    std::cin >> reply;

    GameClient<Protocol>::send(reply);
}

template <NetworkProtocol Protocol>
void GameClient<Protocol>::display_story() {
    auto length = GameClient<Protocol>::template receive<uint32_t>();
    if (!length) return;

    std::cout << "Final story:" << std::endl;

    for (size_t part_id = 0; part_id < *length; ++part_id) {
        auto part = GameClient<Protocol>::template receive<std::string>();

        std::cout << *part << " ";
    }

    std::cout << std::endl;
}
