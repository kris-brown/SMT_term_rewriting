#ifndef THEORIES  // #include guards
#define THEORIES
#include <stdexcept>
#include <string>

Theory cat();
Theory intarray();

Theory get_theory(std::string s) {
    if (s == "cat") return cat();
    else if (s == "intarray") return intarray();
    else throw std::runtime_error("Theory not supported");
}
#endif
