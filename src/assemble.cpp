//
// Created by Matthew Edwards on 2/22/18.
//
#include <cassert>

#include "Assembler.h"

int main(int argc, char **argv)
{
    using namespace m20;

    assert(argc == 3);
    std::string infile(argv[1]);
    std::string outfile(argv[2]);

    Assembler assembler;
    bool success = assembler.assemble(infile, outfile);

    return success ? 0 : 1;
}
