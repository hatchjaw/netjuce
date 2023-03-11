//
// Created by tar on 3/7/23.
//

#include "NetBuffer.h"

NetBuffer::NetBuffer(uint16_t capacity) :
        fifo{capacity},
        buffer{new uint8_t[capacity]} {}

NetBuffer::~NetBuffer() {
    delete[] buffer;
}

void NetBuffer::write(const uint8_t *src, int len) {
    auto writeHandle = fifo.write(len);
    auto i{0}, j{0};

    for (; i != writeHandle.blockSize1; ++i) {
        // write the item at index writeHandle.startIndex1 + i
        buffer[writeHandle.startIndex1 + i] = src[i];
    }

    for (; j != writeHandle.blockSize2; ++j) {
        // write the item at index writeHandle.startIndex2 + i
        buffer[writeHandle.startIndex2 + j] = src[i + j];
    }
}

void NetBuffer::read(uint8_t *dest, int len) {
    auto readHandle = fifo.read(len);
    auto i{0}, j{0};

    for (; i != readHandle.blockSize1; ++i) {
        // read the item at index readHandle.startIndex1 + i
        dest[i] = buffer[readHandle.startIndex1 + i];
    }

    for (; j != readHandle.blockSize2; ++j) {
        // read the item at index readHandle.startIndex2 + i
        dest[i + j] = buffer[readHandle.startIndex2 + j];
    }

//    fifo.reset();
}
