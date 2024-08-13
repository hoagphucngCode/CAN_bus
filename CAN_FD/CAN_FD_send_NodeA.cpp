#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <ios>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/error.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <utility>
#include <vector>
#include <sstream>

// Function to send a CAN FD message
void sendCANFDMessage(int s, uint32_t can_id, const std::string& data_str, bool is_hex) {
    struct canfd_frame frame;
    // Setting Extended CAN ID flag for CAN FD
    frame.can_id = can_id | CAN_EFF_FLAG; 
    memset(frame.data, 0, sizeof(frame.data));

    // Parse data_std into bytes
    size_t data_length = data_str.length();
    size_t byte_length = (is_hex) ? data_length / 2 : data_length;

    if (byte_length > CANFD_MAX_DLEN) {
        std::cerr << "Error: CAN FD Message data too long. Maximum length is 64 bytes." << std::endl;
        return;
    }

    frame.len = byte_length;

    if (is_hex) {
        for (size_t i = 0; i < byte_length; i++) {
            std::stringstream ss;
            ss << std::hex << data_str.substr(i * 2, 2);
            unsigned int byte;
            ss >> byte;
            frame.data[i] = static_cast<uint8_t>(byte);
        }
    } else {
        std::copy(data_str.begin(), data_str.end(), frame.data);
    }

    if (write(s, &frame, sizeof(frame)) != sizeof(frame)) {
        perror("Write");
    } else {
        std::cout << "Message CAN FD sent: ID = 0x" << std::hex << can_id << ", Data = " << data_str << std::endl;
    }
}

int main() {
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;

    // Create CAN FD socket
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        perror("Socket");
        return 1;
    }

    // Set up interface
    strcpy(ifr.ifr_name, "vcan0");
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        return 1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Set the socket options for CAN FD
    int enable_canfd = 1;
    if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(enable_canfd)) != 0) {
        perror("CAN FD socket options");
        return 1;
    }

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind");
        return 1;
    }

    std::string can_id_str;
    std::string data_str;
    std::vector<std::pair<uint32_t, std::string>> messages;
    std::string input;
    bool is_hex = false;

    // Get user input for multiple messages
    while (true) {
        std::cout << "Enter CAN FD ID (type 'done' to fininsh): ";
        std::getline(std::cin, can_id_str);
        if (can_id_str == "done") break;

        std::cout << "Enter Data (ASCII or hex, up to 64 bytes): ";
        std::getline(std::cin, data_str);

        // Check if the data is in hexadecimal format
        is_hex = (data_str.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos && data_str.length() % 2 == 0);

        // Convert CAN ID to integer
        uint32_t can_id;
        try {
            can_id = std::stoul(can_id_str, nullptr, 0);
        } catch (const std::exception& e) {
            std::cerr << "Invalid CAN ID: " << e.what() << std::endl;
            continue;
        }

        // Store the message
        messages.emplace_back(can_id, data_str);
    }

    // Send all messages
    for (const auto& message : messages) {
        sendCANFDMessage(s, message.first, message.second, is_hex);
    }

    close(s);
    return 0;
}