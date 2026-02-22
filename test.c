// TEST.C
// GIF TEST SUITE
//
// GOALS
// - cover *new api* + *old compat wrappers*
// - no allocations
// - bounded loops ... no hangs
// - tests are standalone ... just build this file

#include <stdio.h>
#include <string.h>
#include <stdint.h>

// *implementation*
#define GIF_IMPLEMENTATION
#include "gif.h"

// -----------------------------
// HARNESS
// -----------------------------

typedef struct {
  int total;
  int passed;
  int failed;
} TestState;

static void t_print_header(const char *name)
{
  // *section*
  printf("\n== %s ==\n", name);
}

static void t_pass(TestState *t, const char *name)
{
  t->passed += 1;
  printf("PASS: %s\n", name);
}

static void t_fail(TestState *t, const char *name, int line, const char *expr, int gif_rc)
{
  t->failed += 1;
  printf("FAIL: %s\n", name);
  printf("  at test.c:%d\n", line);
  printf("  expr: %s\n", expr);
  printf("  gif_rc: %d %s\n", gif_rc, gif_get_error_string(gif_rc));
}

#define TEST_ASSERT_GIF(T, NAME, COND, GIF_RC) \
  do { \
    (T)->total += 1; \
    if ((COND)) { \
      t_pass((T), (NAME)); \
    } else { \
      t_fail((T), (NAME), __LINE__, #COND, (GIF_RC)); \
    } \
  } while (0)

// -----------------------------
// DATA
// -----------------------------

// *1x1 gif* ... single frame
static const gif_u8 G_GIF_1X1[] = {
  // header
  'G','I','F','8','9','a',
  // lsd
  0x01,0x00,
  0x01,0x00,
  0xF0,
  0x00,
  0x00,
  // gct (2 colors)
  0xFF,0xFF,0xFF,
  0x00,0x00,0x00,
  // gce (delay 10 cs = 100 ms)
  0x21,0xF9,0x04,
  0x00,
  0x0A,0x00,
  0x00,
  0x00,
  // image desc
  0x2C,
  0x00,0x00,0x00,0x00,
  0x01,0x00,0x01,0x00,
  0x00,
  // lzw min code size
  0x02,
  // image data
  0x02,0x02,0x44,0x01,
  0x00,
  // trailer
  0x3B
};

static const gif_u8 G_BAD_SIG[] = {
  'B','A','D','8','9','a',
  0x01,0x00,
  0x01,0x00,
  0x00,
  0x00,
  0x00,
  0x3B
};

// -----------------------------
// TESTS
// -----------------------------

static void test_strings_and_constants(TestState *t)
{
  const char *name;
  const char *s;

  name = "strings_and_constants";
  t_print_header(name);

  TEST_ASSERT_GIF(t, name, (gif_usize)GIF_SCRATCH_BUFFER_REQUIRED_SIZE > 0u, GIF_SUCCESS);

  s = gif_get_error_string(GIF_SUCCESS);
  TEST_ASSERT_GIF(t, name, s != 0, GIF_SUCCESS);

  s = gif_get_error_string(GIF_ERROR_BAD_FILE);
  TEST_ASSERT_GIF(t, name, s != 0, GIF_ERROR_BAD_FILE);
}

static void test_init_errors(TestState *t)
{
  const char *name;
  GIF_Context ctx;
  gif_u8 scratch[GIF_SCRATCH_BUFFER_REQUIRED_SIZE];
  int rc;

  name = "init_errors";
  t_print_header(name);

  // *invalid param* ... null data
  rc = gif_init(&ctx, (const gif_u8 *)0, 0u, scratch, (gif_usize)sizeof(scratch));
  TEST_ASSERT_GIF(t, name, rc == GIF_ERROR_INVALID_PARAM, rc);

  // *bad signature*
  rc = gif_init(&ctx, G_BAD_SIG, (gif_usize)sizeof(G_BAD_SIG), scratch, (gif_usize)sizeof(scratch));
  TEST_ASSERT_GIF(t, name, rc == GIF_ERROR_BAD_FILE, rc);
}

static void test_static_decode_canvas(TestState *t)
{
  const char *name;
  GIF_Context ctx;
  gif_u8 scratch[GIF_SCRATCH_BUFFER_REQUIRED_SIZE];
  int rc;
  int w;
  int h;
  int delay;

  // *canvas* ... large enough for 1x1
  gif_u8 canvas[4 * (int)GIF_OUTPUT_BPP];

  name = "static_decode_canvas";
  t_print_header(name);

  w = 0;
  h = 0;
  delay = 0;

  memset(canvas, 0, sizeof(canvas));

  rc = gif_init(&ctx, G_GIF_1X1, (gif_usize)sizeof(G_GIF_1X1), scratch, (gif_usize)sizeof(scratch));
  TEST_ASSERT_GIF(t, name, rc == GIF_SUCCESS, rc);

  rc = gif_get_info(&ctx, &w, &h);
  TEST_ASSERT_GIF(t, name, rc == GIF_SUCCESS, rc);
  TEST_ASSERT_GIF(t, name, w == 1, rc);
  TEST_ASSERT_GIF(t, name, h == 1, rc);

  rc = gif_next_frame(&ctx, canvas, &delay);
  TEST_ASSERT_GIF(t, name, rc == GIF_SUCCESS, rc);
  TEST_ASSERT_GIF(t, name, delay == 100, rc);

  // *no more frames*
  rc = gif_next_frame(&ctx, canvas, &delay);
  TEST_ASSERT_GIF(t, name, rc == GIF_ERROR_NO_FRAME, rc);

  rc = gif_close(&ctx);
  TEST_ASSERT_GIF(t, name, rc == GIF_SUCCESS, rc);
}

static void test_static_decode_rect(TestState *t)
{
  const char *name;
  GIF_Context ctx;
  gif_u8 scratch[GIF_SCRATCH_BUFFER_REQUIRED_SIZE];
  int rc;
  int delay;
  int x;
  int y;
  int w;
  int h;

  gif_u8 canvas[4 * (int)GIF_OUTPUT_BPP];

  name = "static_decode_rect";
  t_print_header(name);

  delay = 0;
  x = 123;
  y = 123;
  w = 123;
  h = 123;

  memset(canvas, 0, sizeof(canvas));

  rc = gif_init(&ctx, G_GIF_1X1, (gif_usize)sizeof(G_GIF_1X1), scratch, (gif_usize)sizeof(scratch));
  TEST_ASSERT_GIF(t, name, rc == GIF_SUCCESS, rc);

  rc = gif_next_frame_rect(&ctx, canvas, &delay, &x, &y, &w, &h);
  TEST_ASSERT_GIF(t, name, rc == GIF_SUCCESS, rc);
  TEST_ASSERT_GIF(t, name, delay == 100, rc);
  TEST_ASSERT_GIF(t, name, x == 0, rc);
  TEST_ASSERT_GIF(t, name, y == 0, rc);
  TEST_ASSERT_GIF(t, name, w == 1, rc);
  TEST_ASSERT_GIF(t, name, h == 1, rc);

  rc = gif_close(&ctx);
  TEST_ASSERT_GIF(t, name, rc == GIF_SUCCESS, rc);
}

// -----------------------------
// RENDERER
// -----------------------------

typedef struct {
  int begin_called;
  int end_called;
  int blit_called;
  int bw;
  int bh;
  int last_x;
  int last_y;
  int last_w;
  int last_h;
  int last_delay;
} TestRenderer;

static void tr_begin(void *user, int canvas_w, int canvas_h)
{
  TestRenderer *r;
  r = (TestRenderer *)user;
  r->begin_called = 1;
  r->bw = canvas_w;
  r->bh = canvas_h;
}

static void tr_blit_indexed(void *user,
                            int x, int y, int w, int h,
                            const gif_u8 *idx, int idx_stride,
                            const gif_u8 *pal_rgb, int pal_colors,
                            int transparent_index, int has_transparency)
{
  TestRenderer *r;

  // *avoid unused warnings*
  (void)idx;
  (void)idx_stride;
  (void)pal_rgb;
  (void)pal_colors;
  (void)transparent_index;
  (void)has_transparency;

  r = (TestRenderer *)user;
  r->blit_called += 1;
  r->last_x = x;
  r->last_y = y;
  r->last_w = w;
  r->last_h = h;
}

static void tr_end(void *user, int delay_ms)
{
  TestRenderer *r;
  r = (TestRenderer *)user;
  r->end_called = 1;
  r->last_delay = delay_ms;
}

static void test_renderer_api(TestState *t)
{
  const char *name;
  GIF_Context ctx;
  gif_u8 scratch[GIF_SCRATCH_BUFFER_REQUIRED_SIZE];
  TestRenderer tr;
  GIF_Renderer r;
  int rc;
  int delay;

  name = "renderer_api";
  t_print_header(name);

  memset(&tr, 0, sizeof(tr));
  memset(&r, 0, sizeof(r));

  r.user = &tr;
  r.begin = tr_begin;
  r.blit_indexed = tr_blit_indexed;
  r.end = tr_end;

  delay = 0;

  rc = gif_init(&ctx, G_GIF_1X1, (gif_usize)sizeof(G_GIF_1X1), scratch, (gif_usize)sizeof(scratch));
  TEST_ASSERT_GIF(t, name, rc == GIF_SUCCESS, rc);

  rc = gif_next_frame_render(&ctx, &r, &delay);
  TEST_ASSERT_GIF(t, name, rc == GIF_SUCCESS, rc);

  TEST_ASSERT_GIF(t, name, tr.begin_called != 0, rc);
  TEST_ASSERT_GIF(t, name, tr.end_called != 0, rc);
  TEST_ASSERT_GIF(t, name, tr.blit_called != 0, rc);

  TEST_ASSERT_GIF(t, name, tr.bw == 1, rc);
  TEST_ASSERT_GIF(t, name, tr.bh == 1, rc);

  TEST_ASSERT_GIF(t, name, tr.last_x == 0, rc);
  TEST_ASSERT_GIF(t, name, tr.last_y == 0, rc);
  TEST_ASSERT_GIF(t, name, tr.last_w == 1, rc);
  TEST_ASSERT_GIF(t, name, tr.last_h == 1, rc);

  TEST_ASSERT_GIF(t, name, delay == 100, rc);
  TEST_ASSERT_GIF(t, name, tr.last_delay == 100, rc);

  rc = gif_close(&ctx);
  TEST_ASSERT_GIF(t, name, rc == GIF_SUCCESS, rc);
}

// -----------------------------
// COMPAT
// -----------------------------

static void test_compat_api(TestState *t)
{
  const char *name;
  GIF_Context ctx;
  gif_u8 scratch[GIF_SCRATCH_BUFFER_REQUIRED_SIZE];
  gif_u8 canvas[4 * (int)GIF_OUTPUT_BPP];
  int rc;
  int delay;

  name = "compat_api";
  t_print_header(name);

  memset(canvas, 0, sizeof(canvas));

  rc = gif_init(&ctx, G_GIF_1X1, (gif_usize)sizeof(G_GIF_1X1), scratch, (gif_usize)sizeof(scratch));
  TEST_ASSERT_GIF(t, name, rc == GIF_SUCCESS, rc);

  delay = 0;
  rc = gif_next_frame_compat(&ctx, canvas, &delay);
  TEST_ASSERT_GIF(t, name, rc == 1, rc);
  TEST_ASSERT_GIF(t, name, delay == 100, rc);

  delay = 0;
  rc = gif_next_frame_compat(&ctx, canvas, &delay);
  TEST_ASSERT_GIF(t, name, rc == 0, rc);

  rc = gif_close(&ctx);
  TEST_ASSERT_GIF(t, name, rc == GIF_SUCCESS, rc);
}

// -----------------------------
// MAIN
// -----------------------------

int main(void)
{
  int rc;
  TestState t;

  rc = 0;
  memset(&t, 0, sizeof(t));

  test_strings_and_constants(&t);
  test_init_errors(&t);
  test_static_decode_canvas(&t);
  test_static_decode_rect(&t);
  test_renderer_api(&t);
  test_compat_api(&t);

  printf("\n== SUMMARY ==\n");
  printf("total:  %d\n", t.total);
  printf("passed: %d\n", t.passed);
  printf("failed: %d\n", t.failed);

  if (t.failed != 0) {
    rc = 1;
  }

  return rc;
}
