//
// Created by tar on 3/7/23.
//

#ifndef NETJUCE_UTILS_H
#define NETJUCE_UTILS_H

#include <iostream>

class Utils {
public:
    static void hexDump(const uint8_t *buf, int nBytes) {
        int word{10}, row{0};
        printf("HEAD");
        for (const uint8_t *p = buf; word < nBytes + 10; ++p, ++word) {
            if (word % 16 == 0) {
                if (word != 0) std::cout << std::endl;
                printf("%04x ", row);
                ++row;
            } else if (word % 2 == 0) {
                std::cout << " ";
            }
            printf("%02x ", *p);
        }
        std::cout << std::endl << std::endl;
    }
};

#endif //NETJUCE_UTILS_H
