#include "server.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <future>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <vector>

#include "config.h"
#include "console/io.h"
#include "logger/debug.h"
#include "logger/logger.h"

static int set_noblock(int sock) {
    return fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
}

struct Server {
    using ClientId = std::string;

    Server();
    ~Server();

    void fetch_clients();

    bool iterate();

   private:
    void list() const;
    void print_scoreboard() const;
    void answer();

    void fetch_answers();

    ClientId read_client_id();

    // TODO: void disconnect(const ClientId& client);

    int sock_ = 0;

    // TODO: Encapsulate client-related fields into a structure
    std::unordered_map<ClientId, int> clients_{};
    std::unordered_map<ClientId, unsigned> scoreboard_{};
    std::unordered_map<ClientId, std::optional<int>> last_answer_{};
};

int as_server() {
    Server server;

    receive_command((Options<int>){{"start", 1}}, INPUT_PREFIX,
                    "help - see this list,\nstart - start the competition");

    server.fetch_clients();

    while (server.iterate());

    return EXIT_SUCCESS;
}

Server::Server() {
    sock_ = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;

    server_address.sin_port = htons(CONN_PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    bind(sock_, (sockaddr*)&server_address, sizeof(server_address));

    set_noblock(sock_);

    listen(sock_, MAX_CLIENT_COUNT);
}

Server::~Server() { close(sock_); }

void Server::fetch_clients() {
    for (;;) {
        int client_sock = accept(sock_, nullptr, nullptr);
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            errno = 0;
            break;
        }

        std::string client_id = std::to_string(clients_.size());
        clients_[client_id] = client_sock;
        scoreboard_[client_id] = 0;
        last_answer_[client_id] = {};
    }
}

bool Server::iterate() {
    int command = receive_command(
        (Options<int>){
            {"stop", 0},
            {"list", 1},
            {"answer", 2},
            {"board", 3},
        },
        INPUT_PREFIX,
        "help   - see this list,\n"
        "list   - list participants waiting for an answer,\n"
        "answer - answer the participant,\n"
        "board  - show scoreboard,\n"
        "stop   - stop the competition.");

    switch (command) {
        case 0: {
            return false;
        } break;
        case 1: {
            fetch_answers();
            list();
        } break;
        case 2: {
            answer();
        } break;
        case 3: {
            print_scoreboard();
        }
        default:
            break;
    }

    return true;
}

void Server::list() const {
    std::unordered_map<ClientId, int> answers;

    for (auto& [key, value] : last_answer_) {
        if (value) {
            answers[key] = *value;
        }
    }

    if (answers.size() == 0) {
        std::cout << "There are no active requests." << std::endl;
    } else {
        std::cout << "There are " << answers.size() << "requests:\n";
        for (auto& [key, value] : answers) {
            std::cout << "\tFrom " << key << ":\t" << value << "\n";
        }
    }
}

void Server::print_scoreboard() const {
    // TODO: Implement
}

void Server::answer() {
    fetch_answers();
    ClientId id = read_client_id();

    if (!last_answer_[id]) {
        std::cout << "This client does not have a request that can be "
                     "answered."
                  << std::endl;
        return;
    } else {
        std::cout << "Last number suggested by " << id << " is "
                  << *last_answer_[id];
    }

    std::cout << "Is client's number higher or lower than the answer? "
                 "(higher / lower / equal)"
              << std::endl;

    int8_t suggestion = receive_command(
        (Options<int8_t>){
            {"higher", 1},
            {"lower", -1},
            {"equal", 0},
        },
        INPUT_PREFIX,
        "help   - see this list,\n"
        "higher - participant's guess is higher than the answer,\n"
        "lower  - participant's guess is lower than the answer,\n"
        "equal  - participant's guess matches the answer.");

    ssize_t status = write(clients_[id], &suggestion, sizeof(suggestion));
    if (status < 0) {
        // TODO: disconnect(key);
        clients_.erase(id);
        last_answer_.erase(id);
        scoreboard_.erase(id);
    }
}

void Server::fetch_answers() {
    static int buffer[MAX_PACKAGE_SIZE] = {};
    for (auto& [key, value] : clients_) {
        ssize_t length = recv(value, buffer, sizeof(*buffer) * MAX_PACKAGE_SIZE,
                              MSG_DONTWAIT);
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            errno = 0;
            continue;
        } else if (length < 0) {
            // TODO: disconnect(key);
            clients_.erase(key);
            last_answer_.erase(key);
            scoreboard_.erase(key);
        }

        last_answer_[key] = value;
        scoreboard_[key]++;
    }
}

Server::ClientId Server::read_client_id() {
    ClientId id;

    for (;;) {
        std::cout << INPUT_PREFIX;
        std::cin >> id;
        if (clients_.count(id) > 0)
            break;
        else {
            std::cout << "Client with the specified ID does not exist."
                      << std::endl;
        }
    }

    return id;
}
