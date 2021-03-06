#ifndef _Mikabox_coroutine_h_
#define _Mikabox_coroutine_h_

#ifdef __cplusplus
extern "C" {
#endif

// Implementation of a simple co-operative scheduler.

// co_next() may only be called on the main thread and
// executes a thread until it calls co_yield().

// If co_yield() is called on the main thread, co_next()
// will be called for each active thread instead.

#include <stdint.h>

struct reg_set {
  // AAPCS32 2019Q1.1, Ch. 6.1.1/6.1.2
  // ARM1176JZF-S has VFP-v2
  uint64_t d[8];
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
  uint32_t r8;
  uint32_t r9;
  uint32_t r10;
  uint32_t r11;
  uint32_t sp;
  uint32_t pc;
} __attribute__((packed, aligned(8)));

struct coroutine {
  struct reg_set regs;
  enum co_state {
    CO_STATE_NEW = 0,
    CO_STATE_RUN,
    CO_STATE_YIELD,
    CO_STATE_DONE
  } state;
  #define CO_FLAG_FPU   (1 << 0)
  #define CO_FLAG_USER  (1 << 1)
  uint8_t flags;
} __attribute__((aligned(8)));

void co_create(struct coroutine *co,
  void (*fn)(uint32_t), uint8_t flags, void *stack_end);
void co_start(struct coroutine *co, uint32_t arg);
void co_next(struct coroutine *co);
void co_yield();
void co_syscall_yield(struct reg_set *saved_regs);

#ifdef __cplusplus
}
#endif

#endif
