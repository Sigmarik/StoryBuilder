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
#include "networking/basic_server.h"
template <NetworkProtocol Protocol>
struct GameServer : public NetworkServer<Protocol> {
    GameServer();

    void accept_players();

    void start_round();

    void gather_replies();

    void reveal_story();

    void list_players() const;

    using PlayerId = NetworkServer<Protocol>::ClientId;

   protected:
    virtual void on_client_connect(NetworkServer<Protocol>::
                                       ClientId client) override {
        auto name = receive_from<std::string>(client);
        if (!name) return;

        players_[client] = *name;
    }

    virtual void on_client_disconnect(NetworkServer<Protocol>::
                                          ClientId client) override {
        if (players_.contains(client)) return;

        players_.erase(client);
    }

   private:
    std::vector<std::string> story_{};
    std::map<PlayerId, std::string> players_{};
};

template <NetworkProtocol Protocol>
int as_server() {
    GameServer<Protocol> server;

    server.accept_players();

    server.start_round();

    server.gather_replies();

    server.reveal_story();

    return EXIT_SUCCESS;
}

template int as_server<NetworkProtocol::TCP>();

template int as_server<NetworkProtocol::UDP>();

template <NetworkProtocol Protocol>
GameServer<Protocol>::GameServer() : NetworkServer<Protocol>(CONN_PORT) {}

template <NetworkProtocol Protocol>
void GameServer<Protocol>::accept_players() {
    GameServer<Protocol>::start_accepting(8888 + (uint16_t)rand() % 100);

    std::cout << "Server is accepting players. (start / help)" << std::endl;

    receive_command((Options<int>){{"start", 0}}, INPUT_PREFIX,
                    "help - show this message,\n"
                    "start - start the game,\n"
                    "participants - show the list of participants.");

    GameServer<Protocol>::check_new_connections();

    GameServer<Protocol>::stop_accepting();
}

static const std::vector<std::string> OBJECTIVES{
    "Stinky", "Humble",   "Brave",    "Golden", "Stupid",      "Shy",
    "Naive",  "Northern", "Southern", "Polar",  "Adventurous", "Fat",
    "Skinny", "Strong",   "Weak",     "Smart",  "Dumb",        "Controversial",
};

static const std::vector<std::string> NOUNS{
    "goose",     "dwarf",      "elf",       "boy",        "girl",
    "man",       "polar bear", "archivist", "programmer", "wizard",
    "barbarian", "troll",      "engineer",  "mechanic",   "pilot",
    "sailor",    "driver",     "artificer", "artist",     "dancer",
};

template <NetworkProtocol Protocol>
void GameServer<Protocol>::start_round() {
    story_.clear();

    story_.push_back(OBJECTIVES[(size_t)rand() % OBJECTIVES.size()]);
    story_.push_back(NOUNS[(size_t)rand() % NOUNS.size()]);
}

template <NetworkProtocol Protocol>
void GameServer<Protocol>::gather_replies() {
    for (auto& [player_id, player_name] : players_) {
        send_to<std::string>(player_id, story_[story_.size() - 2]);
        send_to<std::string>(player_id, story_[story_.size() - 1]);

        auto reply = receive_from<std::string>(player_id);

        if (!reply) continue;

        printf("%s's addition: %s\n", player_name.c_str(), reply->c_str());

        story_.push_back(*reply);
    }
}

template <NetworkProtocol Protocol>
void GameServer<Protocol>::reveal_story() {
    for (auto& [player_id, player_name] : players_) {
        send_to<uint32_t>(player_id, (uint32_t)story_.size());

        for (const std::string& part : story_) {
            send_to<std::string>(player_id, part);
        }
    }
}

template <NetworkProtocol Protocol>
void GameServer<Protocol>::list_players() const {
    printf("Players:\n");

    for (auto& [player_id, player_name] : players_) {
        printf("\t%s\n", player_name.c_str());
    }
}
