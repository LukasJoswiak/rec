// Copyright 2019 Lukas Joswiak

#include "server/code.hpp"

#include <iostream>

static bool kInitialized = false;

void Code::Init() {
  if (!kInitialized) {
    if (cm256_init()) {
      std::cerr << "Failed to initialize cm256, exiting..." << std::endl;
      exit(1);
    }

    kInitialized = true;
  }
}

bool Code::Encode(unsigned int* data, int bytes, cm256_block* blocks,
                  uint8_t** recovery_blocks, int* block_size) {
  Init();

  *block_size = bytes / kOriginalBlocks;

  cm256_encoder_params params;
  params.BlockBytes = *block_size;
  params.OriginalCount = kOriginalBlocks;
  params.RecoveryCount = kRedundantBlocks;

  for (int i = 0; i < params.OriginalCount; ++i) {
    blocks[i].Block = (char*) data + i * params.BlockBytes;
  }

  // TODO: convert to smart pointer.
  uint8_t* created_recovery_blocks =
      new uint8_t[params.RecoveryCount * params.BlockBytes];

  if (cm256_encode(params, blocks, created_recovery_blocks)) {
    return false;
  }

  *recovery_blocks = created_recovery_blocks;

  return true;
}

bool Code::Decode(cm256_block* blocks, int block_size) {
  Init();

  cm256_encoder_params params;
  params.BlockBytes = block_size;
  params.OriginalCount = kOriginalBlocks;
  params.RecoveryCount = kRedundantBlocks;

  if (cm256_decode(params, blocks)) {
    return false;
  }

  return true;
}
