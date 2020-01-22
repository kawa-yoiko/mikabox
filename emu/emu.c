#include "elf.h"
#include "swi.h"
#include "syscalls.h"

#define GLEW_STATIC
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "unicorn/unicorn.h"

#include <stdio.h>

#define MEM_START   0x80000000
#define MEM_SIZE    0x18000000  // 384 MiB
#define MEM_END     (MEM_START + MEM_SIZE)
#define PAGE_SIZE   0x100000

#undef MEM_SIZE
#define MEM_SIZE    0x1000000   // 16 MiB

#define WIN_W 800
#define WIN_H 480

// GLFW

static GLFWwindow *window; 

static void glfw_err_callback(int error, const char *desc)
{
  printf("GLFW errors with code %d (%s)\n", error, desc);
}

static void glfw_fbsz_callback(GLFWwindow *window, int w, int h)
{
  glViewport(0, 0, w, h);
}

void setup_glfw()
{
  if (!glfwInit()) {
    printf("Cannot initialize GLFW\n");
    exit(1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

  window = glfwCreateWindow(WIN_W, WIN_H, "Mikabox Emulator", NULL, NULL);
  if (window == NULL) {
    printf("Cannot create GLFW window\n");
    exit(1);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    printf("Cannot initialize GLEW\n");
    exit(1);
  }
}

// Unicorn Engine

static uc_engine *uc;

static inline uint32_t align(uint32_t addr, uint32_t align)
{
  return (addr + align - 1) & ~(align - 1);
}

static void *alloc(elf_word vaddr, elf_word memsz, elf_word flags)
{
  void *p = malloc(memsz);

  uint32_t perms = 0;
  if (flags & 4) perms |= UC_PROT_READ;
  if (flags & 2) perms |= UC_PROT_WRITE;
  if (flags & 1) perms |= UC_PROT_EXEC;

  uc_err err;
  if ((err = uc_mem_unmap(uc, vaddr, align(memsz, PAGE_SIZE))) != UC_ERR_OK) {
    printf("uc_mem_unmap() returned error %u (%s)\n", err, uc_strerror(err));
    return NULL;
  }
  if ((err = uc_mem_map_ptr(
      uc, vaddr, align(memsz, PAGE_SIZE), perms, p)) != UC_ERR_OK) {
    printf("uc_mem_map_ptr() returned error %u (%s)\n", err, uc_strerror(err));
    return NULL;
  }

  return p;
}

static void fp_get(void *user, void *dest, uint32_t offs, uint32_t len)
{
  FILE *fp = user;
  fseek(fp, offs, SEEK_SET);
  fread(dest, len, 1, user);
}

static inline const char *mem_type_str(uc_mem_type t)
{
  switch (t) {
    case UC_MEM_READ: return "read";
    case UC_MEM_WRITE: return "write";
    case UC_MEM_FETCH: return "fetch";
    case UC_MEM_READ_UNMAPPED: return "read unmapped";
    case UC_MEM_WRITE_UNMAPPED: return "write unmapped";
    case UC_MEM_FETCH_UNMAPPED: return "fetch unmapped";
    case UC_MEM_READ_PROT: return "read protected";
    case UC_MEM_WRITE_PROT: return "write protected";
    case UC_MEM_FETCH_PROT: return "fetch protected";
    default: return "--";
  }
}

static void handler_unmapped(
  uc_engine *uc, uc_mem_type type,
  uint64_t address, int size, int64_t value, void *user_data)
{
  if (type == UC_MEM_FETCH_UNMAPPED && address == 0) {
    printf("Done!\n");
    return;
  }
  printf("Invalid memory access 0x%08x (type = %s, value = 0x%08x)\n",
    (uint32_t)address, mem_type_str(type), (uint32_t)value);
}

void emu()
{
  uc_err err;

  // Initialize Unicorn
  if ((err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc)) != UC_ERR_OK) {
    printf("uc_open() returned error %u (%s)\n", err, uc_strerror(err));
    return;
  }

  // Map memory
  if ((err = uc_mem_map(
      uc, MEM_START, MEM_SIZE, UC_PROT_READ | UC_PROT_WRITE)) != UC_ERR_OK) {
    printf("uc_mem_map() returned error %u (%s)\n", err, uc_strerror(err));
    return;
  }

  // Parse and load ELF
  FILE *fp = fopen("user/a.out", "r");
  if (fp == NULL) {
    printf("Unable to open file\n");
    return;
  }
  elf_addr entry;
  uint8_t elfret = elf_load(fp_get, alloc, fp, &entry);
  fclose(fp);
  if (elfret != 0) {
    printf("elf_load() returned error %u\n", elfret);
    return;
  }

  // Add hooks
  uc_hook hook_mem, hook_syscall;
  uc_hook_add(uc, &hook_mem, UC_HOOK_MEM_INVALID, handler_unmapped, NULL, 1, 0);
  uc_hook_add(uc, &hook_syscall, UC_HOOK_INTR, handler_syscall, NULL, 1, 0);

  // Initialize syscalls
  syscalls_init();

  // Initialize stack pointer
  uint32_t initial_sp = MEM_END;
  uint32_t initial_lr = 0x0;
  uc_reg_write(uc, UC_ARM_REG_SP, &initial_sp);
  uc_reg_write(uc, UC_ARM_REG_LR, &initial_lr);

  // Execute
  printf("Entry 0x%08x\n", entry);
  if ((err = uc_emu_start(uc, entry, 0, 1000000, 0)) != UC_ERR_OK) {
    printf("uc_emu_start() returned error %u (%s)\n", err, uc_strerror(err));
    return;
  }
}

int main()
{
  setup_glfw();
  emu();
  return 0;
}
