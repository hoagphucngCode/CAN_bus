#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

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

    // Assign address for socket CAN
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind");
        return 1;
    }

    struct can_frame frame;
    std::cout << "Node B: Waiting to receive CAN messages..." << std::endl;

    while (true) {
        int nbytes = read(s, &frame, sizeof(frame));
        if (nbytes < 0) {
            perror("Read");
            return 1;
        }

        // Print out CAN messages
        std::cout << "Received CAN message with ID: " << frame.can_id << std::endl;
        std::cout << "Data (decimal): ";
        for (int i = 0; i < frame.can_dlc; i++) {
            std::cout << static_cast<int>(frame.data[i]) << " ";
        }
        std::cout << std::endl;
    }

    close(s);
    return 0;
}