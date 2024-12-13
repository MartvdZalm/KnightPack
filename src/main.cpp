#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

std::string exec(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), fclose);

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

void displayHelp(const nlohmann::json& flagsJson)
{
    std::cout << "Available commands: " << std::endl;
    for (const auto& flag : flagsJson["flags"]) {
        std::cout << "  " << flag["short"] << ", " << flag["flag"] << " : " << flag["description"] << std::endl;
    }
}

void processArguments(const std::unordered_map<std::string, std::string>& flags)
{
    if (flags.find("search_by_name") != flags.end()) {
        std::cout << "Searching for package by name: " << flags.at("search_by_name") << std::endl;
        std::cout << exec(("dpkg -l | grep " + flags.at("search_by_name")).c_str()) << std::endl;
    }

    if (flags.find("version") != flags.end()) {
        std::cout << "Version 1.0.0" << std::endl;
    }
}

int main(int argc, char* argv[])
{
    std::ifstream flagsFile("../src/flags.json");
    if (!flagsFile.is_open()) {
        std::cerr << "Failed to open flags.json" << std::endl;
        return 1;
    }
    
    nlohmann::json flagsJson;
    flagsFile >> flagsJson;

    std::unordered_map<std::string, std::string> flags;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help") {
            displayHelp(flagsJson);
            return 0;
        }

        bool flagHandled = false;
        for (const auto& flag : flagsJson["flags"]) {
            if (arg == flag["flag"] || arg == flag["short"]) {
                if (i + 1 < argc && !std::string(argv[i + 1]).starts_with("-")) {
                    flags[flag["name"]] = argv[i + 1];
                    ++i;
                } else {
                    flags[flag["name"]] = "";
                }
                flagHandled = true;
                break;
            }
        }

        if (!flagHandled) {
            std::cerr << "Unknown flag: " << arg << std::endl;
            return 1;
        }
    }

    if (argc <= 1) {
        displayHelp(flagsJson);
    }

    processArguments(flags);

    return 0;
}
