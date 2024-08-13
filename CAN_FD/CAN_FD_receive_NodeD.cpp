#include <cstdio>
#include <ios>
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
    struct canfd_frame frame;

    // Create socket CAN FD
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        perror("Socket");
        return 1;
    }

    // Configure interface CAN FD
    strcpy(ifr.ifr_name, "vcan0");
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        return 1;
    }

    // Assign address for socket CAN FD
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind");
        return 1;
    }

    // Set the socket options for CAN FD
    int enable_canfd = 1;
    if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(enable_canfd)) != 0) {
        perror("CAN FD socket options");
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

    std::cout << "Node D: waiting to receive CAN FD Messages with ID form 0x200 to 0x299 ..." << std::endl;

    int nbytes = read(s, &frame, CANFD_MTU);
    if (nbytes < 0) {
        perror("Read");
        return 1;
    }

    // Print out CAN FD Messages
    std::cout << "Received CAN FD message with ID: 0x" << std::hex << (frame.can_id & CAN_EFF_MASK) << ", Data = ";
    for (int i = 0; i < frame.len; i++) {
        std::cout << std::hex << static_cast<int>(frame.data[i]) << " ";
    }
    std::cout << std::dec << std::endl;

    close(s);
    return 0;
}