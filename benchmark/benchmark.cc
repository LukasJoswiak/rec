#include <iostream>

extern "C" {
  #include "ec_wrapper.h"
}

int main() {
  std::cout << "Hello, World!" << std::endl;

  unsigned char data[24];
  std::memset(data, 1, 8);
  std::memset(data + 8, 2, 8);
  std::memset(data + 16, 3, 8);
  for (int ai : data)
    printf("%.2x", ai);
  std::cout << std::endl;

  init_zfec(3, 5);

  unsigned char* outblocks[2];
  encode(data, 24, outblocks);
  for (int i = 0; i < 8; ++i)
    printf("%.2x", outblocks[0][i]);
  std::cout << std::endl;

  const unsigned char* inblocks[] = {outblocks[0], outblocks[1], data + 16};
  unsigned int indexes[] = {3, 4, 2};

  unsigned char b0[8], b1[8];
  unsigned char* outpkts[] = {b0, b1};
  decode(inblocks, outpkts, indexes, 8);

  for (int i = 0; i < 8; ++i)
    printf("%.2x", b1[i]);
  std::cout << std::endl;

  return 0;
}
