#ifndef THEORIES  // #include guards
#define THEORIES
#include <stdexcept>
#include <string>

#include "cat.hpp"
#include "intarray.hpp"
#include "boolalg.hpp"
#include "monoid.hpp"
#include "preorder.hpp"

std::vector<Theory> alltheories(){
    return {cat(),intarray(),boolalg(),monoid(), preorder()};
}

Theory get_theory(std::string s) {
    for (auto && t:alltheories())
        if (s==t.name) return t;
    throw std::runtime_error("Theory not supported");
}
#endif
