.global co_jump_arg
co_jump_arg:
  ldr       ip, =#0x101 // CO_FLAG_FPU for both
  tst       r2, ip
  fstmiadne r0!, {d8-d15}
  addeq     r0, #64
  stmia     r0!, {r4-r11, sp, lr}
  mov       r0, r3
  ldr       lr, =co_done
  fldmiadne r1!, {d8-d15}
  addeq     r1, #64
  ldmia     r1!, {r4-r11, sp, pc}

.global co_jump
co_jump:
  ldr       ip, =#0x101 // CO_FLAG_FPU for both
  tst       r2, ip
  fstmiadne r0!, {d8-d15}
  addeq     r0, #64
  stmia     r0!, {r4-r11, sp, lr}
  fldmiadne r1!, {d8-d15}
  addeq     r1, #64
  tst       r2, #2      // to CO_FLAG_USER
  ldmeqia   r1!, {r4-r11, sp, pc}
  // Enter user mode
  ldmia     r1!, {r4-r11}
  ldmia     r1!, {r0, r2}
  b         _text_user_vaddr  // user.S

// Saves all registers, including VFP ones and lr
// ip: Address to struct reg_set
.global regs_save
regs_save:
  fstmiad   ip!, {d8-d15}
  stmia     ip!, {r4-r11, sp, lr}
  b         _swi_stub_cont

// Does not change mode
.global regs_load
regs_load:
  fldmiad   r0!, {d8-d15}
  ldmia     r0!, {r4-r11, sp, pc}
