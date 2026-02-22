# GIF Decoder Library

![header-only](https://img.shields.io/badge/header--only-brightgreen)  ![platform-independent](https://img.shields.io/badge/platform-independent-blue)  ![zero-allocations](https://img.shields.io/badge/zero%20allocations-success)  ![license-MIT](https://img.shields.io/badge/license-MIT-green)  ![safety-enhanced](https://img.shields.io/badge/safety-enhanced-red)  

**TurboStitchGIF** is a lightweight, header-only C library for decoding GIF images with a focus on efficiency, safety, and minimal resource usage.

Designed for embedded systems and performance-critical applications, it provides a complete API for decoding both static and animated GIFs while maintaining a tiny footprint and zero dynamic allocations.

## ✨ Key Features

- **Single-header implementation** – Just include `gif.h` in your project
- **Zero dynamic allocations** – Works with user-provided scratch memory only
- **Platform independent** – Pure C99, no OS or libc dependencies beyond basic string.h
- **Multiple output formats** – RGB888, RGB565LE, RGBA8888 (selectable at compile time)
- **Dual LZW backends:**
  - *Safe mode:* Bounds-checked, minimal memory usage (default)
  - *Turbo mode:* Maximum speed, fewer checks (`GIF_LZW_TURBO_UNSAFE`)
- **Flexible rendering** – Canvas mode (direct RGB output) or Renderer mode (indexed + callbacks)
- **Full animation support** – Frame delays, transparency, disposal methods, NETSCAPE looping
- **Configurable limits** – Set canvas size, color limits, LZW parameters via preprocessor defines
- **Built-in test suite** – Standalone `test.c` covering all APIs and edge cases
- **Optional micro-cache** – Speeds up LZW decoding by caching expanded codes
- **Turbo optimizations** – Span-based blitting, 2-pixel-at-once RGB565 map
- **Error handling** – Detailed error codes + optional callback
- **Prefix mode** – Namespace all symbols with a user-defined prefix

## 🚀 Getting Started

### ***Basic Usage (Canvas Mode)***

```c
#define GIF_IMPLEMENTATION
#include "gif.h"

// Scratch buffer size is a compile-time constant
uint8_t scratch[GIF_SCRATCH_BUFFER_REQUIRED_SIZE];

void decode_gif(const uint8_t* gif_data, size_t gif_size) {
    GIF_Context ctx;
    int result = gif_init(&ctx, gif_data, gif_size, scratch, sizeof(scratch));
    if (result != GIF_SUCCESS) {
        printf("Init error: %s\n", gif_get_error_string(result));
        return;
    }

    int width, height;
    gif_get_info(&ctx, &width, &height);
    
    // Allocate canvas for RGB output (format selected by GIF_OUTPUT_FORMAT)
    uint8_t* canvas = malloc(width * height * GIF_OUTPUT_BPP);
    int delay_ms;
    
    while (gif_next_frame(&ctx, canvas, &delay_ms) == GIF_SUCCESS) {
        printf("Frame decoded, delay: %d ms\n", delay_ms);
        // Canvas now contains the frame in selected output format
    }
    
    gif_close(&ctx);
    free(canvas);
}
```

### ***Renderer Mode (Indexed Output)***

```c
typedef struct {
    int frame_count;
} MyData;

void begin_callback(void* user, int w, int h) {
    MyData* d = (MyData*)user;
    d->frame_count = 0;
    printf("Animation started: %dx%d\n", w, h);
}

void blit_callback(void* user, int x, int y, int w, int h,
                   const uint8_t* idx, int stride,
                   const uint8_t* palette, int num_colors,
                   int transparent, int has_transparency) {
    MyData* d = (MyData*)user;
    d->frame_count++;
    // idx contains indexed pixels, palette contains RGB triplets
    // Your custom blitter can now composite this patch
}

void end_callback(void* user, int delay_ms) {
    MyData* d = (MyData*)user;
    printf("Frame %d complete, delay %d ms\n", d->frame_count, delay_ms);
}

// Usage
GIF_Renderer renderer = {
    .user = &my_data,
    .begin = begin_callback,
    .blit_indexed = blit_callback,
    .end = end_callback
};

gif_next_frame_render(&ctx, &renderer, &delay_ms);
```

---

## ⚙️ Configuration

Define these macros before including `gif.h`.

### Basic Limits

| Macro | Default | Description |
|--------|----------|------------|
| GIF_MAX_WIDTH | 480 | Maximum canvas width (affects line buffer size) |
| GIF_MAX_COLORS | 256 | Maximum palette colors |
| GIF_MAX_CODE_SIZE | 12 | LZW max code size (12 → 4096 entries) |

---

### Output Format (choose one)

| Macro | BPP | Description |
|--------|------|------------|
| GIF_OUTPUT_RGB888 | 3 | 24-bit RGB (default) |
| GIF_OUTPUT_RGB565LE | 2 | 16-bit RGB565 little-endian |
| GIF_OUTPUT_RGBA8888 | 4 | 32-bit RGBA (alpha always 0xFF) |

Set via:

```c
#define GIF_OUTPUT_FORMAT GIF_OUTPUT_RGB565LE
```

---

### LZW Backend & Performance

| Macro | Default | Description |
|--------|----------|------------|
| GIF_LZW_BACKEND | GIF_LZW_SAFE | GIF_LZW_SAFE or GIF_LZW_TURBO_UNSAFE |
| GIF_TURBO_RISK_ACCEPTED | undefined | Must be defined to use _UNSAFE backend |
| GIF_TURBO_BLIT | 0 | Enable span-based pixel blitting |
| GIF_TURBO_MAP2 | 0 | Use 256KB lookup table for RGB565 (2 pixels/cycle) |
| GIF_LZW_MICROCACHE | 1 | Enable micro-cache for LZW expansion |
| GIF_LZW_MICROCACHE_SLOTS | 128 | Number of cache slots (power of two) |
| GIF_LZW_MICROCACHE_MAXLEN | 64 | Maximum cached span length |
| GIF_LZW_MICROCACHE_ARENA_SIZE | 16384 | Cache arena size (bytes) |

---

### Safety Toggles

| Macro | Default | Description |
|--------|----------|------------|
| GIF_MINIMAL_GUARDS | 1 | Enable extra runtime checks |
| GIF_NO_CLAMP_INDEX | 0 | Disable palette index clamping (faster, unsafe) |
| GIF_NO_DIM_CHECKS | 0 | Disable dimension boundary checks (faster, unsafe) |

---

### Namespacing

| Macro | Description |
|--------|------------|
| GIF_USE_PREFIX | Enable prefix mode |
| GIF_PREFIX | Prefix token (e.g., MY_) when GIF_USE_PREFIX is defined |

# 📚 Complete API Reference

## ***Core Types***

```c
// Fixed-width types (provided by library)
typedef unsigned char   gif_u8;      // 8-bit unsigned
typedef unsigned short  gif_u16;     // 16-bit unsigned
typedef unsigned int    gif_u32;     // 32-bit unsigned
typedef unsigned long   gif_usize;   // size-compatible type

// Error codes enumeration
typedef enum {
    GIF_SUCCESS = 0,                    // Operation completed successfully
    GIF_ERROR_DECODE = 1,                // Error during LZW decoding
    GIF_ERROR_INVALID_PARAM = 2,         // NULL or invalid parameter
    GIF_ERROR_BAD_FILE = 3,               // Invalid/corrupted GIF file
    GIF_ERROR_EARLY_EOF = 4,              // Unexpected end of file
    GIF_ERROR_NO_FRAME = 5,                // No more frames available
    GIF_ERROR_BUFFER_TOO_SMALL = 6,        // Scratch buffer too small
    GIF_ERROR_INVALID_FRAME_DIMENSIONS = 7, // Frame outside canvas
    GIF_ERROR_UNSUPPORTED_COLOR_DEPTH = 8,  // Palette too large
    GIF_ERROR_BUFFER_OVERFLOW = 9,          // Internal buffer overflow
    GIF_ERROR_INVALID_LZW_CODE = 10,        // Invalid LZW code in stream
    GIF_ERROR_UNSUPPORTED_DIMENSIONS = 11   // Exceeds GIF_MAX_WIDTH
} GIF_Result;

// Error callback type
typedef void (*GIF_ErrorCallback)(int error_code, const char *message);

// Frame rectangle structure
typedef struct {
    int x;      // X position of frame
    int y;      // Y position of frame
    int w;      // Width of frame
    int h;      // Height of frame
} GIF_FrameRect;
```

## ***Renderer Structure***

```c
// Renderer callbacks structure - all fields must be filled by user
typedef struct {
    // User data passed to all callbacks
    void *user;

    // Called once before the first frame of animation
    // Useful for initializing renderer state
    void (*begin)(void *user, int canvas_w, int canvas_h);

    // Called for each contiguous block of indexed pixels
    // May be called multiple times per frame (once per row, or combined rows)
    void (*blit_indexed)(
        void *user,
        int x, int y,              // Top-left position on canvas
        int w, int h,               // Width and height of this patch
        const gif_u8 *idx,          // Indexed pixel data (row-major)
        int idx_stride,              // Stride in bytes (usually = w)
        const gif_u8 *pal_rgb,       // RGB palette (triplets, length = pal_colors * 3)
        int pal_colors,               // Number of colors in palette
        int transparent_index,        // Index of transparent color (-1 if none)
        int has_transparency);        // 1 if transparency is enabled

    // Called after each complete frame
    // delay_ms contains the frame duration from GIF
    void (*end)(void *user, int delay_ms);
} GIF_Renderer;
```

## ***Main Context Structure***

```c
// Main decoder context (opaque to user, but fields shown for completeness)
typedef struct {
    // Input stream
    const gif_u8 *data;     // Pointer to GIF data
    gif_usize size;          // Total size of GIF data
    gif_usize pos;           // Current read position

    // Canvas dimensions
    gif_u32 canvas_w;        // Width of canvas
    gif_u32 canvas_h;         // Height of canvas

    // Palettes (internal use)
    gif_u8  gpal[GIF_MAX_COLORS * 3];     // Global palette storage
    gif_u8  lpal[GIF_MAX_COLORS * 3];     // Local palette storage
    gif_u16 gpal_n;                         // Number of colors in global palette
    gif_u16 lpal_n;                         // Number of colors in local palette
    const gif_u8 *apal;                     // Active palette pointer
    gif_u16 apal_n;                          // Active palette size

    // Background
    gif_u8 bg_index;          // Background color index
    gif_u8 bg_rgb[3];          // Background RGB value

    // Current frame rectangle
    gif_u16 fx;                // Frame X position
    gif_u16 fy;                // Frame Y position
    gif_u16 fw;                // Frame width
    gif_u16 fh;                // Frame height

    // Frame flags
    gif_u8 packed_img;         // Packed image descriptor flags
    gif_u8 has_lpal;           // 1 if frame has local palette
    gif_u8 lzw_min_code_size;  // LZW minimum code size for this frame

    // Graphic Control Extension (GCE) fields
    gif_u8 has_transparency;   // 1 if transparency enabled
    gif_u8 transparent_index;  // Transparent color index
    gif_u8 disposal_method;    // Disposal method (0-3)
    gif_u16 delay_ms;           // Frame delay in milliseconds

    // Looping
    gif_i16 loop_count;         // Remaining loops (-1 = infinite)
    gif_i16 loop_count_init;    // Initial loop count

    // Animation start position (for rewinding)
    gif_usize anim_start_pos;

    // Scratch buffer pointers (partitioned by gif_init)
    gif_u16 *lzw_prefix;        // LZW prefix table
    gif_u8  *lzw_suffix;        // LZW suffix table
    gif_u8  *lzw_stack;          // LZW stack for expansion
    gif_u8  *line_idx;           // Current line's indexed pixels

    // Palette caches (output format specific)
#if (GIF_OUTPUT_FORMAT == GIF_OUTPUT_RGB565LE)
    gif_u16 pal565[GIF_MAX_COLORS];      // RGB565 palette cache
#endif
#if (GIF_OUTPUT_FORMAT == GIF_OUTPUT_RGBA8888)
    gif_u32 pal32[GIF_MAX_COLORS];        // RGBA8888 palette cache
#endif

    // RGB565 map2 (optional 256KB lookup table)
#if (GIF_OUTPUT_FORMAT == GIF_OUTPUT_RGB565LE) && (GIF_TURBO_MAP2 != 0)
    gif_u32 *map2_565;         // 2-pixel lookup table
    gif_u8   map2_ready;        // 1 if map2 is built
#endif

    // LZW micro-cache (optional)
#if (GIF_LZW_MICROCACHE != 0)
    gif_u8  *mc_arena;          // Cache arena for expanded spans
    gif_u32  mc_arena_pos;       // Current position in arena
    gif_u16 *mc_code;            // Code for each slot
    gif_u16 *mc_len;              // Length of cached span
    gif_u32 *mc_off;              // Offset in arena
    gif_u8  *mc_valid;            // 1 if slot is valid
#endif

    // Disposal method 3 backup buffer
    gif_u8   *disp3_buf;         // User-provided backup buffer
    gif_usize disp3_size;         // Size of backup buffer
    gif_u8    disp3_valid;        // 1 if backup contains valid data
    gif_u16   disp3_x;            // Backup rectangle X
    gif_u16   disp3_y;            // Backup rectangle Y
    gif_u16   disp3_w;            // Backup rectangle width
    gif_u16   disp3_h;            // Backup rectangle height

    // Previous frame information (for disposal)
    gif_u8  prev_disposal;       // Previous frame's disposal method
    gif_u16 prev_x;               // Previous frame X
    gif_u16 prev_y;               // Previous frame Y
    gif_u16 prev_w;               // Previous frame width
    gif_u16 prev_h;               // Previous frame height

    // Canvas state
    gif_u8 canvas_inited;        // 1 if canvas has been initialized

    // LZW stream state
    gif_u32 bitbuf;               // Bit buffer for LZW reading
    int     bitcount;              // Number of bits in bitbuf
    gif_u8  sub_left;              // Remaining bytes in current sub-block
    gif_u8  sub_ended;              // 1 if sub-block stream ended

    // Renderer state
    gif_u8 renderer_started;       // 1 if renderer begin() called

    // Error callback
    GIF_ErrorCallback on_error;    // User error callback
} GIF_Context;
```

## ***Initialization Functions***

```c
/**
 * Returns the minimum required scratch buffer size in bytes.
 * This is a compile-time constant equal to GIF_SCRATCH_BUFFER_REQUIRED_SIZE.
 * 
 * @return Size in bytes needed for scratch buffer
 */
gif_usize gif_get_required_scratch_size(void);

/**
 * Initialize GIF decoder context.
 * 
 * @param ctx           Pointer to context structure to initialize
 * @param data          Pointer to GIF file data in memory
 * @param size          Size of GIF data in bytes
 * @param scratch       User-provided scratch buffer
 * @param scratch_size  Size of scratch buffer (must be >= gif_get_required_scratch_size())
 * 
 * @return GIF_SUCCESS on success, error code otherwise
 */
int gif_init(GIF_Context *ctx,
             const gif_u8 *data, gif_usize size,
             gif_u8 *scratch, gif_usize scratch_size);

/**
 * Get GIF canvas dimensions.
 * 
 * @param ctx   Initialized context
 * @param w     Pointer to store width (can be NULL)
 * @param h     Pointer to store height (can be NULL)
 * 
 * @return GIF_SUCCESS on success, error code otherwise
 */
int gif_get_info(GIF_Context *ctx, int *w, int *h);

/**
 * Set error callback function.
 * 
 * @param ctx   Context to attach callback to
 * @param cb    Callback function (NULL to disable)
 * 
 * @return GIF_SUCCESS on success, error code otherwise
 */
int gif_set_error_callback(GIF_Context *ctx, GIF_ErrorCallback cb);

/**
 * Provide backup buffer for disposal method 3.
 * Required for correct rendering of frames that use "restore previous" disposal.
 * If not provided, disposal 3 is downgraded to disposal 1.
 * 
 * @param ctx   Context
 * @param buf   Buffer to store previous frame (can be NULL to disable)
 * @param size  Size of buffer (must be >= canvas_w * canvas_h * GIF_OUTPUT_BPP)
 * 
 * @return GIF_SUCCESS on success, error code otherwise
 */
int gif_set_disposal3_buffer(GIF_Context *ctx, void *buf, gif_usize size);

/**
 * Rewind animation to first frame.
 * Resets position, loop count, and internal state.
 * 
 * @param ctx   Context
 * 
 * @return GIF_SUCCESS on success, error code otherwise
 */
int gif_rewind(GIF_Context *ctx);

/**
 * Close decoder and reset context.
 * 
 * @param ctx   Context to close
 * 
 * @return GIF_SUCCESS on success, error code otherwise
 */
int gif_close(GIF_Context *ctx);
```

## ***Frame Decoding Functions (Canvas Mode)***

```c
/**
 * Decode next frame into RGB canvas.
 * 
 * @param ctx       Context
 * @param canvas    Output buffer for RGB data (format = GIF_OUTPUT_FORMAT)
 *                  Size must be canvas_w * canvas_h * GIF_OUTPUT_BPP
 * @param delay_ms  Pointer to store frame delay in milliseconds
 * 
 * @return GIF_SUCCESS on success, error code otherwise
 *         GIF_ERROR_NO_FRAME when animation ends
 */
int gif_next_frame(GIF_Context *ctx, void *canvas, int *delay_ms);

/**
 * Decode next frame and also return frame rectangle.
 * 
 * @param ctx       Context
 * @param canvas    Output RGB buffer
 * @param delay_ms  Pointer to store frame delay
 * @param x         Pointer to store frame X position
 * @param y         Pointer to store frame Y position
 * @param w         Pointer to store frame width
 * @param h         Pointer to store frame height
 * 
 * @return GIF_SUCCESS on success, error code otherwise
 */
int gif_next_frame_rect(GIF_Context *ctx, void *canvas, int *delay_ms,
                        int *x, int *y, int *w, int *h);

/**
 * Convenience wrapper for next_frame_rect that returns rect in a structure.
 * 
 * @param ctx       Context
 * @param canvas    Output RGB buffer
 * @param delay_ms  Pointer to store frame delay
 * @param r         Pointer to frame rectangle structure to fill
 * 
 * @return GIF_SUCCESS on success, error code otherwise
 */
static inline int gif_next_frame_rect_ex(GIF_Context *ctx, void *canvas, 
                                         int *delay_ms, GIF_FrameRect *r);
```

## ***Frame Decoding Functions (Renderer Mode)***

```c
/**
 * Decode next frame using renderer callbacks.
 * No canvas is needed; indexed pixels are passed to blit_indexed callback.
 * 
 * @param ctx       Context
 * @param r         Renderer structure with callbacks (all fields must be filled)
 * @param delay_ms  Pointer to store frame delay
 * 
 * @return GIF_SUCCESS on success, error code otherwise
 */
int gif_next_frame_render(GIF_Context *ctx, const GIF_Renderer *r, int *delay_ms);
```

## ***Compatibility Functions (Legacy API)***

```c
/**
 * Compatibility version of next_frame with old return convention.
 * 
 * @param ctx       Context
 * @param canvas    Output RGB buffer
 * @param delay_ms  Pointer to store frame delay
 * 
 * @return 1 = frame decoded, 0 = no more frames, -1 = error
 */
int gif_next_frame_compat(GIF_Context *ctx, void *canvas, int *delay_ms);

/**
 * Compatibility void wrapper for gif_rewind.
 * 
 * @param ctx   Context
 */
void gif_rewind_compat(GIF_Context *ctx);

/**
 * Compatibility void wrapper for gif_close.
 * 
 * @param ctx   Context
 */
void gif_close_compat(GIF_Context *ctx);
```

## ***Error Handling Functions***

```c
/**
 * Get human-readable error string for error code.
 * 
 * @param error_code  One of GIF_Result enum values
 * 
 * @return Pointer to static error string
 */
const char *gif_get_error_string(int error_code);

// Global error strings array (for direct access if needed)
extern const char *gif_error_strings[];
```

## Internal Helper Functions (Exposed in Prefix Mode)

***When using `GIF_USE_PREFIX`, all internal symbols become accessible with the chosen prefix.
These are primarily for advanced users who need to extend or modify the library:***

```c
// Memory operations (lightweight replacements)
void *gif_memset(void *dst, int v, gif_usize n);
void *gif_memcpy(void *dst, const void *src, gif_usize n);
int   gif_memcmp(const void *a, const void *b, gif_usize n);

// Endian helpers
gif_u16 gif_read_u16_le(const gif_u8 *p);

// Alignment
gif_u8 *gif_align_ptr(gif_u8 *p, gif_usize a);

// Error reporting
void gif_report(GIF_Context *ctx, int code, const char *msg);

// Input operations
int gif_read_u8(GIF_Context *ctx, gif_u8 *out);
int gif_read_bytes(GIF_Context *ctx, gif_u8 *dst, gif_usize n);
int gif_skip(GIF_Context *ctx, gif_usize n);

// Sub-block handling
int gif_discard_sub_blocks(GIF_Context *ctx);

// Extension parsing
int gif_read_extension(GIF_Context *ctx);
int gif_read_gce(GIF_Context *ctx);
int gif_read_app_ext(GIF_Context *ctx);
int gif_is_netscape_id(const gif_u8 *id11);

// Color table handling
int gif_read_color_table(GIF_Context *ctx, gif_u8 *dst_rgb, gif_u16 *out_n, gif_u16 ncolors);

// Palette caching
void gif_prepare_palette_cache(GIF_Context *ctx);
gif_u16 gif_rgb_to_565(gif_u8 r, gif_u8 g, gif_u8 b);

// RGB565 map2 (optional)
void gif_build_map2_565(GIF_Context *ctx);

// Canvas helpers
void gif_write_rgb_pixel(gif_u8 *dst, const gif_u8 *rgb);
void gif_canvas_clear(GIF_Context *ctx, void *canvas);
void gif_fill_rect_bg(GIF_Context *ctx, void *canvas,
                      gif_u16 x, gif_u16 y, gif_u16 w, gif_u16 h);

// Disposal handling
int  gif_backup_rect_if_needed(GIF_Context *ctx, void *canvas);
void gif_restore_backup_if_needed(GIF_Context *ctx, void *canvas);
void gif_apply_prev_disposal(GIF_Context *ctx, void *canvas);

// LZW stream
void gif_lzw_stream_begin(GIF_Context *ctx);
int  gif_lzw_stream_read_byte(GIF_Context *ctx, gif_u8 *out);
int  gif_lzw_discard_rest(GIF_Context *ctx);

// LZW bit reader
int gif_lzw_read_code(GIF_Context *ctx, gif_u16 code_size, gif_u16 code_mask, gif_u16 *out_code);

// LZW micro-cache (optional)
void   gif_mc_reset(GIF_Context *ctx);
gif_u16 gif_mc_slot(gif_u16 code);
int    gif_mc_lookup(GIF_Context *ctx, gif_u16 code, const gif_u8 **out_ptr, gif_u16 *out_len);
void   gif_mc_insert(GIF_Context *ctx, gif_u16 code, const gif_u8 *span, gif_u16 len);

// LZW expansion
int  gif_lzw_expand_to_stack_safe(GIF_Context *ctx, gif_u16 clear, gif_u16 code, gif_u32 *out_sp);
void gif_lzw_expand_to_stack_unsafe(GIF_Context *ctx, gif_u16 clear, gif_u16 code, gif_u32 *out_sp);

// Row blitters
void gif_blit_plain_row(GIF_Context *ctx, gif_u8 *dst_row, const gif_u8 *idx_row);
void gif_blit_turbo_row(GIF_Context *ctx, gif_u8 *dst_row, const gif_u8 *idx_row);
void gif_blit_map2_565_row(GIF_Context *ctx, gif_u8 *dst_row, const gif_u8 *idx_row);

// Row emission
int gif_emit_row(GIF_Context *ctx, void *canvas, const GIF_Renderer *r, int y_draw);
int gif_emit_row_canvas(GIF_Context *ctx, void *canvas, int y_draw);
int gif_emit_row_renderer(GIF_Context *ctx, const GIF_Renderer *r, int y_draw);

// Interlace handling
int gif_map_interlace_y(gif_u16 fh, int *pass, int *pass_row, int *out_y);

// LZW decoding (main entry points)
int gif_decode_image_data_safe(GIF_Context *ctx, void *canvas, const GIF_Renderer *r);
int gif_decode_image_data_unsafe(GIF_Context *ctx, void *canvas, const GIF_Renderer *r);

// Frame parsing
int gif_read_image_descriptor(GIF_Context *ctx);
int gif_find_next_image(GIF_Context *ctx);
void gif_restart_animation(GIF_Context *ctx);

// LZW pump
typedef struct {
    void *canvas;
    const GIF_Renderer *r;
    gif_u32 out_x;
    int out_lines;
    int interlaced;
    int pass;
    int pass_row;
} GIF_Pump;

int gif_pump_push_span(GIF_Context *ctx, GIF_Pump *p, const gif_u8 *src, gif_u16 len);
```

## ***Preprocessor Constants***

```c
// LZW table size (4096 for 12-bit codes)
#define GIF_LZW_TABLE_ENTRIES (1u << GIF_MAX_CODE_SIZE)

// Alignment requirement for safety
#define GIF_ALIGN_SLOP 32u

// GIF block markers
#define GIF_TRAILER   0x3Bu
#define GIF_EXT_INTRO 0x21u
#define GIF_IMAGE_SEP 0x2Cu
#define GIF_EXT_GCE   0xF9u
#define GIF_EXT_APP   0xFFu
#define GIF_GCE_BLOCK_SIZE 0x04u

// LZW backend selector values
#define GIF_LZW_SAFE         1
#define GIF_LZW_TURBO_UNSAFE 2

// Scratch buffer component sizes
#define GIF_SCRATCH_PREFIX_SIZE ((gif_u32)GIF_LZW_TABLE_ENTRIES * sizeof(gif_u16))
#define GIF_SCRATCH_SUFFIX_SIZE ((gif_u32)GIF_LZW_TABLE_ENTRIES * sizeof(gif_u8))
#define GIF_SCRATCH_STACK_SIZE  ((gif_u32)GIF_LZW_TABLE_ENTRIES * sizeof(gif_u8))
#define GIF_SCRATCH_LINE_SIZE   ((gif_u32)GIF_MAX_WIDTH * sizeof(gif_u8))

// Complete scratch buffer size (use gif_get_required_scratch_size())
#define GIF_SCRATCH_BUFFER_REQUIRED_SIZE \
    (GIF_ALIGN_SLOP + \
     GIF_SCRATCH_PREFIX_SIZE + GIF_SCRATCH_SUFFIX_SIZE + GIF_SCRATCH_STACK_SIZE + \
     GIF_SCRATCH_LINE_SIZE + \
     (GIF_OUTPUT_FORMAT == GIF_OUTPUT_RGB565LE && GIF_TURBO_MAP2 ? 65536 * sizeof(gif_u32) : 0) + \
     (GIF_LZW_MICROCACHE ? GIF_LZW_MICROCACHE_ARENA_SIZE + \
      GIF_LZW_MICROCACHE_SLOTS * (sizeof(gif_u16) + sizeof(gif_u16) + sizeof(gif_u32) + sizeof(gif_u8)) : 0))
```
# 🧪 Testing

The library includes a comprehensive test suite in `test.c`:

```c
#define GIF_IMPLEMENTATION
#include "gif.h"

int main(void) {
    // All tests are self-contained in test.c
    // Compile and run to verify the library
    return 0;
}
```

---

## Test Coverage

The test suite validates:

- Basic initialization and error cases
- Canvas and renderer mode decoding
- Rect API and disposal method 3
- LZW backends (safe and unsafe)
- Prefix / namespacing mode
- All API variants (including compat wrappers)
- Edge cases and buffer overflow protection
- Error strings and constants validation
- Static 1x1 GIF decoding
- Renderer callback verification

---

# 📦 Scratch Buffer Layout

The library partitions the user-provided scratch buffer as follows:

```
┌────────────────────────────┐
│ Alignment slop             │  (up to 32 bytes, safety padding)
├────────────────────────────┤
│ LZW prefix table           │  uint16_t[4096]  = 8192 bytes
├────────────────────────────┤
│ LZW suffix table           │  uint8_t[4096]   = 4096 bytes
├────────────────────────────┤
│ LZW stack                  │  uint8_t[4096]   = 4096 bytes
├────────────────────────────┤
│ Line buffer                │  uint8_t[GIF_MAX_WIDTH]
│                            │  (default up to 480 bytes)
├────────────────────────────┤
│ RGB565 map2 (optional)     │  65536 * 4 = 262144 bytes
├────────────────────────────┤
│ LZW micro-cache (optional) │  Arena + metadata
│                            │  (size configurable)
└────────────────────────────┘
```

---

## Alignment Guarantees

- `uint16_t` data is 2-byte aligned
- `uint32_t` data is 4-byte aligned
- All scratch partitions are correctly aligned for the target architecture

The required scratch size can be queried via:

```c
gif_get_required_scratch_size();
```
# 💡 Example Projects

This library is ideal for:

- Embedded displays and IoT devices  
- Game engines and tooling  
- Resource-constrained applications  
- Custom image viewers  
- Educational projects  
- Safety-critical systems  
- Bootloaders with splash screens  
- Low-power sensor displays  

---

# ☕ Support

If you enjoy using this library and find it useful, consider supporting my work with a coffee!

I am a C purist dedicated to the art of bare-metal systems programming. My philosophy is simple: **zero dependencies and zero dynamic memory allocation**. I consider `malloc` my enemy — I prefer predictable, stack-based, and static memory to ensure ultimate stability and speed.

Due to limited hardware access, I craft and debug all my code directly on my smartphone. This constraint has forced me to become a disciplined architect, creating tiny, high-performance binaries where every byte is justified.

Your support helps me maintain my *zero-bloat* open-source projects and get closer to a dedicated workstation to continue pushing the limits of pure C.

---

## Ko-fi

<a href="https://ko-fi.com/ferki" target="_blank">
  <img src="https://ko-fi.com/img/githubbutton_sm.svg"
       alt="Buy Me a Coffee at ko-fi.com"
       height="36" />
</a>

> Currently donations are not active — coming soon!

---

## BTC

```
bc1qd6rnejket3slkwr3nvz22fcdejmlygzmswpqaa
```

---

# 📜 License

This project is licensed under the **MIT License**.  
See the `LICENSE` file for details.