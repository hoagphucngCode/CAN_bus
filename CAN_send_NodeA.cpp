#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <ios>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>

void printUsage() {
    std::cerr << "Usage: CAN_send <CAN_ID> <data>\n";
    std::cerr << "CAN_ID: 0-0x7FF (11-bit)\n";
    std::cerr << "Data: Hexadecimal string of up to 8 bytes\n";
}

int main() {
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;

    // Create socket CAN
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        perror("Socket");
        return 1;
    }

    // Configure interface CAN
    strcpy(ifr.ifr_name, "vcan0");
    ioctl(s, SIOCGIFINDEX, &ifr);

    // Assigne address for socket CAN
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind");
        return 1;
    }

    // Create message CAN
    struct can_frame frame;
    std::string userInput;
    std::vector<can_frame> frames;

    while (true) {
        // Input ID
        std::cout << "Enter CAN ID (decimal or hexadecimal): ";
        std::getline(std::cin, userInput);

        // Handle CAN ID
        unsigned int can_id;
        std::stringstream ss(userInput);
        if (userInput.find("0x") == 0) {
            std::stringstream ss(userInput);
            ss >> std::hex >> can_id;
        } else {
            ss >> std::dec >> can_id;
        }

        if (can_id > 0x7FF) {
            std::cerr << "Error: CAN ID must be between 0 and 0x7FF.\n";
            printUsage();
            return 1;
        }

        frame.can_id = can_id;

        // Input data
        std::cout << "Enter CAN message data (up to 8 bytes): ";
        std::getline(std::cin, userInput);

        if (userInput.length() % 2 != 0 || userInput.length() > 16) {
            std::cerr << "Error: Data must be a hexadecimal string of up to 16 characters.\n";
            printUsage();
            return 1;
        } 

        // Handle data
        size_t pos = 0;
        int i = 0;
        while (i < 8 && pos <userInput.length()) {
            std::string byteString = userInput.substr(pos, 2);
            frame.data[i++] = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
            pos += 2;
        }
        frame.can_dlc = i;

        frames.push_back(frame);
        std::cout << "Message with CAN ID 0x" << std::hex << frame.can_id << " added to queue." << std::endl;

        std::cout << "Send another message? (y/n): ";
        std::getline(std::cin, userInput);
        if (userInput != "y") break;
    }

    for (const auto& frm : frames) {
        if (write(s, &frm, sizeof(frm)) != sizeof(frm)) {
            perror("Write");
            return 1;
        }
        std::cout << "Message with CAN ID 0x" << std::hex << frm.can_id << " sent." << std::endl;
    }

    close(s);
    return 0;
}
