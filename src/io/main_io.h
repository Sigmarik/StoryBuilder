/**
 * @file main_io.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief IO system for the main module
 * @version 0.1
 * @date 2023-07-25
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef MAIN_IO_H
#define MAIN_IO_H

#include <argp.h>

#include "src/config.h"

static const char ARGS_DOC[] = "";

enum OptCodeKey {
    _OPT_CUSTOM_KEYS_SHIFT = 500,
    OPT_OWL,
};

static const argp_option PARSER_OPTIONS[] = {
    {"owl", OPT_OWL, NULL, 0, "Lets the owls out"},
    {"server", 's', NULL, 0, "Runs the program in server mode"},
    {"udp", 'u', NULL, 0, "Forces the program to use UDP"},
    {}  // <-- NULL-terminator
};

struct Options {
    void enable_server() { server_ = true; }
    bool is_server() const { return server_; }

    bool is_udp() const { return udp_; }
    void use_udp() { udp_ = true; }

   private:
    bool server_ = false;
    bool udp_ = false;
};

/**
 * @brief Parse command line argument
 *
 * @param key argument key
 * @param arg argument
 * @param state parser state
 * @return 0 if parsing was successful, error code otherwise
 */
error_t parse(int key, char* arg, struct argp_state* state);

static const argp ARG_P = {.options = PARSER_OPTIONS,
                           .parser = parse,
                           .args_doc = ARGS_DOC,
                           .doc = PROGRAM_DESCRIPTION};

#endif
