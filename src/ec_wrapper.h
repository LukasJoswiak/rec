// C wrapper around erasure coding library.

// Encodes the given data and stores redundant blocks in output parameter.
//
// @param data data to encode
// @param n size of data, in bytes
// @param output output parameter to store redundant blocks in (must have m - k slots).
//        memory is allocated on the heap for stored blocks, up to the caller to free
// @param k number of blocks necessary to reconstruct the original data
// @param m total number of blocks
//
// n should be a multiple of k.
void encode(unsigned char* data, int n, unsigned char** outblocks, int k, int m);
