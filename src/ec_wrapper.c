#include "ec_wrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fec.h>

// Globals to store zfec state.
fec_t* fec = NULL;
int k = 0;
int m = 0;

void zfec_init(int fec_k, int fec_m) {
  fec = fec_new(fec_k, fec_m);
  k = fec_k;
  m = fec_m;
}

void
encode(unsigned char* data, int n, unsigned char** outblocks) {
  if (!fec) {
    fprintf(stderr, "Must call init first\n");
    exit(1);
  }

  const unsigned char* blocks[k];
  int block_size = n / k;
  int redundant_blocks = m - k;

  // Create array of pointers to each block within data.
  size_t i;
  for (i = 0; i < k; ++i) {
    blocks[i] = data + i * block_size;
  }

  // Allocate storage for redundant blocks.
  for (i = 0; i < redundant_blocks; ++i) {
    outblocks[i] = (unsigned char*) malloc(block_size);
  }

  // Assign block numbers to redundant blocks.
  unsigned block_nums[redundant_blocks];
  for (i = k; i < m; ++i) {
    block_nums[i - k] = i;
  }

  fec_encode(fec, blocks, outblocks, block_nums, redundant_blocks, block_size);
}

void
decode(const unsigned char** inblocks,
    unsigned char** outblocks,
    unsigned indexes[],
    unsigned int block_size) {
  if (!fec) {
    fprintf(stderr, "Must call init first\n");
    exit(1);
  }

  fec_decode(fec, inblocks, outblocks, indexes, block_size);
}
