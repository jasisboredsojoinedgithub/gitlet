#include "Repo.h"
#include <iostream>
#include <string>
#include <vector>

bool inputChecker(int expectedLength, const std::vector<std::string>& args) {
    if (args.size() == expectedLength) {
        return true;
    }
    std::cout << "Incorrect Operands" << std::endl;
    return false;
}

int main(int argc, char* argv[]) {
    Repo r;
    std::vector<std::string> args(argv + 1, argv + argc);
    
    if (args.empty()) {
        std::cout << "Please enter a command." << std::endl;
    } else {
        std::string command = args[0];

        if (command == "init") {
            if (inputChecker(1, args)) {
                r.init();
            }
        } else if (command == "add") {
            if (inputChecker(2, args)) {
                r.add(args[1]);
            }
        } else if (command == "commit") {
            if (inputChecker(2, args)) {
                r.commitment(args[1]);
            }
        } else if (command == "rm") {
            if (inputChecker(2, args)) {
                r.rm(args[1]);
            }
        } else if (command == "log") {
            if (inputChecker(1, args)) {
                r.log();
            }
        } else if (command == "global-log") {
        if (inputChecker(1, args)) {
            r.global();
        }
    } else if (command == "find") {
        if (inputChecker(2, args)) {
            r.find(args[1]);
        }
    } else if (command == "status") {
        if (inputChecker(1, args)) {
            r.status();
        }
    } else if (command == "checkout") {
        if (args.size() == 2) {
            r.checkout({args.begin() + 1, args.end()});
        } else if (args.size() == 3 && args[1] == "--") {
            std::vector<std::string> checkoutArgs = {args[1], args[2]};
            r.checkout(checkoutArgs);
        } else if (args.size() == 4 && args[2] == "--") {
            std::vector<std::string> checkoutArgs = {args[1], args[2], args[3]};
            r.checkout(checkoutArgs);
        } else {
            std::cout << "Incorrect Operands" << std::endl;
        }
    } else if (command == "branch") {
        if (inputChecker(2, args)) {
            r.branch(args[1]);
        }
    } else if (command == "rm-branch") {
        if (inputChecker(2, args)) {
            r.rmb(args[1]);
        }
    } else if (command == "reset") {
        if (inputChecker(2, args)) {
            r.reset(args[1]);
        }
    } else if (command == "merge") {
        if (inputChecker(2, args)) {
            r.merge(args[1]);
        }
    } else {
        std::cout << "No command with that name exists." << std::endl;
    }
}
return 0;
}