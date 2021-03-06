#include "mikabox.h"
#include "syscalls.h"
#include <math.h>

extern unsigned char _bss_begin;
extern unsigned char _bss_end;

uint8_t qwq[1024];
uint8_t qvq[] = "=~=";
uint8_t quq = { 0 };

uint32_t audio_blocksize;

static uint32_t buttons;

__attribute__ ((noinline)) void crt_init()
{
  unsigned char *begin = &_bss_begin, *end = &_bss_end;
  while (begin < end) *begin++ = 0;
}

void draw()
{
  // Create context
  int ctx = gfx_ctx_create();

  // Create vertex arrays, uniform arrays and shader
  int va1, ua1, sh1;
  va1 = gfx_varr_create(6, 3);
  ua1 = gfx_uarr_create(0);
  sh1 = gfx_shad_create("#C");
  int va2, ua2, sh2;
  va2 = gfx_varr_create(6, 4);
  ua2 = gfx_uarr_create(0);
  sh2 = gfx_shad_create("#CA");
  int va3, ua3, sh3, sh3t, tex3;
  va3 = gfx_varr_create(6, 6);
  ua3 = gfx_uarr_create(2);
  sh3 = gfx_shad_create("#TCA");
  sh3t = gfx_shad_create("#T");
#define tw 96
#define th 64
  tex3 = syscall(256 + 16, tw, th);

  int ia = gfx_iarr_create(3);

  // Create batch
  int bat1, bat2, bat3, bat3t;
  bat1 = gfx_bat_create(va1, ua1, sh1);
  bat2 = gfx_bat_create(va2, ua2, sh2);

  // Close and reopen
  for (int i = 0; i < 20; i++) {
    gfx_varr_close(va2);
    gfx_uarr_close(ua2);
    gfx_shad_close(sh2);
    gfx_iarr_close(ia);
    gfx_bat_close(bat1);
    gfx_bat_close(bat2);
    va2 = gfx_varr_create(6, 4);
    ua2 = gfx_uarr_create(0);
    sh2 = gfx_shad_create("#CA");
    ia = gfx_iarr_create(3);
    bat1 = gfx_bat_create(va1, ua1, sh1);
    bat2 = gfx_bat_create(va2, ua2, sh2);
  }

  // Set up texture and batch 3
  static uint8_t z[th][tw][4];
  for (int i = 0; i < th; i++)
    for (int j = 0; j < tw; j++) {
      z[i][j][1] = 0xdd;
      z[i][j][2] = 0xdd - i;
      z[i][j][3] = 0xdd - j;
      z[i][j][0] = 0xdd;
    }
  gfx_tex_update(tex3, z, v3d_tex_fmt_argb);
  syscall(256 + 50, ua3, 0, tex3, (2 << 0) | (2 << 2));
  bat3 = gfx_bat_create(va3, ua3, sh3);
  bat3t = gfx_bat_create(va3, ua3, sh3t);

  // Populate index buffer
  uint16_t idxs[3] = {0, 1, 2};
  gfx_iarr_put(ia, 0, idxs, 3);

  float attr[8];
  attr[3] = 1.0;
  attr[4] = 0.8;
  for (int i = 0; i <= 1; i++) {
    attr[0] = (i == 0 ? 0.75 : 0.25) * 800;
    attr[1] = (i == 0 ? 0.75 : 0.25) * 480;
    attr[2] = 0.9;
    gfx_varr_put(va1, i * 3 + 0, &attr[0], 1);
    attr[0] = 0.75 * 800;
    attr[1] = 0.25 * 480;
    attr[2] = 0.7;
    gfx_varr_put(va1, i * 3 + 1, &attr[0], 1);
    attr[0] = 0.25 * 800;
    attr[1] = 0.75 * 480;
    attr[2] = 0.5;
    gfx_varr_put(va1, i * 3 + 2, &attr[0], 1);
  }

  for (int i = 0; i <= 1; i++) {
    attr[0] = (i == 0 ? 0.25 : 0.75) * 800;
    attr[1] = (i == 0 ? 0.25 : 0.75) * 480;
    attr[2] = 1.0;
    attr[3] = 0.5;
    attr[4] = 0.9;
    attr[5] = 1.0;
    gfx_varr_put(va2, i * 3 + 0, &attr[0], 1);
    attr[0] = 0.9 * 800;
    attr[1] = 0.4 * 480;
    attr[2] = 0.7 * 0.3;
    attr[3] = 0.5 * 0.3;
    attr[4] = 0.9 * 0.3;
    attr[5] = 0.3;
    gfx_varr_put(va2, i * 3 + 1, &attr[0], 1);
    attr[0] = 0.4 * 800;
    attr[1] = 0.9 * 480;
    attr[2] = 0.5 * 0.3;
    attr[3] = 0.5 * 0.3;
    attr[4] = 0.9 * 0.3;
    attr[5] = 0.3;
    gfx_varr_put(va2, i * 3 + 2, &attr[0], 1);
  }

  for (int i = 0; i <= 1; i++) {
    attr[0] = (i == 0 ? 0.8 : 0.95) * 800;
    attr[1] = (i == 0 ? 0.8 : 0.95) * 480;
    attr[2] = attr[3] = (i == 0 ? -1 : +2);
    attr[4] = attr[5] = attr[6] = attr[7] = 1.0;
    gfx_varr_put(va3, i * 3 + 0, &attr[0], 1);
    attr[0] = 0.8 * 800;
    attr[1] = 0.95 * 480;
    attr[2] = -1; attr[3] = 2;
    gfx_varr_put(va3, i * 3 + 1, &attr[0], 1);
    attr[0] = 0.95 * 800;
    attr[1] = 0.8 * 480;
    attr[2] = 2; attr[3] = -1;
    attr[4] = 0.05;
    attr[5] = attr[6] = 0.1;
    attr[7] = 0.1;
    gfx_varr_put(va3, i * 3 + 2, &attr[0], 1);
  }

  while (1) {
    // Wait
    syscall(256 + 5, ctx);

    // Reset
    uint64_t t = syscall64(4, 0);
    syscall(256 + 1, ctx, syscall(256 + 18),
      t == 0 ? 0xffffffff : 0xffffddcc);

    // Use batch 1 and add call
    gfx_ctx_batch(ctx, bat1);
    gfx_ctx_call(ctx, 0, 3, t == 0 ? 0 : 3);

    // Use batch 2 and add call
    gfx_ctx_batch(ctx, bat2);
    idxs[0] = (t == 0 ? 1 : 3);
    gfx_iarr_put(ia, 1, idxs, 1);
    gfx_ctx_call(ctx, 1, 3, ia);

    // Use batch 3 and add call
    uint64_t tick = syscall64(2);
    gfx_ctx_batch(ctx, tick % 1000000 < 500000 ? bat3 : bat3t);
    gfx_ctx_call(ctx, 0, 6, 0);

    // Issue and wait
    gfx_ctx_issue(ctx);
    mika_yield(1);
  }
}

void synth()
{
  while (1) {
    static int16_t p[8192];
    static uint32_t q = 0;
    float freq = 440.0f * powf(2.0f, __builtin_popcount(buttons) / 12.0f);
    for (int i = 0; i < audio_blocksize; i++) {
      p[i * 2] = p[i * 2 + 1] =
        (int16_t)(sinf(q / 44100.0f * freq * 2 * acosf(-1)) * 16384.0);
      q++;
    }
    aud_write(p);
    mika_yield(1);
  }
}

void event()
{
  while (1) {
    buttons = mika_btns(0);
    mika_yield(1);
  }
}

void update()
{
  char s[17] = { 0 };
  while (1) {
    mika_yield(1);
    /*uint64_t t = syscall64(4, 0);
    for (int j = 15; j >= 0; j--) {
      s[j] = "0123456789abcdef"[t & 0xf];
      t >>= 4;
    }
    mika_log(0, s);*/
    //for (int i = 0; i < 3e5; i++) __asm__ __volatile__ ("");
    //for (int i = 0; i < 1e4; i++) mika_rand();
  }
}

void main()
{
  crt_init();

/*
  int j = 0;
  while (1) {
    for (int i = 0; i < 500000; i++) __asm__ __volatile__ ("");
    if (++j == 100) {
        mika_log(0, "Hello world!\n");
        j = 0;
    }
    mika_yield(1);
  }
*/

  uint32_t f;
  // Write
  f = fil_open("haha.txt", 0x02 | 0x08);
  fil_write(f, "huhuhu", 6);
  fil_seek(f, fil_tell(f) - 1);
  fil_trunc(f);
  fil_close(f);

  // Read
  char buf[32] = { 0 };
  f = fil_open("haha.txt", 0x01);
  fil_read(f, buf, 1);
  mika_log(0, buf);
  uint32_t sz = fil_size(f);
  fil_read(f, buf, 1);
  mika_log(0, buf);
  fil_seek(f, sz - 2);
  fil_read(f, buf, sizeof(buf) - 1);
  mika_log(0, buf);

  // Rename
  fil_rename("haha.txt", "huhu.txt");
  // Make directory
  fil_mkdir("ahahaa");
  fil_mkdir("ahahab");
  // Stat
  buf[1] = '\0';
  buf[0] = "-FD"[fil_stat("ahahaa")]; mika_log(0, buf);
  buf[0] = "-FD"[fil_stat("huhu.txt")]; mika_log(0, buf);
  // - Different behaviour: trailing slashes should be accepted on hardware
  buf[0] = "-FD"[fil_stat("ahahaa/")]; mika_log(0, buf);
  buf[0] = "-FD"[fil_stat("zzzz")]; mika_log(0, buf);

  // Open directory
  // - Both should succeed
  fil_opendir("ahahaa");
  mika_log(0, "----");
  uint32_t d = fil_opendir("/");
  uint32_t ty;
  while ((ty = fil_readdir(d, buf)) != 0) {
    if (ty == 2) mika_log(0, "> dir");
    mika_log(0, buf);
  }
  fil_closedir(d);
  mika_log(0, "----");

  // Unlink
  syscall(512 + 33, "ahahaa");
  syscall(512 + 33, "huhu.txt");
  syscall(512 + 33, "zzzz");

  audio_blocksize = aud_blocksize();

  syscall(0, draw, synth, event, update);
  mika_yield(1);
}
