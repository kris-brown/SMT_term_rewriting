#ifndef THEORIES  // #include guards
#define THEORIES
#include <stdexcept>
#include <string>

#include "cat.hpp"
#include "intarray.hpp"
#include "boolalg.hpp"

Theory get_theory(std::string s) {
    std::vector<Theory>theories{cat(), intarray(), boolalg()};
    for (auto && t:theories)
        if (s==t.name) return t;

    throw std::runtime_error("Theory not supported");
}
#endif
