# Changelog
All changes between releases will be documented in this file.

## Unreleased
### Added
 - Man page for `cobalt` command using `asciidoc(1)`, along with `Makefile`
   changes to build documentation.
 - Added `printf(3)`-style format string argument to `show` command.
 - Argument to the `add` command (`--data|-d`) may be used to specify data on
   the command line, instead of opening `$EDITOR`.
 - Interface for deleting tasks, both in the API and CLI (using the `-d` switch
   to the `modify` command).
 - Interface for modifying the data of a task, both in the API and CLI (using
   the `-e` switch to the `modify` command).
 - Interface for manually running garbage collection (`cobalt gc`).
 - Documentation for some internal APIs and data structures.
 - File descriptor tracking, for determining leaked file descriptors with
   `testlib`.

### Fixed
 - Added newline to the end of `show` error message.
 - Fix use of dstring after it is cleared.

## 0.0.1 (2018/08/27)
### Added
 - Initial changelog based on [Keep a
   Changelog](http://keepachangelog.com/en/1.0.0/).
 - Opcode `OP_NOP` for a no-op and `OP_ABORT` for failing, mainly used in
   testing.
 - Tests for validating correct opcode functionality.

### Changed
 - `testbench.pl` will create a test fixture directory for each test and pass
   the path as the first argument; fixtures are removed on success.

### Fixed
 - Correctly remove created file if the subsequent `write(2)` fails.

