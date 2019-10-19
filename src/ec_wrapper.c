#include "ec_wrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fec.h>

void encode(unsigned char* data, int n, unsigned char** outblocks, int k, int m) {
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

  fec_t *const fec = fec_new(k, m);

  fec_encode(fec, blocks, outblocks, block_nums, redundant_blocks, block_size);
}
