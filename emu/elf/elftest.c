#include <stdio.h>  // Ahhhhhhh~~~!!!
#include <stdlib.h>
#include <string.h>

#include "elf.h"

void elf_load_program(const elf_ehdr *ehdr, const elf_phdr *program)
{
}

void buf_get(void *from, void *to, uint32_t offs, uint32_t len)
{
  memcpy(to, (uint8_t *)from + offs, len);
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("Usage: %s <file>\n", argv[0]);
    return 0;
  }

  FILE *f = fopen(argv[1], "r");
  if (!f) {
    printf("Cannot open file %s\n", argv[1]);
    return 1;
  }

  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *buf = (char *)malloc(len);
  if (!buf) {
    printf("Cannot allocate memory for reading the entire file\n");
    return 2;
  }
  if (fread(buf, len, 1, f) != 1) {
    printf("Cannot read from file %s\n", argv[1]);
    return 3;
  }
  fclose(f);

  printf("File size %lu B\n", len);
  uint8_t ret = elf_load(len, buf_get, buf);
  printf("load_elf returns %d\n", ret);

  return 0;
}
