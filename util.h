#include <sys/types.h>

int isBigEndian();
u_int64_t htonll(u_int64_t in);

void putByte(char* buffer, int* index, char b);
void putShort(char* buffer, int* index, u_int16_t s);
void putInt(char* buffer, int* index, u_int32_t i);
void putLong(char* buffer, int* index, u_int64_t l);