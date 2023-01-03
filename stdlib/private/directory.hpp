#ifndef _directory_hpp_
#define _directory_hpp_
#include <filesystem>
#include "comet.h"

extern "C" {
    VALUE directory_create(VM *vm, const std::filesystem::path& path);
}

#endif