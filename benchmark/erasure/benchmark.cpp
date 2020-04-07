#include <chrono>
#include <iostream>
#include <random>

#include "../../lib/cm256/cm256.h"

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

// Run multiple encoding and decoding rounds using the provided benchmark
// function.
// @param data the data to encode
// @param n the size of data, in bytes
// @param trials the number of trials to run
// @param benchmark_fn the benchmark function to run
void RunTrials(unsigned int* data, int n, int trials,
    std::function<void(unsigned int*, unsigned int)> benchmark_fn) {
  for (int i = 0; i < trials; ++i) {
    std::cout << "Trial " << i << ": ";
    benchmark_fn(data, n);
  }
  std::cout << "Finished benchmark" << std::endl;
}

// Encode given data into m blocks. Use the two redundant blocks and the third
// data block to regenerate the first two data blocks, and verify their
// integrity. Also prints the time taken to run the encoding and decoding steps.
// @param data the data to encode
// @param n the size of data, in bytes
void RunZfecBenchmark(unsigned int* data, unsigned int n) {
  zfec_init(k, m);
  int block_size = n / k;

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

// Encode and decode given data using the CM256 library.
// @param data the data to encode
// @param n the size of data, in bytes
void RunCM256Benchmark(unsigned int* data, int n) {
  int block_size = n / k;

  cm256_encoder_params params;
  params.BlockBytes = block_size;
  params.OriginalCount = k;
  params.RecoveryCount = m - k;

  cm256_block blocks[k];
  for (int i = 0; i < params.OriginalCount; ++i) {
    blocks[i].Block = data + i * params.BlockBytes;
  }

  // Storage for redundant blocks.
  uint8_t* recovery_blocks = new uint8_t[params.RecoveryCount * params.BlockBytes];

  auto start = std::chrono::high_resolution_clock::now();

  if (cm256_encode(params, blocks, recovery_blocks)) {
    exit(1);
  }

  // Set block indices.
  for (int i = 0; i < params.OriginalCount; ++i) {
    blocks[i].Index = cm256_get_original_block_index(params, i);
  }

  // Regenerate original data from two recovery chunks and third original block.
  blocks[0].Block = recovery_blocks;
  blocks[0].Index = cm256_get_recovery_block_index(params, 0);
  blocks[1].Block = recovery_blocks + block_size;
  blocks[1].Index = cm256_get_recovery_block_index(params, 1);

  if (cm256_decode(params, blocks)) {
    exit(1);
  }

  auto end = std::chrono::high_resolution_clock::now();

  // Ensure decoded data matches original data.
  for (int i = 0; i < block_size / sizeof(int); ++i) {
    assert(data[i] == ((unsigned int*) blocks[0].Block)[i]);
    assert(data[i + block_size] == ((unsigned int*) blocks[1].Block)[i]);
  }

  delete[] recovery_blocks;

  auto duration =
    std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  std::cout << "Encoding and decoding of data took " << duration.count()
            << " microseconds" << std::endl;
}

int main() {
  if (cm256_init()) {
    exit(1);
  }

  int n = 2400000;
  unsigned int* data = (unsigned int*) malloc(n * sizeof(int));
  GenerateData(data, n);

  // Warm up caches.
  RunTrials(data, n, 3, RunZfecBenchmark);
  RunTrials(data, n, 3, RunCM256Benchmark);

  // Run benchmarks.
  std::cout << "Beginning zfec benchmark" << std::endl;
  RunTrials(data, n, 20, RunZfecBenchmark);

  std::cout << "Beginning cm256 benchmark" << std::endl;
  RunTrials(data, n, 20, RunCM256Benchmark);

  free(data);

  return 0;
}
