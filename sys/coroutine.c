#include "coroutine.h"
#include "printf/printf.h"

#define MAX_RECURSION 16
static struct coroutine co_toplevel = {
  .state = CO_STATE_RUN,
  .flags = 0
};
struct coroutine *stack[MAX_RECURSION] = { &co_toplevel };
uint16_t stack_top = 0;  // Points directly to the topmost level

void co_jump_arg(struct reg_set *save_regs, struct reg_set *load_regs, uint16_t flags, uint32_t arg);

void co_jump(struct reg_set *save_regs, struct reg_set *load_regs, uint16_t flags);

#define catflags(_from, _to) (((uint16_t)(_from) << 8) | (_to))

void regs_load(struct reg_set *load_regs);

void co_create(struct coroutine *co, void (*fn)(uint32_t))
{
  co->state = CO_STATE_NEW;
  co->regs.sp = (uint32_t)&co->stack[0] + CO_STACK;
  co->regs.pc = (uint32_t)fn;
}

void co_start(struct coroutine *co, uint32_t arg)
{
  if (co->state == CO_STATE_DONE || co->state == CO_STATE_RUN) {
    return;
  } else if (co->state == CO_STATE_YIELD) {
    co_next(co);
    return;
  }
  co->state = CO_STATE_RUN;
  stack[++stack_top] = co;
  co_jump_arg(
    &stack[stack_top - 1]->regs, &co->regs,
    catflags(stack[stack_top - 1]->flags, co->flags), arg);
}

void co_next(struct coroutine *co)
{
  if (co->state == CO_STATE_DONE || co->state == CO_STATE_RUN) {
    return;
  } else if (co->state == CO_STATE_NEW) {
    co_start(co, 0);
    return;
  }
  co->state = CO_STATE_RUN;
  stack[++stack_top] = co;
  co_jump(
    &stack[stack_top - 1]->regs, &co->regs,
    catflags(stack[stack_top - 1]->flags, co->flags));
}

void co_yield()
{
  if (stack_top == 0) return;
  stack_top--;
  stack[stack_top + 1]->state = CO_STATE_YIELD;
  co_jump(
    &stack[stack_top + 1]->regs, &stack[stack_top]->regs,
    catflags(stack[stack_top + 1]->flags, stack[stack_top]->flags));
}

void co_syscall_yield(struct reg_set *saved_regs)
{
  if (stack_top == 0) return;
  stack[stack_top]->regs = *saved_regs;
  stack_top--;
  stack[stack_top + 1]->state = CO_STATE_YIELD;
  __asm__ __volatile__ ("cpsie i");
  regs_load(&stack[stack_top]->regs);
}

// Exported for use in coroutine.S
/*
void co_syscall_done()
{
  stack_top--;
  stack[stack_top + 1]->state = CO_STATE_DONE;
  // Re-enable interrupts and simply load register bank
  __asm__ __volatile__ ("cpsie i");
  regs_load(&stack[stack_top]->regs);
}
*/
void co_done()
{
  stack_top--;
  stack[stack_top + 1]->state = CO_STATE_DONE;
  // Perform a normal jump
  // XXX: Can also be a simple load of registers
  co_jump(
    &stack[stack_top + 1]->regs, &stack[stack_top]->regs,
    catflags(stack[stack_top + 1]->flags, stack[stack_top]->flags));
}
