#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ios>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>

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
    ioctl(s, SIOGIFINDEX, &ifr);

    // Assign address to socket CAN
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("Bind");
        return 1;
    }

    // Receive message CAN
    struct can_frame frame;
    std::vector<struct can_frame> messages;

    std::cout << "Waiting to receive CAN messages..." << std::endl;

    while (true) {
        int nbytes = read(s, &frame, sizeof(frame));
        if (nbytes < 0) {
            perror("Read");
            return 1;
        }    

        messages.push_back(frame);

        auto min_frame = *std::min_element(messages.begin(), messages.end(),
            [](const struct can_frame& a, const struct can_frame& b) {
                return a.can_id < b.can_id;
            }
        );

        // Print out message CAN received
        system("clear");

        std::cout << "Received CAN frame with ID: 0x" << std::hex << min_frame.can_id << std::dec << std::endl;
        std::cout << "Data: ";
        
        // Show data with string
        for (int i = 0; i < min_frame.can_dlc; i++) {
            std::cout << std::hex << static_cast<int>(min_frame.data[i]) << " ";
        }
        std::cout << std::dec << std::endl;

        std::cout << "Data (ASCII): ";
        for (int i = 0; i < min_frame.can_dlc; i++) {
            if (frame.data[i] >= 32 && min_frame.data[i] <= 126) {
                std::cout << static_cast<char>(min_frame.data[i]);
            } else {
                std::cout << '.';
            }
        }
        std::cout << std::endl;
    }

    close(s);
    return 0;
}