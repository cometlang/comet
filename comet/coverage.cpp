#include <iostream>
#include <fstream>
#include <filesystem>

extern "C" {

#include "comet_private.h"

static const std::string coverage_file = ".coverage";

void outputCoverage(VM *vm)
{
    if (std::filesystem::is_regular_file(coverage_file)) {
        std::filesystem::remove(coverage_file);
    }
    std::ofstream outfile;
    outfile.open(coverage_file);

    outfile.close();
}

}