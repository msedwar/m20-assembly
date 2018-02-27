/* =============================================================================
 * M20 Assembly
 *
 * Author: Matthew Edwards (msedwar)
 * Date:   February 22, 2018
 * Description:
 *      Utility functions and classes. (Implementation)
 * =============================================================================
 */

#include "Utils.h"

char* m20::intToBytes(unsigned int i, char *buffer)
{
    buffer[0] = (char) ((i >> 24) & 0xFF);
    buffer[1] = (char) ((i >> 16) & 0xFF);
    buffer[2] = (char) ((i >> 8) & 0xFF);
    buffer[3] = (char) (i & 0xFF);
    return buffer;
}

unsigned short m20::bytesToShort(const char *buffer)
{
    unsigned int i = ((((unsigned int) buffer[0] & 0xFF) << 8)
                     | ((unsigned int) buffer[1] & 0xFF));
    return (unsigned short) i;
}

unsigned int m20::bytesToInt(const char *buffer)
{
    unsigned int i = (((unsigned int) buffer[0] & 0xFF) << 24)
                     | (((unsigned int) buffer[1] & 0xFF) << 16)
                     | (((unsigned int) buffer[2] & 0xFF) << 8)
                     | ((unsigned int) buffer[3] & 0xFF);
    return i;
}
