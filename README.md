# rec (Replicated Erasure Coding)

![build](https://github.com/LukasJoswiak/rec/workflows/build/badge.svg)

rec is a replicated key-value store which uses erasure coding to reduce network and storage costs associated with replication. This work is largely based on [When Paxos Meets Erasure Code: Reduce Network and Storage Cost in State Machine Replication](https://madsys.cs.tsinghua.edu.cn/publications/HPDC2014-mu.pdf).

## Build

Run `./install` to install required dependencies. Run `make` to build the project once dependencies have been installed.

The install script checks for `protoc`, the protobuf compiler. If it is installed, the script will not build protobuf from source. Make sure the protobuf runtime library exists in your PATH in addition to the `protoc` compiler.

## Run

Running `make` will create two binaries, `bin/rec` and `bin/client`. Each takes a number of options, described below.

```bash
./bin/rec <server-name> <server-port>
./bin/client <server-address> <server-port> <number-of-clients> <requests-per-client>
```

Currently, the list of servers must be hardcoded in [servers.hpp](https://github.com/LukasJoswiak/rec/blob/master/include/server/servers.hpp). Running with five servers and erasure coding enabled, a sample run might look like:

```bash
# Terminal 1
./bin/rec server1 1111

# Terminal 2
./bin/rec server2 1112

# Terminal 3
./bin/rec server3 1113

# Terminal 4
./bin/rec server4 1114

# Terminal 5
./bin/client localhost 1114 1 100
```

With erasure coding enabled and five total servers, a quorum is four servers.

Enabling/disabling erasure coding can be configured in [code.hpp](https://github.com/LukasJoswiak/rec/blob/master/include/server/code.hpp), where you can also control the number of original and redundant blocks. The number of original blocks plus the number of redundant blocks should sum to the total number of servers in the system so one chunk can be sent to each server. A compile time assertion checks for this.
