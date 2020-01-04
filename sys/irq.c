#include "irq.h"
#include "common.h"
#include "printf/printf.h"
#include <stddef.h>

#define IRQ_MAX 72
static irq_callback_t callbacks[IRQ_MAX] = { NULL };
static void *args[IRQ_MAX] = { NULL };

void irq_handler(uint32_t ret_addr)
{
  // Check interrupt source
  static int count = 0;
  if (++count == 20) { printf("IRQ!\n"); count = 0; }
/*
  mem_barrier();
  uint32_t pend_base = *IRQ_PENDBASIC;
  uint32_t pend_1 = *IRQ_PEND1;
  uint32_t pend_2 = *IRQ_PEND2;
  uint8_t source;
  if (pend_1) {
    source = 0 + __builtin_ctz(pend_1);
  } else if (pend_2) {
    source = 32 + __builtin_ctz(pend_2);
  } else if (pend_base & 0xff) {
    source = 64 + __builtin_ctz(pend_base & 0xff);
  } else {
    // Should not reach here
    return;
  }
  mem_barrier();

  if (source == 9) printf("IRQ %u\n", source);
  else if (pend_1 & (1 << 9)) printf("OvO\n");
  if (callbacks[source]) {
  printf("IRQ %u\n", source);
  charbuf_flush();
  fb_flip_buffer();
    callbacks[source](args[source] != NULL ? args[source] : (void *)ret_addr);
    mem_barrier();
  }
*/
}

void irq_set_callback(uint8_t source, irq_callback_t fn, void *arg)
{
  mem_barrier();
  if (source >= IRQ_MAX) return;
  if (fn != NULL) {
    if (source < 32) *IRQ_ENAB1 = (1 << source);
    else if (source < 64) *IRQ_ENAB2 = (1 << (source - 32));
    else *IRQ_ENABBASIC = (1 << (source - 64));
  } else {
    if (source < 32) *IRQ_DISA1 = (1 << source);
    else if (source < 64) *IRQ_DISA2 = (1 << (source - 32));
    else *IRQ_DISABASIC = (1 << (source - 64));
  }
  callbacks[source] = fn;
  args[source] = arg;
  mem_barrier();
}
