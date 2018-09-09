# Changelog
All changes between releases will be documented in this file.

## Unreleased
### Added
 - Additional manpage documentation for CLI commands.

### Changed
 - Refactored usage messages so they are automatically generated from the
   argument sets of individual commands.

## 0.2.0 (2018/09/08)
### Added
 - Install and uninstall targets to Makefile.
 - Implemented loading the board during a query, so the `co_query_getboard()`
   functions will work.
 - Format `%b` is used by `cobalt show` to print the board for the task.
 - Library API for reading all tasks.
 - If no board or task is provided to `cobalt show`, then all tasks are printed.

### Changed
 - Converted `testbench.pl` to `run-test.pl`, which runs a single test. This
   allows the Makefile to divide tests into different rules and parallelize the
   results.
 - Database cleanup will also cleanup all unused query results; this allows
   callers to stop reading queries and exit without leaking memory.
 - Refactored argument parsing to remove a lot of boilerplate.

### Fixed
 - Memory leak when running the `gc` CLI command.
 - Use of uninitialized list when freeing an unused database.

## 0.1.0 (2018/09/01)
### Added
 - Man page for `cobalt` command using `asciidoc(1)`, along with `Makefile`
   changes to build documentation.
 - Added `printf(3)`-style format string argument to `show` command.
 - Argument to the `show` command to show a single id instead of a whole board.
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

