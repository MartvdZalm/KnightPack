#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>


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


void processArguments(const std::unordered_map<std::string, std::string> &flags)
{
	if (flags.find("--name") != flags.end()) {
		std::cout << "Searching for package by name: " << flags.at("--name") << std::endl;
		std::cout << exec(("dpkg -l | grep " + flags.at("--name")).c_str()) << std::endl;
	}
}


int main(int argc, char* argv[])
{
	std::unordered_map<std::string, std::string> flags;

	for (int i = 0; i < argc; ++i) {
		std::string arg = argv[i];

		if (arg.starts_with("--")) {
			if (i + 1 < argc && !std::string(argv[i + 1]).starts_with("--")) {
				flags[arg] = argv[i + 1];
				++i;
			} else {
				flags[arg] = "";
			}
		}
	}

	processArguments(flags);

    return 0;
}
