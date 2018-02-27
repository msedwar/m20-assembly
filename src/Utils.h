/* =============================================================================
 * M20 Assembly
 *
 * Author: Matthew Edwards (msedwar)
 * Date:   February 22, 2018
 * Description:
 *      Utility functions and classes.
 * =============================================================================
 */

#ifndef M20_ASSEMBLY_UTILS_H
#define M20_ASSEMBLY_UTILS_H

namespace m20
{
    char *intToBytes(unsigned int i, char *buffer);

    unsigned short bytesToShort(const char *buffer);
    unsigned int bytesToInt(const char *buffer);
}

#endif // M20_ASSEMBLY_UTILS_H
