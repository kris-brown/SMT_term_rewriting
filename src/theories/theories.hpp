#ifndef THEORIES  // #include guards
#define THEORIES
#include <stdexcept>
#include <string>

#include "cat.hpp"
#include "natarray.hpp"
#include "boolalg.hpp"
#include "monoid.hpp"
#include "preorder.hpp"

std::vector<Theory> alltheories(){
    return {cat(),natarray(),boolalg(),monoid(), preorder()};
}

Theory get_theory(std::string s) {
    for (auto && t:alltheories())
        if (s==t.name) return t;
    throw std::runtime_error("Theory " + s + " not supported.");
}
#endif
