# CHANGELOG

## [1.2.0] - 2026-02-16

### Added
- **Output format selection** – `GIF_OUTPUT_FORMAT` with three modes:
  - `GIF_OUTPUT_RGB888`  (3 bytes per pixel)
  - `GIF_OUTPUT_RGB565LE` (2 bytes per pixel, little‑endian)
  - `GIF_OUTPUT_RGBA8888` (4 bytes per pixel, alpha always 0xFF)
- **Scratch size helper** – `gif_get_required_scratch_size()` returns the compile‑time constant `GIF_SCRATCH_BUFFER_REQUIRED_SIZE`.
- **Renderer API** – `gif_next_frame_render()` accepts a `GIF_Renderer` structure with callbacks:
  - `begin` – called once before the first frame
  - `blit_indexed` – streams indexed pixel rows + palette for each frame
  - `end` – called after each frame with the frame delay
- **Rect API** – `gif_next_frame_rect()` and inline `gif_next_frame_rect_ex()` expose the decoded frame’s position and size.
- **Disposal method 3 support** – `gif_set_disposal3_buffer()` lets the user provide a backup buffer to correctly restore the previous frame when `disposal=3` is used.
- **Prefix / dual‑include mode** – `GIF_USE_PREFIX` + `GIF_PREFIX` allow namespacing of all public symbols (e.g., `SAFE_gif_next_frame`).
- **LZW backend selection** – `GIF_LZW_BACKEND` chooses between:
  - `GIF_LZW_SAFE` – default, bounds‑checked, minimal memory
  - `GIF_LZW_TURBO_UNSAFE` – faster but requires `GIF_TURBO_RISK_ACCEPTED` to be defined (fewer checks)
- **LZW micro‑cache** – optional caching of expanded codes (enabled by default):
  - `GIF_LZW_MICROCACHE`, `GIF_LZW_MICROCACHE_SLOTS`, `GIF_LZW_MICROCACHE_MAXLEN`, `GIF_LZW_MICROCACHE_ARENA_SIZE`
- **Turbo blit** – `GIF_TURBO_BLIT` (default 0) enables span‑based pixel copying to reduce per‑pixel branching.
- **Turbo map2 for RGB565** – `GIF_TURBO_MAP2` (default 0) uses a 256 KiB lookup table to convert two indexed pixels at once.
- **Risk gate for unsafe backend** – `GIF_TURBO_RISK_ACCEPTED` must be defined to use the `_UNSAFE` LZW backend.
- **Additional compile‑time safety toggles**:
  - `GIF_MINIMAL_GUARDS` – enable/disable extra runtime checks
  - `GIF_NO_CLAMP_INDEX` – disable palette index clamping (faster, but unsafe)
  - `GIF_NO_DIM_CHECKS` – disable dimension boundary checks (faster, but unsafe)
- **Compatibility helpers** – keep old return conventions:
  - `gif_next_frame_compat()` – returns 1 (frame), 0 (finished), -1 (error)
  - `gif_rewind_compat()` – void wrapper
  - `gif_close_compat()` – void wrapper
- **Test suite** – moved out of the header into a standalone `test.c` file, covering all new APIs and error conditions.

### Changed
- Decode flow clarified: either decode into a canvas (`gif_next_frame*`) or stream indexed output via renderer (`gif_next_frame_render`).
- Improved consistency of loop counting and rewinding across animated files.
- Better handling of transparency interactions with disposal methods.
- Internal LZW routines now separate safe and unsafe versions.

### Fixed
- Edge cases in disposal‑3 handling when no backup buffer is provided (graceful downgrade to disposal‑1).
- Palette caching now correctly updates when local palette changes.
- Sub‑block skipping after LZW errors no longer leaves the stream in an inconsistent state.

## [1.1.0] - 2025-09-12

### Added
- New error codes: `GIF_ERROR_BUFFER_TOO_SMALL` and `GIF_ERROR_BAD_FILE`
- Error message utility: `gif_get_error_string()` for descriptive error strings

### Changed
- C99-only: removed platform-specific code, keep the implementation portable
- Error reporting: functions return consistent integer error codes

### Fixed
- Endianness and palette parsing corner cases
- Safer handling of truncated or corrupted input streams

## [1.0.0] - 2025-07-16

### Initial Release
- Basic GIF decoding functionality
- Support for both static and animated GIFs
- Zero dynamic allocations
