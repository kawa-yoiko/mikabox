#include "audio.h"
#include "printf/printf.h"
#include <stddef.h>
#include <string.h>

static bool request_pending = true;
static uint32_t dropped = 0;

static int16_t buffer[2048];

uint32_t audio_blocksize()
{
  return 1764 / 2;
}

bool audio_pending()
{
  return request_pending;
}

void audio_clear_pending()
{
  request_pending = false;
}

uint32_t audio_dropped()
{
  uint32_t ret = dropped;
  dropped = 0;
  return ret;
}

void *audio_write_pos()
{
  if (!request_pending) {
    printf("Already written\n");
    return NULL;
  }
  request_pending = false;
  return (void *)buffer;
}

unsigned audio_callback(int16_t **o_buf, unsigned chunk_size)
{
  if (request_pending) dropped++;
  else request_pending = true;
  *o_buf = buffer;
  return chunk_size;
}
