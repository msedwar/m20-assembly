//
// Created by Matthew Edwards on 2/26/18.
//

#include <cassert>
#include <string>

#include "Simulator.h"

int main(int argc, char **argv)
{
    using namespace m20;
    assert(argc == 2);

    Simulator simulator(65536);
    simulator.load(std::string(argv[1]));
    simulator.simulate();

    return 0;
}
