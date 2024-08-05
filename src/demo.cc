#include <iostream>
#include <string>
#include <unistd.h>
#include "smsaero.hh"

int main(const int argc, char *const argv[]) {
    int cmd;
    std::string user_email;
    std::string auth_token;
    std::string message;
    std::string to_number;

    opterr = 0;

    while ((cmd = getopt(argc, argv, "e:t:n:m:h?")) != -1) {
        switch (cmd) {
            case '?':
            case 'h':
                std::cout << "Help:" << std::endl;
                std::cout << "-e: Email\t\t(ex: -e 'user@local.host')" << std::endl;
                std::cout << "-t: Auth Token\t\t(ex: -t 'your_token')" << std::endl;
                std::cout << "-n: To Number(s)\t(ex: -n 70000000000,70000000001)" << std::endl;
                std::cout << "-m: Message to send\t(ex: -m 'Hello, World!')" << std::endl;
                std::cout << "-h: This help dialog" << std::endl;
                return 0;
            case 'e':
                user_email = optarg;
                break;
            case 't':
                auth_token = optarg;
                break;
            case 'n':
                to_number = optarg;
                break;
            case 'm':
                message = optarg;
                break;
            default:
                std::cerr << "Invalid option provided. Call using -h for help." << std::endl;
                return -1;
        }
    }

    if (user_email.empty() || auth_token.empty() || to_number.empty() || message.empty()) {
        std::cerr << "Missing arguments. Call using -h for help." << std::endl;
        return -1;
    }

    try {
        smsaero::SmsAero smsAero(user_email, auth_token);
        const smsaero::json result = smsAero.send_sms(to_number, message);
        std::cout << result << std::endl;
    } catch (const smsaero::SmsAeroError &e) {
        std::cerr << "SmsAero error: " << e.what() << std::endl;
        return -1;
    } catch (const std::exception &e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Caught unknown exception" << std::endl;
        return -1;
    }
    return 0;
}
