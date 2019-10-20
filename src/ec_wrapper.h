// C wrapper around erasure coding library.


// zfec wrapper functions

// Initializes the zfec code.
//
// @param fec_k number of blocks necessary to reconstruct the original data
// @param fec_m total number of blocks
void zfec_init(int fec_k, int fec_m);

// Encodes the given data and stores redundant blocks in output parameter.
//
// @param data data to encode
// @param n size of data, in bytes
// @param output output parameter to store redundant blocks in (must have
//        m - k slots). Memory is allocated on the heap for stored blocks, up
//        to the caller to free
//
// n should be a multiple of k.
void
encode(unsigned char* data, int n, unsigned char** outblocks);

// Decodes the given input blocks and stores reconstructed blocks in output
// parameter.
//
// @param inblocks k blocks used to reconstruct missing primary blocks
// @param output parameter used to store reconstructed primary blocks
// @param block nums indicating which blocks are being passed
// @param block_size size of each block, in bytes
void
decode(const unsigned char** inblocks,
    unsigned char** outblocks,
    unsigned indexes[],
    unsigned int block_size);
