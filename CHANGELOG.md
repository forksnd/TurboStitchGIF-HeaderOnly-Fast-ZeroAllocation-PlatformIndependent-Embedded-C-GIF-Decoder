# CHANGELOG

## [1.1.0] - 2025-09-12

### Added
- Enhanced safety features: Comprehensive bounds checking throughout the codebase
- New error codes: Added `GIF_ERROR_BUFFER_OVERFLOW` and `GIF_ERROR_INVALID_LZW_CODE`
- Error message utility: New `gif_get_error_string()` function for descriptive error messages
- Built-in testing framework: Comprehensive test suite enabled with `GIF_TEST` macro
- Test runner function: `gif_run_tests()` for easy validation
- Improved error handling: All internal functions now return proper error codes

### Changed
- API improvements: All functions now have consistent error return values
- Memory safety: Enhanced buffer overflow protection in LZW decoding
- Platform independence: Removed all platform-specific code, pure C99 compliance
- Error reporting: More descriptive error messages through callback system

### Fixed
- Endianness handling: Improved cross-platform compatibility
- Parameter validation: Added null pointer checks throughout API
- Buffer management: Better handling of edge cases and corrupted files
- Documentation: Updated README with new API and testing instructions

## [1.0.0] - 2025-07-16

### Initial Release
- Basic GIF decoding functionality
- Support for both static and animated GIFs
- Dual decoding modes (Safe and Turbo)
- Zero dynamic allocations
- Header-only implementation
- Basic error handling