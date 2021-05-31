# Building this program

## Release builds

This program was written so that the same code could produce the coarse grained, and fined grained code from the same source.

### Fine Grained

To build the fine grained application, run:

1. `make clean` This will clean all previous object files.
2. `make` This will build the fine grained implementation, which is the default.

By default the `Makefile` will output the binary as `appserver` in the current directory.
This can be changed however, by specifing the `BIN` environment variable before running any make commands.
Example:

`BIN=exampleserver make`, which will place the application at `exampleserver` in the current directory.

### Coarse Grained

To build the coarse grained application, run:

1. `make clean` This will clean all previous object files. This is important, as the two implementations require different compiler flags, and mixing the object files will produce incorrect output.
2. `make` This will build the coarse grained implementation.

By default the `Makefile` will output the binary as `$(BIN)-coarse` in the current directory.
This prefix can be changed however, by specifing the `BIN` environment variable before running any make commands.
Example:

`BIN=exampleserver make`, which will place the application at `exampleserver-coarse` in the current directory.

## Debug Builds

This source can also generate debug builds that provide useful debugging information on stderr. They can be built by appending `debug` to the make build commands above.

For example, to build the corase grained debug implementation, run `make coarse-debug` instead of just `make coarse`. Run the `make clean` command beforehand as usual.

## External Libraries Note

The `chan.{c,h}` file and the `queue.{c,h}` file were originally a part of an external library. However, they have been heavily modified by me for use with this project (especially `queue.{c,h}`). The original library can be found: <https://github.com/tylertreat/chan/>. It is licensed under the Apache-2.0 License. The original library provided a thread safe channel implementation, for use for communicating across threads. I have adapted it by modifing the backing queue structure to be limitless, which required essentially a complete rewrite of `queue.c`.
