#ifndef INCLUDE_SERVER_CODE_HPP_
#define INCLUDE_SERVER_CODE_HPP_

#include "cm256/cm256.h"

class Code {
 public:
  // Number of original blocks to split the data into. kOriginalBlocks +
  // kRedundantBlocks should be < 256.
  static constexpr int kOriginalBlocks = 3;
  // Number of redundant blocks to generate.
  static constexpr int kRedundantBlocks = 2;
  // Total blocks.
  static constexpr int kTotalBlocks = kOriginalBlocks + kRedundantBlocks;

  // Encodes the given data of size bytes, storing the original and recovery
  // blocks in the output parameters blocks and recovery_blocks, and the block
  // size in the output parameter block_size. Returns true on success, or false
  // on failure.
  //
  // Caller is responsible for freeing the memory allocated for recovery_blocks.
  static bool Encode(unsigned int* data, int bytes,
      cm256_block* blocks, uint8_t** recovery_blocks,
      int* block_size);

  // Decodes the given data built from original shares and recovery shares.
  // Block indices must be properly set before calling this function. Returns
  // true on success, false on failure.
  static bool Decode(cm256_block* blocks, int block_size);

 private:
  Code() {}

  // Initializes library if it has not been initialized.
  static void Init();
};

#endif  // INCLUDE_SERVER_CODE_HPP_
