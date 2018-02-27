//
// Created by Matthew Edwards on 2/22/18.
//

#include <cassert>
#include <string>
#include <vector>

#include "Linker.h"

int main(int argc, char **argv)
{
    using namespace m20;

    assert(argc >= 3);

    std::vector<const std::string> files;
    for (int i = 2; i < argc; ++i)
    {
        files.emplace_back(argv[i]);
    }

    Linker linker;
    linker.link(files, argv[1]);

    return 0;
}
