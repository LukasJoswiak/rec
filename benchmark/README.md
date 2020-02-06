# Erasure Code Library Benchmarks

This directory contains benchmarking code for erasure coding libraries. The benchmark first generates a large number of integers, then runs multiple encoding and decoding rounds while recording the elapsed time.

## Run

Run the benchmarking suite from the top level directory by invoking the `benchmark` target with `make`.

```
$ make benchmark
```

## Sample Output

```
$ ./bin/benchmark
Generating integer array of size 2400000...
Trial 0: Encoding and decoding of data took 42377 microseconds
Trial 1: Encoding and decoding of data took 40771 microseconds
Trial 2: Encoding and decoding of data took 40775 microseconds
Finished benchmark
Trial 0: Encoding and decoding of data took 5165 microseconds
Trial 1: Encoding and decoding of data took 4759 microseconds
Trial 2: Encoding and decoding of data took 4692 microseconds
Finished benchmark
Beginning zfec benchmark
Trial 0: Encoding and decoding of data took 41528 microseconds
Trial 1: Encoding and decoding of data took 41224 microseconds
Trial 2: Encoding and decoding of data took 40879 microseconds
Trial 3: Encoding and decoding of data took 41046 microseconds
Trial 4: Encoding and decoding of data took 40054 microseconds
Trial 5: Encoding and decoding of data took 40663 microseconds
Trial 6: Encoding and decoding of data took 41058 microseconds
Trial 7: Encoding and decoding of data took 40082 microseconds
Trial 8: Encoding and decoding of data took 41395 microseconds
Trial 9: Encoding and decoding of data took 43715 microseconds
Trial 10: Encoding and decoding of data took 40655 microseconds
Trial 11: Encoding and decoding of data took 40147 microseconds
Trial 12: Encoding and decoding of data took 40771 microseconds
Trial 13: Encoding and decoding of data took 41728 microseconds
Trial 14: Encoding and decoding of data took 40463 microseconds
Trial 15: Encoding and decoding of data took 40633 microseconds
Trial 16: Encoding and decoding of data took 39855 microseconds
Trial 17: Encoding and decoding of data took 39933 microseconds
Trial 18: Encoding and decoding of data took 39736 microseconds
Trial 19: Encoding and decoding of data took 39284 microseconds
Finished benchmark
Beginning cm256 benchmark
Trial 0: Encoding and decoding of data took 4971 microseconds
Trial 1: Encoding and decoding of data took 4906 microseconds
Trial 2: Encoding and decoding of data took 4466 microseconds
Trial 3: Encoding and decoding of data took 4876 microseconds
Trial 4: Encoding and decoding of data took 4966 microseconds
Trial 5: Encoding and decoding of data took 4785 microseconds
Trial 6: Encoding and decoding of data took 4555 microseconds
Trial 7: Encoding and decoding of data took 4684 microseconds
Trial 8: Encoding and decoding of data took 4543 microseconds
Trial 9: Encoding and decoding of data took 4540 microseconds
Trial 10: Encoding and decoding of data took 4570 microseconds
Trial 11: Encoding and decoding of data took 4668 microseconds
Trial 12: Encoding and decoding of data took 4684 microseconds
Trial 13: Encoding and decoding of data took 4469 microseconds
Trial 14: Encoding and decoding of data took 4772 microseconds
Trial 15: Encoding and decoding of data took 4555 microseconds
Trial 16: Encoding and decoding of data took 5077 microseconds
Trial 17: Encoding and decoding of data took 4873 microseconds
Trial 18: Encoding and decoding of data took 4404 microseconds
Trial 19: Encoding and decoding of data took 4600 microseconds
Finished benchmark
```
