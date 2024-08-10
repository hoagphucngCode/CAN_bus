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
    struct can_frame frame;

    // Create socket CAN
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        perror("Socket");
        return 1;
    }

    // Configure interface CAN
    strcpy(ifr.ifr_name, "vcan0");
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        return 1;
    }

    // Assign address for socket CAN
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind");
        return 1;
    }

    // CAN ID filter setup
    struct can_filter rfilter[1];
    rfilter[0].can_id = 0x200;
    rfilter[0].can_mask = 0x700;
    if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter)) < 0) {
        perror("Setsockopt");
        return 1;
    }

    std::cout << "Node C: Waiting to receive CAN messages with ID from 0x200 to 0x299 ..." << std::endl;

    while (true) {
        int nbytes = read(s, &frame, sizeof(frame));
        if (nbytes < 0) {
            perror("Read");
            return 1;
        }

        // Print out CAN messages
        std::cout << "Received CAN message: ID = 0x" << std::hex << frame.can_id << ", Data = ";
        for (int i = 0; i < frame.can_dlc; i++) {
            std::cout << std::hex << static_cast<int>(frame.data[i]) << " ";
        }
        std::cout << std::dec << std::endl;
    }

    close(s);
    return 0;
}
