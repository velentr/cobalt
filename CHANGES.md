# Changelog
All changes between releases will be documented in this file.

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

