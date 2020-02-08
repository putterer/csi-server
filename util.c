#include <arpa/inet.h>
#include <string.h>

#include "util.h"

/**
 * Check whether the current system uses a big endian byte order
 * @returns true if big endian is being used, false otherwise
 */
int isBigEndian() {
    int x = 1;
    return *((char*)&x) != 1;
}

/**
 * Converts a host order long long (64-bit value) to network byte order
 * @param in the host order value
 * @returns the network order value
 */
u_int64_t htonll(u_int64_t in) {
    if(isBigEndian()) {
        return in;
    }

    u_int64_t out = 0;
    for(int i = 0;i < 8;i++) {
        *(((char*)&out) + i) = *(((char*)&in) + 7 - i);
    }
    return out;
}

/**
 * Adds a byte to the data buffer
 */
void putByte(char* buffer, int* index, char b) {
    buffer[*index] = b;
    *index += 1;
}

/**
 * Adds a short (16 bit) to the data buffer
 */
void putShort(char* buffer, int* index, u_int16_t s) {
    u_int16_t networkOrder = htons(s);
    memcpy((buffer + *index), &networkOrder, sizeof(networkOrder));
    *index += 2;
}

/**
 * Adds an int (32 bit) to the data buffer
 */
void putInt(char* buffer, int* index, u_int32_t i) {
    u_int32_t networkOrder = htonl(i);
    memcpy((buffer + *index), &networkOrder, sizeof(networkOrder));
    *index += 4;
}

/**
 * Adds a long (64 bit) to the data buffer
 */
void putLong(char* buffer, int* index, u_int64_t l) {
    u_int64_t networkOrder = htonll(l);
    memcpy((buffer + *index), &networkOrder, sizeof(networkOrder));
    *index += 8;
}

