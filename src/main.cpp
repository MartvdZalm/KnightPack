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
#include <chrono>
#include <ctime>
#include <sys/utsname.h>


namespace Color
{
    const std::string RESET = "\033[0m";
    const std::string GREEN = "\033[32m";
    const std::string RED = "\033[31m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string CYAN = "\033[36m";
}

std::string getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm* timeInfo = std::localtime(&currentTime);
    
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);
    
    return std::string(buffer);
}

void logCommand(const std::string& command)
{
    std::ofstream logFile("command_log.txt", std::ios_base::app);
    if (logFile.is_open()) {
        logFile << getCurrentTime() << " - " << command << std::endl;
    } else {
        std::cerr << "Failed to open log file." << std::endl;
    }
}

std::string exec(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), fclose);

    if (!pipe) {
        throw std::runtime_error("Failed to execute command: " + std::string(cmd));
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    if (result.empty()) {
        std::cerr << "Command failed or returned empty output." << std::endl;
    }

    logCommand(std::string(cmd));

    return result;
}

class PackageManager
{
public:
    static std::string get()
    {
        static std::string packageManager = detectPackageManager();
        return packageManager;
    }

private:
    static std::string detectPackageManager()
    {
        std::string os = detectOS();

        if (os == "macOS") {
            std::string result = exec("which brew");
            if (!result.empty()) {
                return "brew";
            }
        }

        std::string result = exec("which apt");
        if (!result.empty()) {
            return "apt";
        }

        result = exec("which yum");
        if (!result.empty()) {
            return "yum";
        }

        result = exec("which pacman");
        if (!result.empty()) {
            return "pacman";
        }

        return "unknown";
    }

    static std::string detectOS()
    {
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            std::string osName = buffer.sysname;
            if (osName == "Darwin") {
                return "macOS";
            } else {
                return "Linux";
            }
        }

        return "unknown";
    }
};

void printSuccess(const std::string& message)
{
    std::cout << Color::GREEN << message << Color::RESET << std::endl;
}

void printError(const std::string& message)
{
    std::cout << Color::RED << message << Color::RESET << std::endl;
}

void printWarning(const std::string& message)
{
    std::cout << Color::YELLOW << message << Color::RESET << std::endl; 
}

void printInfo(const std::string& message)
{
    std::cout << Color::CYAN << message << Color::RESET << std::endl;
}

void updatePackage(const std::string& packageName)
{
    std::string cmd;
    std::string packageManager = PackageManager::get();

    if (packageManager == "brew") {
        cmd = "brew upgrade " + packageName;
    } else if (packageManager == "apt") {
        cmd = "sudo apt update && sudo apt upgrade -y " + packageName;
    } else if (packageManager == "yum") {
        cmd = "sudo yum update " + packageName;
    } else if (packageManager == "pacman") {
        cmd = "sudo pacman -Syu " + packageName;
    } else {
        printError("Unsupported package manager!");
        return;
    }

    std::cout << exec(cmd.c_str()) << std::endl;
}

void installPackage(const std::string& packageName)
{
    std::string cmd;
    std::string packageManager = PackageManager::get();

    if (packageManager == "brew") {
        cmd = "brew install " + packageName;
    } else if (packageManager == "apt") {
        cmd = "sudo apt install -y " + packageName;
    } else if (packageManager == "yum") {
        cmd = "sudo yum install -y " + packageName;
    } else if (packageManager == "pacman") {
        cmd = "sudo pacman -S " + packageName;
    } else {
        std::cerr << "Unsupported package manager!" << std::endl;
        return;
    }

    std::cout << exec(cmd.c_str()) << std::endl;
}

void removePackage(const std::string& packageName)
{
    std::string cmd;
    std::string packageManager = PackageManager::get();
    
    if (packageManager == "brew") {
        cmd = "brew uninstall " + packageName;
    } else if (packageManager == "apt") {
        cmd = "sudo apt remove -y " + packageName;
    } else if (packageManager == "yum") {
        cmd = "sudo yum remove -y " + packageName;
    } else if (packageManager == "pacman") {
        cmd = "sudo pacman -R " + packageName;
    } else {
        std::cerr << "Unsupported package manager!" << std::endl;
        return;
    }

    std::cout << exec(cmd.c_str()) << std::endl;
}

void listInstalledPackages()
{
    std::string result;
    std::string packageManager = PackageManager::get();

    if (packageManager == "brew") {
        result = exec("brew list");
    } else if (packageManager == "apt") {
        result = exec("dpkg -l");
    } else if (packageManager == "yum") {
        result = exec("yum list installed");
    } else if (packageManager == "pacman") {
        result = exec("pacman -Q");
    }

    std::cout << result << std::endl;
}

void displayHelp(const nlohmann::json& flagsJson)
{
    std::cout << Color::BLUE <<  "Package Manager Help" << Color::RESET << std::endl;
    std::cout << "Usage: [command] [options]\n" << std::endl;
    std::cout << "Available commands: " << std::endl;
    for (const auto& flag : flagsJson["flags"]) {
        if (flag.contains("short")) {
            std::cout << "  " << flag["short"] << ", " << flag["flag"] << " : " << flag["description"] << std::endl;
        } else {
            std::cout << "  " << flag["flag"] << " : " << flag["description"] << std::endl;
        }
    }
}

void showPackageDetails(const std::string& packageName)
{
    std::string result;
    std::string packageManager = PackageManager::get();

    if (packageManager == "brew") {
        result = exec(("brew info " + packageName).c_str());
    } else if (packageManager == "apt") {
        result = exec(("apt-cache show " + packageName).c_str());
    } else if (packageManager == "yum") {
        result = exec(("yum info " + packageName).c_str());
    } else if (packageManager == "pacman") {
        result = exec(("pacman -Qi " + packageName).c_str());
    }

    std::cout << "Package details for " << packageName << ":\n" << result << std::endl;
}

void showPackagePaths(const std::string& packageName)
{
    std::string result;
    std::string packageManager = PackageManager::get();

    if (packageManager == "brew") {
        result = exec(("brew list --verbose " + packageName).c_str());
    } else if (packageManager == "apt") {
        result = exec(("dpkg -L " + packageName).c_str());
    } else if (packageManager == "yum") {
        result = exec(("rpm -ql " + packageName).c_str());
    } else if (packageManager == "pacman") {
        result = exec(("pacman -Ql " + packageName).c_str());
    } else {
        printError("Unsupported package manager: " + packageManager);
        return;
    }

    std::cout << "Package paths for " << packageName << ":\n" << result << std::endl;
}

void displayHistory()
{
    std::ifstream file("command_log.txt");

    if (!file.is_open()) {
        std::cerr << "Failed to open command_log.txt" << std::endl;
        return;
    }

    char ch;
    while (file.get(ch)) {
        std::cout << ch;
    }

    file.close();
}

void clearHistory()
{
    std::ofstream file("command_log.txt", std::ios::trunc);

    if (!file.is_open()) {
        std::cerr << "Failed to open command_log.txt" << std::endl;
        return;
    }

    std::cout << "File content removed successfully!" << std::endl;

    file.close();
}

void processArguments(const std::unordered_map<std::string, std::string>& flags)
{
    if (flags.find("search_by_name") != flags.end()) {
        const std::string& searchTerm = flags.at("search_by_name");
        if (searchTerm.empty()) {
            printError("Missing search term for 'search_by_name' flag.");
            return;
        }

        std::cout << "Searching for package by name: " << flags.at("search_by_name") << std::endl;
        std::cout << exec(("dpkg -l | grep " + flags.at("search_by_name")).c_str()) << std::endl;
    }

    if (flags.find("version") != flags.end()) {
        std::cout << "Version 1.0.0" << std::endl;
    }

    if (flags.find("show_package_paths") != flags.end()) {
        const std::string& packageName = flags.at("show_package_paths");
        if (packageName.empty()) {
            printError("Missing package name for 'show_package_paths' flag.");
            return;
        } 
        std::cout << "Showing paths for package: " << flags.at("show_package_paths") << std::endl;
        showPackagePaths(flags.at("show_package_paths"));
    }

    if (flags.find("install") != flags.end()) {
        const std::string& packageName = flags.at("install");
        if (packageName.empty()) {
            printError("Missing package name for 'install' flag.");
            return;
        }
        installPackage(flags.at("install"));
    }

    if (flags.find("remove") != flags.end()) {
        const std::string& packageName = flags.at("remove");
        if (packageName.empty()) {
            printError("Missing package name for 'remove' flag.");
            return;
        }
        removePackage(flags.at("remove"));
    }

    if (flags.find("list_installed") != flags.end()) {
        listInstalledPackages();
    }

    if (flags.find("show_package_details") != flags.end()) {
        const std::string& packageName = flags.at("show_package_details");
        if (packageName.empty()) {
            printError("Missing package name for 'show_package_details' flag.");
            return;
        }
        showPackageDetails(flags.at("show_package_details"));
    }

    if (flags.find("display_history") != flags.end()) {
        displayHistory();
    }

    if (flags.find("clear_history") != flags.end()) {
        clearHistory();
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
            if (arg == flag["flag"] || (flag.contains("short") && arg == flag["short"])) {
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
