#include "main.h"
#include "common.h"
#include "charbuf.h"
#include "mmu.h"
#include "irq.h"
#include "prop_tag.h"
#include "v3d.h"

#include "printf/printf.h"
#include "ampi.h"
#include "ampienv.h"
#include "uspi.h"
#include "uspios.h"
#include "sdcard/sdcard.h"
#include "fatfs/ff.h"

#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI  3.1415926535897932384626433832795
#endif

extern unsigned char _bss_begin;
extern unsigned char _bss_end;
extern unsigned char _bss_ord_begin;
extern unsigned char _bss_ord_end;

struct framebuffer {
  uint32_t pwidth;
  uint32_t pheight;
  uint32_t vwidth;
  uint32_t vheight;
  uint32_t pitch;
  uint32_t bpp;
  uint32_t xoffs;
  uint32_t yoffs;
  uint32_t buf;
  uint32_t size;
};

static uint8_t *fb_bufs[BUF_COUNT];
uint8_t fb_bufid = 0;
uint8_t *fb_buf;
uint32_t fb_pitch;

void fb_flip_buffer()
{
  set_virtual_offs(0, SCR_H * fb_bufid);
  fb_bufid = (fb_bufid + 1) % BUF_COUNT;
  fb_buf = fb_bufs[fb_bufid];
}

void (*periodic)() = NULL;
uint32_t uspi_tick = 0;
void uspi_upd_timers();
static uint32_t z = 0;
extern uint32_t y;  // Error count

void timer2_callback(void *_unused)
{
  do *TMR_CS = 4; while (*TMR_CS & 4);
  uint32_t t = *TMR_CLO;
  t = t - t % 10000 + 10000;
  *TMR_C2 = t;

  // printf("Timer!\n");
  if (periodic) periodic();
  uspi_tick++;
  uspi_upd_timers();

  static uint32_t count = 0;
  if (++count == 100) {
    count = 0;
    printf("\n%u %u\n", z, y);
    z = 0;
    y = (y <= 5 ? 0 : y - 5);
  }
}

void timer3_callback(void *ret_addr)
{
  do *TMR_CS = 8; while (*TMR_CS & 8);
  uint32_t t = *TMR_CLO;
  t = t - t % 500000 + 500000;
  *TMR_C3 = t;

  //printf("%u: %p\n", t, ret_addr);
  mem_barrier();
  static bool on = false;
  on = !on;
  mem_barrier();
  *GPFSEL4 |= (1 << 21);
  if (on) *GPCLR1 = (1 << 15);
  else *GPSET1 = (1 << 15);
  mem_barrier();
}

static volatile int frame_count = 0;

void vsync_callback(void *_unused)
{
  *(volatile uint32_t *)(PERI_BASE + 0x600000) = 0;
/*
  ctx.bufaddr = (uint32_t)fb_buf;
  v3d_op(&ctx);
  fb_flip_buffer();
  frame_count++;
*/
  charbuf_flush();
  fb_flip_buffer();
}

static bool has_key = false;

static inline int16_t myrand()
{
  static int seed = 1;
  seed = seed * 1103515245 + 12345;
  return (seed & 0xffff);
}

static int16_t wavetable[256];

static unsigned synth(int16_t *buf, unsigned chunk_size)
{
  //for (uint32_t i = 0; i < 1000000; i++) __asm__ __volatile__ ("");
  static uint8_t phase = 0;
  for (unsigned i = 0; i < chunk_size; i += 2) {
    //int16_t sample = (has_key ? (int16_t)(32767.0 / 2 * (sin(phase / 255.0 * M_PI * 2) + sin(phase / 127.5 * M_PI * 2))) : 0);
    int16_t sample = (has_key ? wavetable[phase] : 0);
    //int16_t sample = wavetable[i];
    buf[i] = buf[i + 1] = sample;
    //phase += 2; // Folds over to 0 ~ 255, generates 344.5 Hz (F4 - ~1/4 semitone)
    phase++;
  }
  return chunk_size;
}

static void kbd_upd_callback(uint8_t mod, const uint8_t k[6])
{
  printf("\r%02x %02x %02x %02x %02x %02x | mod = %02x",
    k[0], k[1], k[2], k[3], k[4], k[5], mod);
  has_key = (k[0] || k[1] || k[2] || k[3] || k[4] || k[5]);
}

static void gpad_upd_callback(unsigned index, const USPiGamePadState *state)
{
  printf("\r%d %08x", state->nbuttons, state->buttons);
  has_key = (state->buttons & 0x800);
}

void doda();
void dodo(uint32_t fb);

void sys_main()
{
  for (uint8_t *p = &_bss_begin; p < &_bss_end; p++) *p = 0;
  for (uint8_t *p = &_bss_ord_begin; p < &_bss_ord_end; p++) *p = 0;

  uint32_t bss_ord_page_begin = (uint32_t)&_bss_ord_begin >> 20;
  uint32_t bss_ord_page_end = (uint32_t)(&_bss_ord_end - 1) >> 20;

  for (uint32_t i = 0; i < 4096; i++)
    mmu_table_section(mmu_table, i << 20, i << 20, (i < 64 ? (8 | 4) : 0));
  for (uint32_t i = bss_ord_page_begin; i <= bss_ord_page_end; i++)
    mmu_table_section(mmu_table, i << 20, i << 20, 0);
  mmu_enable(mmu_table);

  mem_barrier();
  *TMR_CS = 8 | 4;
  *TMR_C2 = *TMR_CLO + 1000000;
  *TMR_C3 = *TMR_CLO + 1000000;
  irq_set_callback(2, timer2_callback, NULL);
  //irq_set_callback(3, timer3_callback, NULL);

  mem_barrier();
  struct framebuffer *f = mmu_ord_alloc(sizeof(struct framebuffer), 16);
  memset(f, 0, sizeof(struct framebuffer));
  f->pwidth = SCR_W; f->pheight = SCR_H;
  f->vwidth = SCR_W; f->vheight = SCR_H * BUF_COUNT;
  f->bpp = 32;
  // 1: channel for framebuffer
  send_mail(((uint32_t)f + 0x40000000) >> 4, 1);
  recv_mail(1);

  uint8_t *base = (uint8_t *)(f->buf);
  fb_pitch = f->pitch;

  for (uint32_t i = 0; i < BUF_COUNT; i++)
    fb_bufs[i] = base + fb_pitch * f->pheight * i;
  fb_buf = fb_bufs[0];
  mmu_ord_pop();  // f

  mem_barrier();
  charbuf_init(SCR_W, SCR_H);
  printf("Hello world!\n");
  printf("ARM clock rate: %u\n", get_clock_rate(3));

  //irq_set_callback(48, vsync_callback, NULL);

  sdInit();
  int32_t i = sdInitCard();
  printf("sdInitCard() returns %d\n", i);

  // MBR
  // Should end with 55aa
  uint8_t carddata[512] = { 0 };
  i = sdTransferBlocks(0x0LL, 1, carddata, 0);
  printf("sdTransferBlocks() returns %d\n", i);
  for (uint32_t j = 432; j < 512; j += 16) {
    printf("\n%3x | ", j);
    for (uint8_t k = 0; k < 16; k++)
      printf("%2x", carddata[j + k]);
  }
  _putchar('\n');

  FATFS fs;
  DIR dir;
  FILINFO finfo;
  FRESULT fr;

  fr = f_mount(&fs, "", 1);
  printf("f_mount() returned %d\n", (int32_t)fr);
  fr = f_opendir(&dir, "/");
  printf("f_opendir() returned %d\n", (int32_t)fr);
  while (1) {
    fr = f_readdir(&dir, &finfo);
    if (fr != FR_OK || finfo.fname[0] == 0) break;
    printf("%4d.%2d.%2d %2d:%2d:%2d  ",
      ((finfo.fdate >> 9) & 127) + 1980,
      (finfo.fdate >> 5) & 15,
      (finfo.fdate >> 0) & 15,
      (finfo.ftime >> 11) & 31,
      (finfo.ftime >> 5) & 63,
      (finfo.ftime >> 0) & 31);
    printf("%s%s\n", finfo.fname, (finfo.fattrib & AM_DIR) ? "/" : "");
  }
  f_closedir(&dir);

  uint8_t mac[6];
  get_mac_addr(mac);
  printf("MAC address: %02x %02x %02x %02x %02x %02x\n",
    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  y = 60; // Trigger a USB initialization

  for (uint32_t i = 0; i < 256; i++) {
    wavetable[i] = (int16_t)(sin((double)i / 128 * M_PI * 2) * 32767);
  }

  mem_barrier();
  AMPiInitialize(44100, 1764);  // 20 ms latency/block size
  AMPiSetChunkCallback(synth);
  bool b = AMPiStart();
  printf(b ? "Yes\n" : "No\n");

  mem_barrier();
  doda();

  int p = 0;
  while (1) {
    //printf(AMPiIsActive() ? "\rActive  " : "\rInactive");
    AMPiPoke();
    for (uint32_t i = 0; i < 100000; i++) __asm__ __volatile__ ("");
    z++;
    if (p) p++;
    if (y >= 60 || p >= 4000) {
      y = 0;
      USPiDeinitialize();
      bool result = USPiInitialize();
      printf("USPi initialization %s\n", result ? "succeeded" : "failed");

      printf("Keyboard %savailable\n", USPiKeyboardAvailable() ? "" : "un");
      if (USPiKeyboardAvailable())
        USPiKeyboardRegisterKeyStatusHandlerRaw(kbd_upd_callback);

      printf("Gamepad %savailable\n", USPiGamePadAvailable() ? "" : "un");
      if (USPiGamePadAvailable())
        USPiGamePadRegisterStatusHandler(gpad_upd_callback);

      p = (!USPiKeyboardAvailable() && !USPiGamePadAvailable());
    }

    dodo((uint32_t)fb_buf);
    fb_flip_buffer();
  }

/*
  mem_barrier();
  doda();

  mem_barrier();
  printf("All done batman, we have triangles!\n");

  uint32_t t = *TMR_CLO;
  uint32_t frame_count = 0;
  uint32_t seconds = 0;
  do {
    mem_barrier();
    dodo((uint32_t)fb_buf);
    fb_flip_buffer();
    frame_count++;
    mem_barrier();
    uint32_t t1 = *TMR_CLO;
    if (t1 > t + (seconds + 1) * 1000000) {
      seconds++;
      printf("%2u s: %4u frames\n", seconds, frame_count);
    }
  } while (seconds < 7);

  mem_barrier();
  printf("Frames: %d (%.2f FPS)\n", frame_count, frame_count / 15.0f);

  mem_barrier();
  irq_set_callback(48, vsync_callback, NULL);

  mem_barrier();
  *GPFSEL4 |= (1 << 21);
  while (1) {
    *GPCLR1 = (1 << 15);
    for (uint32_t i = 0; i < 100000000; i++) __asm__ __volatile__ ("");
    *GPSET1 = (1 << 15);
    for (uint32_t i = 0; i < 100000000; i++) __asm__ __volatile__ ("");
  }
*/

/*
  AMPiInitialize(44100, 4000);
  AMPiSetChunkCallback(synth);
  bool b = AMPiStart();
  printf(b ? "Yes\n" : "No\n");
  while (1) {
    printf(AMPiIsActive() ? "\rActive  " : "\rInactive");
    if (!AMPiIsActive()) {
      MsDelay(1000);
      AMPiStart();
    }
    MsDelay(20);
    AMPiPoke();
  }


  mem_barrier();
  *GPFSEL4 |= (1 << 21);
  uint8_t count = 99;
  while (count > 0) {
    printf("\n");
    printf("%u bottle%s of beer on the wall\n", count, count == 1 ? "" : "s");
    printf("%u bottle%s of beer\n", count, count == 1 ? "" : "s");

    mem_barrier();
    *GPCLR1 = (1 << 15);
    for (uint32_t i = 0; i < 100000000; i++) __asm__ __volatile__ ("");

    printf("Take one down, pass it around\n");
    count--;
    printf("%u bottle%s of beer on the wall\n", count, count == 1 ? "" : "s");

    *GPSET1 = (1 << 15);
    for (uint32_t i = 0; i < 100000000; i++) __asm__ __volatile__ ("");
  }
*/
}
