#include <iostream>

extern "C" {
  #include "ec_wrapper.h"
}

int main() {
  std::cout << "Hello, World!" << std::endl;

  test();

  return 0;
}
