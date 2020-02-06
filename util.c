#include <arpa/inet.h>
#include <string.h>

#include "util.h"

int isBigEndian() {
    int x = 1;
    return *((char*)&x) != 1;
}

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

void putByte(char* buffer, int* index, char b) {
    buffer[*index] = b;
    *index += 1;
}

void putShort(char* buffer, int* index, u_int16_t s) {
    u_int16_t networkOrder = htons(s);
    memcpy((buffer + *index), &networkOrder, sizeof(networkOrder));
    *index += 2;
}

void putInt(char* buffer, int* index, u_int32_t i) {
    u_int32_t networkOrder = htonl(i);
    memcpy((buffer + *index), &networkOrder, sizeof(networkOrder));
    *index += 4;
}

void putLong(char* buffer, int* index, u_int64_t l) {
    u_int64_t networkOrder = htonll(l);
    memcpy((buffer + *index), &networkOrder, sizeof(networkOrder));
    *index += 8;
}

