#include "v3d.h"
#include "prop_tag.h"
#include "swi.h"
#include "printf/printf.h"
#include "regs.h"
#include <math.h>

static uint32_t ctx;
static uint32_t va1, va2, va3;
static uint32_t ua1, ua2, ua3;
static uint32_t sca, stca;
static uint32_t batch1, batch2, batch3;
static uint32_t idxs;
static uint32_t nanikore, checker;
static uint32_t target;

void donk()
{
  syscall(256 + 15, ctx);
  syscall(256 + 47, va1);
  syscall(256 + 47, va2);
  syscall(256 + 47, va3);
  syscall(256 + 63, ua1);
  syscall(256 + 63, ua2);
  syscall(256 + 63, ua3);
  syscall(256 + 79, sca);
  syscall(256 + 79, stca);
  syscall(256 + 95, batch1);
  syscall(256 + 95, batch2);
  syscall(256 + 95, batch3);
  syscall(256 + 111, idxs);
  syscall(256 + 31, nanikore);
  syscall(256 + 31, target);
}

void doda()
{
  ctx = syscall(256);

  static v3d_vert v = { .varyings = {0, 0, 0, 0, 0, 0} };

  va3 = syscall(256 + 32, 3, 4);
  v.x = 10.0f; v.y = 10.0f;
  v.varyings[0] = 0.5f; v.varyings[1] = 1.0f; v.varyings[2] = 1.0f; v.varyings[3] = 1.0f;
  syscall(256 + 33, va3, 0, (uint32_t)&v, 1);
  v.x = 10.0f; v.y = 470.0f;
  v.varyings[0] = 0.5f; v.varyings[1] = 0.25f; v.varyings[2] = 0.5f; v.varyings[3] = 0.5f;
  syscall(256 + 33, va3, 1, (uint32_t)&v, 1);
  v.x = 600.0f; v.y = 245.0f;
  v.varyings[0] = 0.5f; v.varyings[1] = 0.5f; v.varyings[2] = 0.25f; v.varyings[3] = 0.5f;
  syscall(256 + 33, va3, 2, (uint32_t)&v, 1);

  ua3 = syscall(256 + 48, 1);
  sca = syscall(256 + 64, (uint32_t)"#CA");
  batch3 = syscall(256 + 80, va3, ua3, sca);

#if 0
/*
  ctx = v3d_ctx_create();
  va1 = v3d_vertarr_create(4, 6);
  static v3d_vert v = { .varyings = {0, 0, 0, 0, 0, 0} };
*/
  va1 = syscall(256 + 32, 4, 6);
  v.x = 250.0f; v.y = 100.0f;
  v.varyings[0] = 0.5f;
  v.varyings[1] = 0.0f;
  v.varyings[2] = 0.3f;
  v.varyings[3] = 0.3f;
  v.varyings[4] = 0.3f;
  v.varyings[5] = 0.3f;
  //v3d_vertarr_put(&va1, 0, &v, 1);
  syscall(256 + 33, va1, 0, (uint32_t)&v, 1);
  v.x = 100.0f; v.y = 400.0f;
  v.varyings[0] = 0.0f;
  v.varyings[1] = 1.0f;
  v.varyings[2] = 0.3f;
  v.varyings[3] = 0.3f;
  v.varyings[4] = 0.3f;
  v.varyings[5] = 0.3f;
  //v3d_vertarr_put(&va1, 1, &v, 1);
  syscall(256 + 33, va1, 1, (uint32_t)&v, 1);
  v.x = 400.0f; v.y = 400.0f;
  v.varyings[0] = 1.0f;
  v.varyings[1] = 1.0f;
  v.varyings[2] = 0.3f;
  v.varyings[3] = 0.3f;
  v.varyings[4] = 0.3f;
  v.varyings[5] = 0.3f;
  //v3d_vertarr_put(&va1, 2, &v, 1);
  syscall(256 + 33, va1, 2, (uint32_t)&v, 1);
  v.x = 400.0f; v.y = 100.0f;
  v.varyings[0] = 1.0f;
  v.varyings[1] = 0.0f;
  v.varyings[2] = 0.3f;
  v.varyings[3] = 0.3f;
  v.varyings[4] = 0.3f;
  v.varyings[5] = 0.3f;
  //v3d_vertarr_put(&va1, 3, &v, 1);
  syscall(256 + 33, va1, 3, (uint32_t)&v, 1);

  uint16_t i[6] = {0, 1, 2, 1, 2, 3};
  idxs = syscall(256 + 96, 128);
  syscall(256 + 97, idxs, 0, (uint32_t)&i[0], 6);

  extern uint8_t _binary_utils_nanikore_bin_start;
  nanikore = syscall(256 + 16, 512, 256);
  //syscall(256 + 17, nanikore, (uint32_t)&_binary_utils_nanikore_bin_start, 0);

  ua1 = syscall(256 + 48, 3);
  syscall(256 + 50, ua1, 0, nanikore, 0);

  stca = syscall(256 + 64, (uint32_t)"#TCA");
  batch1 = syscall(256 + 80, va1, ua1, stca);

  target = syscall(256 + 16, 800, 480);

  va2 = syscall(256 + 32, 4, 6);
  for (uint8_t x = 0; x <= 1; x++)
  for (uint8_t y = 0; y <= 1; y++) {
    v.x = 300.0f + 400.0f * x; v.y = 20.0f + 240.0f * y;
    v.varyings[0] = x; v.varyings[1] = y;
    v.varyings[2] = 1.0f; v.varyings[3] = 1.0f; v.varyings[4] = 1.0f; v.varyings[5] = 1.0f;
    syscall(256 + 33, va2, x + x + y, (uint32_t)&v, 1);
  }

  ua2 = syscall(256 + 48, 2);
  syscall(256 + 50, ua2, 0, target, 0);
  batch2 = syscall(256 + 80, va2, ua2, stca);

/*
  ua1 = v3d_unifarr_create(3);

  batch1 = v3d_batch_create(va1, ua1, v3d_shader_create("#TCA"));

  idxs = v3d_mem_indexbuf(128);

  extern uint8_t _binary_utils_nanikore_bin_start;
  nanikore = v3d_tex_create(512, 256);
  v3d_tex_update(&nanikore, &_binary_utils_nanikore_bin_start, v3d_tex_fmt_rgb);

  va2 = v3d_vertarr_create(6, 6);
  v.varyings[2] = 1.0;
  v.varyings[3] = 1.0;
  v.varyings[4] = 1.0;
  v.varyings[5] = 1.0;
  for (uint8_t i = 0; i <= 1; i++) {
    v.x = 40.0f + i * 720;
    v.y = 40.0f + i * 400;
    v.varyings[0] = i - 0.3f; v.varyings[1] = i;
    v3d_vertarr_put(&va2, i * 3 + 0, &v, 1);
    v.x = 760.0f; v.y = 40.0f; v.varyings[0] = 0.7f; v.varyings[1] = 0.0f;
    v3d_vertarr_put(&va2, i * 3 + 1, &v, 1);
    v.x = 40.0f; v.y = 440.0f; v.varyings[0] = -0.3f; v.varyings[1] = 1.0f;
    v3d_vertarr_put(&va2, i * 3 + 2, &v, 1);
  }

  ua2 = v3d_unifarr_create(2);
  batch2 = v3d_batch_create(va2, ua2, v3d_shader_create("#TCA"));

#define cw 32
#define ch 64
  static uint8_t c[ch][cw][3];
  for (int i = 0; i < ch; i++)
    for (int j = 0; j < cw; j++)
      c[i][j][0] = c[i][j][1] = c[i][j][2] =
        ((i < 4 && j < 4) || ((i ^ j) & 4)) ? 0xff : 0xcc;

  checker = v3d_tex_create(cw, ch);
  v3d_tex_update(&checker, &c[0][0][0], v3d_tex_fmt_rgb);

  target = v3d_tex_create(800, 480);
  v3d_unifarr_puttex(&ua2, 0, target, v3d_magfilt_linear | v3d_wrap_s_mirror);

  va3 = v3d_vertarr_create(3, 4);
  v.x = 10.0f; v.y = 10.0f;
  v.varyings[0] = 0.5f; v.varyings[1] = 1.0f; v.varyings[2] = 1.0f; v.varyings[3] = 1.0f;
  v3d_vertarr_put(&va3, 0, &v, 1);
  v.x = 10.0f; v.y = 470.0f;
  v.varyings[0] = 0.5f; v.varyings[1] = 0.25f; v.varyings[2] = 0.5f; v.varyings[3] = 0.5f;
  v3d_vertarr_put(&va3, 1, &v, 1);
  v.x = 600.0f; v.y = 245.0f;
  v.varyings[0] = 0.5f; v.varyings[1] = 0.5f; v.varyings[2] = 0.25f; v.varyings[3] = 0.5f;
  v3d_vertarr_put(&va3, 2, &v, 1);

  ua3 = v3d_unifarr_create(1);
  v3d_unifarr_putf32(&ua3, 0, 0);
  batch3 = v3d_batch_create(va3, ua3, v3d_shader_create("#CA"));
*/
#endif
}

void dodo(uint32_t fb)
{
  syscall(256 + 5, ctx);

  uint32_t scr = syscall(256 + 18);
  syscall(256 + 1, ctx, scr, 0xffadbecf);

  syscall(256 + 2, ctx, batch3);
  syscall(256 + 3, ctx, 0, 3, 0);

  //syscall(256 + 2, ctx, batch2);
  //syscall(256 + 3, ctx, 1, 6, idxs);

  syscall(256 + 4, ctx);

/*
  // Render to texture
  v3d_ctx_wait(&ctx);
  v3d_ctx_anew(&ctx, target, 0x0);

  v3d_ctx_use_batch(&ctx, &batch1);

  v3d_call call = {
    .is_indexed = false,
    .num_verts = 3,
    .start_index = 0,
  };
  extern bool has_key;
  for (int i = 0; i < (has_key ? 10 : 2); i++)
    v3d_ctx_add_call(&ctx, &call);

  uint16_t i[3] = {1, 2, 3};
  v3d_mem_indexcopy(&idxs, 0, i, 3);
  v3d_ctx_add_call(&ctx, &(v3d_call){
    .is_indexed = true,
    .num_verts = 3,
    .indices = idxs
  });

  // Any change before issuing will apply
  static uint32_t count = 0;
  float angle = (float)(++count) / 180.0f * acosf(-1.0f);
  static v3d_vert v = { .varyings = {0, 0, 0, 0, 0, 0} };
  v.x = 400.0f + 20.0f * cosf(angle);
  v.y = 400.0f + 20.0f * sinf(angle);
  v.varyings[0] = 1.0f;
  v.varyings[1] = 1.0f;
  v.varyings[2] = 0.3f;
  v.varyings[3] = 0.3f;
  v.varyings[4] = 0.3f;
  v.varyings[5] = 0.3f;
  v3d_vertarr_put(&va1, 2, &v, 1);

  v3d_unifarr_puttex(&ua1, 0, nanikore, 0);
  v3d_unifarr_putf32(&ua1, 2, sinf(angle) * 0.5f + 0.5f);

  v3d_ctx_use_batch(&ctx, &batch3);
  v3d_ctx_add_call(&ctx, &(v3d_call){
    .is_indexed = false,
    .num_verts = 3,
    .start_index = 0
  });

  v3d_ctx_issue(&ctx);
  // Render to screen
  v3d_ctx_wait(&ctx);
  v3d_ctx_anew(&ctx, v3d_tex_screen(fb), 0xffadbecf);

  v3d_ctx_use_batch(&ctx, &batch2);
  v3d_ctx_add_call(&ctx, &(v3d_call){
    .is_indexed = false,
    .num_verts = 6,
    .start_index = 0,
  });

  v3d_ctx_issue(&ctx);
*/
}
