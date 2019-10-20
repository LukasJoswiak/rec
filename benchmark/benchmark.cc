#include <chrono>
#include <iostream>
#include <random>

extern "C" {
  #include "ec_wrapper.h"
}

// Erasure coding parameters. Require three blocks to rebuild data, and produce
// five blocks total.
const int k = 3;
const int m = 5;

// Fill given array with n randomly generated, single digit integers.
void GenerateData(unsigned int* data, int n) {
  std::cout << "Generating integer array of size " << n << "..." << std::endl;
  std::random_device rd;
  std::uniform_int_distribution<int> dist(0, 9);
  for (int i = 0; i < n; ++i) {
    data[i] = dist(rd);
  }
}

// Encode given data into m blocks. Use the two redundant blocks and the third
// data block to regenerate the first two data blocks, and verify their
// integrity. Also prints the time taken to run the encoding and decoding steps.
void BenchmarkZFec(unsigned int* data, unsigned int n) {
  zfec_init(k, m);
  int block_size = n / 3;

  // Redundant block storage.
  unsigned int* rblocks[2];

  // Reconstruction information.
  unsigned int indexes[] = {3, 4, 2};;
  unsigned int* b0 = (unsigned int*) malloc(block_size * sizeof(data[0]));
  unsigned int* b1 = (unsigned int*) malloc(block_size * sizeof(data[0]));
  unsigned int* outblocks[] = {b0, b1};

  auto start = std::chrono::high_resolution_clock::now();

  encode((unsigned char*) data, n * sizeof(data[0]), (unsigned char**) rblocks);

  // Reconstruct first two blocks using third data block and two redundant
  // blocks.
  const unsigned int* inblocks[] = {rblocks[0], rblocks[1], data + (2 * n / 3)};
  decode((const unsigned char**) inblocks,
      (unsigned char**) outblocks,
      indexes,
      block_size * sizeof(data[0]));

  auto end = std::chrono::high_resolution_clock::now();

  // Make sure data was reconstructed properly.
  for (int i = 0; i < block_size; ++i) {
    assert(data[i] == b0[i]);
    assert(data[i + block_size] == b1[i]);
  }

  free(b0);
  free(b1);

  auto duration =
    std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  std::cout << "Encoding and decoding of data took " << duration.count()
            << " microseconds" << std::endl;
}

int main() {
  int n = 2400000;
  unsigned int* data = (unsigned int*) malloc(n * sizeof(int));
  GenerateData(data, n);

  std::cout << "Beginning zfec benchmark" << std::endl;
  for (int i = 0; i < 20; ++i) {
    std::cout << "Trial " << i << ": ";
    BenchmarkZFec(data, n);
  }
  std::cout << "Finished zfec benchmark" << std::endl;

  free(data);

  return 0;
}
