//
// Created by tar on 3/7/23.
//

#ifndef NETJUCE_NETBUFFER_H
#define NETJUCE_NETBUFFER_H

#include <juce_core/juce_core.h>

class NetBuffer {
public:
    explicit NetBuffer(uint16_t capacity);

    ~NetBuffer();

    void write(const uint8_t *src, int len);

    void read(uint8_t *dest, int len);

private:
    juce::AbstractFifo fifo;
    uint8_t *buffer;
};


#endif //NETJUCE_NETBUFFER_H
