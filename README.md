# Cobalt

`cobalt` is a simple CLI application and C library for tracking personal TODO
lists. All tasks are identified by a unique 32-bit identifier (represented as an
8 character hex string) and are grouped according to "boards". Each task is
composed of a short description.

## Dependencies
There are no runtime dependencies (besides a C library).

### Build-time
 - ragel FSM compiler
 - GCC-compatible C compiler
 - GNU make

### Tests
 - Perl (any version should work)

## Building

To build, first install the necessary dependencies: the ragel FSM compiler, a
GCC-compatible compiler, and GNU make. The path to ragel can be set with the
`RL` environment variable. The path to the compiler can be set with the `CC`
environment variable. Custom `CFLAGS` and `LDFLAGS` may also be provided. Sane
defaults can be set by sourcing the `release.sh` shell script.

To build the main project:
```
$ make CC=gcc CFLAGS=-O2
```

To run the unit tests:
```
$ make test
```

After building, the `libcobalt.so` must be added to your shared library path,
and `cobalt` should be added to your path.

## Usage

Initialize a database:
```
$ cobalt init
```

Add a new task, on the `todo` board:
```
$ cobalt add @todo
```

Show all tasks on the `todo` board:
```
$ cobalt show @todo
```

See more usage information:
```
$ cobalt help
```

