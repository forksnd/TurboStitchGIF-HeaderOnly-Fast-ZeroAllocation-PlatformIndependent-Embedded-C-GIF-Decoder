# GIF Decoder Library

![header-only](https://img.shields.io/badge/header--only-brightgreen)  ![platform-independent](https://img.shields.io/badge/platform-independent-blue)  ![zero-allocations](https://img.shields.io/badge/zero%20allocations-success)  ![license-MIT](https://img.shields.io/badge/license-MIT-green)  ![safety-enhanced](https://img.shields.io/badge/safety-enhanced-red)  

**TurboStitchGIF** is a lightweight, header-only C library for decoding GIF images with a focus on efficiency, safety, and minimal resource usage.  

Designed for embedded systems and performance-critical applications, it provides a simple API for decoding both static and animated GIFs while maintaining a tiny footprint.

## ✨ Key Features

- **Single-header implementation** – Just include `gif.h` in your project  
- **Zero dynamic allocations** – Works with user-provided memory buffers  
- **Platform independent** – Pure C99 with no OS dependencies  
- **Enhanced safety** – Comprehensive bounds checking and error handling  
- **Dual decoding modes:**  
  - *Safe mode:* Minimal memory usage (default)  
  - *Turbo mode:* Optimized for speed (`#define GIF_MODE_TURBO`)  
- **Low memory footprint** – Ideal for embedded systems and microcontrollers  
- **Full animation support** – Handles frames, delays, transparency, and looping  
- **Configurable limits** – Set canvas size and color limits via preprocessor defines  
- **Built-in testing** – Comprehensive test suite with simple test runner

## 🚀 Getting Started

### Basic Usage

```c
#define GIF_IMPLEMENTATION
#include "gif.h"

// Define scratch buffer size (calculate using GIF_SCRATCH_BUFFER_REQUIRED_SIZE)
#define SCRATCH_BUFFER_SIZE GIF_SCRATCH_BUFFER_REQUIRED_SIZE
uint8_t scratch_buffer[SCRATCH_BUFFER_SIZE]; 

void process_gif(const uint8_t* gif_data, size_t gif_size) {
    GIF_Context ctx;
    int result = gif_init(&ctx, gif_data, gif_size, 
                         scratch_buffer, sizeof(scratch_buffer));
    
    if(result != GIF_SUCCESS) {
        printf("Error: %s\n", gif_get_error_string(result));
        return;
    }

    int width, height;
    gif_get_info(&ctx, &width, &height);
    
    uint8_t* frame_buffer = malloc(width * height * 3);
    int delay_ms;
    int frame_result;
    
    while((frame_result = gif_next_frame(&ctx, frame_buffer, &delay_ms)) > 0) {
        // Process frame
        // delay_ms contains frame duration
        printf("Frame decoded, delay: %d ms\n", delay_ms);
    }
    
    if(frame_result < 0) {
        printf("Decoding error: %s\n", gif_get_error_string(frame_result));
    }
    
    gif_close(&ctx);
    free(frame_buffer);
}
```

Configuration Options

Customize the library by defining these before including gif.h:

```c
#define GIF_MAX_WIDTH 800      // Max canvas width
#define GIF_MAX_HEIGHT 600     // Max canvas height
#define GIF_MAX_COLORS 256     // Max colors in palette
#define GIF_MODE_TURBO         // Enable faster decoding
#define GIF_MAX_CODE_SIZE 12   // LZW max code size (usually 12)
#include "gif.h"
```
### Configuration Options

Customize the library by defining these before including `gif.h`:

```c
#define GIF_MAX_WIDTH 800      // Max canvas width
#define GIF_MAX_HEIGHT 600     // Max canvas height
#define GIF_MAX_COLORS 256     // Max colors in palette
#define GIF_MODE_TURBO         // Enable faster decoding
#define GIF_MAX_CODE_SIZE 12   // LZW max code size (usually 12)
#include "gif.h"
```

## 🧪 Testing

The library includes a comprehensive test suite that can be easily integrated into your project:

```c
#define GIF_IMPLEMENTATION
#define GIF_TEST
#include "gif.h"

int main() {
    // Run all tests
    int failed_tests = gif_run_tests();
    
    if(failed_tests == 0) {
        printf("All tests passed!\n");
    } else {
        printf("%d tests failed\n", failed_tests);
    }
    
    return failed_tests;
}
```

### The test suite includes:

- Basic initialization tests  
- Error handling verification  
- Memory bounds checking  
- Frame decoding validation  
- Buffer overflow protection tests  

---

## 📚 API Reference

### Core Functions

| Function                   | Description                          |
|----------------------------|--------------------------------------|
| `gif_init()`               | Initialize decoder context           |
| `gif_get_info()`           | Get GIF dimensions                   |
| `gif_next_frame()`         | Decode next animation frame          |
| `gif_rewind()`             | Restart animation from beginning     |
| `gif_close()`              | Clean up decoder context             |
| `gif_set_error_callback()` | Set custom error handler             |
| `gif_get_error_string()`   | Get descriptive error message        |

---

### Error Codes

The library provides detailed error codes through the `gif_get_error_string()` function:

- `GIF_SUCCESS` – Operation completed successfully  
- `GIF_ERROR_DECODE` – Error during LZW data decoding  
- `GIF_ERROR_INVALID_PARAM` – Invalid parameter passed  
- `GIF_ERROR_BAD_FILE` – Invalid or corrupted GIF file  
- `GIF_ERROR_EARLY_EOF` – Unexpected end of file  
- `GIF_ERROR_NO_FRAME` – No next frame found  
- `GIF_ERROR_BUFFER_TOO_SMALL` – User-provided buffer too small  
- `GIF_ERROR_INVALID_FRAME_DIMENSIONS` – Invalid frame dimensions  
- `GIF_ERROR_UNSUPPORTED_COLOR_DEPTH` – Unsupported color depth  
- `GIF_ERROR_BUFFER_OVERFLOW` – Internal buffer overflow detected  
- `GIF_ERROR_INVALID_LZW_CODE` – Invalid LZW code encountered  

---

### Memory Requirements

The library requires a scratch buffer whose size depends on the selected mode:

```c
// Check required buffer size
size_t buffer_size = GIF_SCRATCH_BUFFER_REQUIRED_SIZE;

// Safe mode (default) - minimal memory usage
#define SCRATCH_BUFFER_SIZE GIF_SCRATCH_BUFFER_REQUIRED_SIZE

// Turbo mode (faster) - requires more memory
#define GIF_MODE_TURBO
#define SCRATCH_BUFFER_SIZE GIF_SCRATCH_BUFFER_REQUIRED_SIZE
```

## 🌟 Why Choose This Library?

- Ultra-lightweight – Perfect for resource-constrained environments  
- No external dependencies – Just the C standard library  
- Enhanced safety – Comprehensive bounds checking and error detection  
- Simple integration – Single header file simplifies project setup  
- Efficient decoding – Optimized LZW implementation with Turbo mode  
- Robust error handling – Detailed error codes and callback support  
- Built-in testing – Comprehensive test suite for validation  
- Transparency support – Handles alpha channel properly  
- Animation features – Full support for frame delays and looping  

---

## 💡 Example Projects

Ideal for:

- Embedded systems displays  
- IoT device interfaces  
- Game development  
- Resource-constrained applications  
- Educational projects  
- Custom image viewers  
- Safety-critical systems  

---

## ☕ Support

If you enjoy using this library and find it useful, consider supporting my work with a coffee!  

I am a C purist dedicated to the art of bare-metal systems programming. My philosophy is simple: zero dependencies and zero dynamic memory allocation. I consider malloc my enemy - I prefer predictable, stack-based, and static memory to ensure ultimate stability and speed. Due to limited hardware access, I craft and debug all my code directly on my smartphone. This constraint has forced me to become a disciplined architect, creating tiny, high-performance binaries where every byte is justified. Your support helps me maintain my "zero-bloat" open-source projects and get closer to a dedicated workstation to continue pushing the limits of pure C.

<a href="https://ko-fi.com/ferki" target="_blank">
  <img src="https://ko-fi.com/img/githubbutton_sm.svg" alt="Buy Me a Coffee at ko-fi.com" height="36" />
</a>

*(Currently donations are not active — coming soon!)*


### BTC:
```
bc1qd6rnejket3slkwr3nvz22fcdejmlygzmswpqaa
```

---

## 📜 License

This project is licensed under the **MIT License** – see the [LICENSE](LICENSE) file for details.
