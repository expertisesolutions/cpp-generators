#include <iostream>
#include <iterator>

#include <generator.hpp>
#include <context.hpp>

#include "reverse.hh"

using namespace efl::eolian::grammar;

int main(int argc, char const *argv[])
{
    std::string out;
    if (!as_generator(::reverse).generate(std::ostream_iterator<char>(std::cout), "John Doe", context_null())) {
      std::cerr << "Failed!" << std::endl;
      return 1;
    }

    std::cout << std::endl;
    return 0;
}
