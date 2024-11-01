/**
 * @file io.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Console-line input/output
 * @version 0.1
 * @date 2024-10-13
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

template <class T>
using Options = std::unordered_map<std::string, T>;

template <class T>
const T& receive_command(const Options<T>& options, const std::string& prefix,
                         const std::string& help) {
    while (1) {
        std::string input;
        std::cout << prefix;
        std::cin >> input;

        if (input == "help") {
            std::cout << help << std::endl;
            continue;
        }

        auto found = options.find(input);
        if (found == options.end()) {
            std::cout
                << "Invalid command, type \"help\" to see the list of options."
                << std::endl;
            continue;
        }

        return found->second;
    }
}
