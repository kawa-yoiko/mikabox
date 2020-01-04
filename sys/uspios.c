#include "uspios.h"
#include "common.h"
#include "kmalloc.h"
#include "prop_tag.h"
#include "printf/printf.h"
#include "uspienv/timer.h"

/*
void *malloc(size_t size)
{
  return kmalloc(size);
}

void free(void *ptr)
{
  kfree(ptr);
}
*/

void uspi_assertion_failed(const char *pExpr, const char *pFile, unsigned nLine)
{
  // Dummy
}

unsigned StartKernelTimer(
  unsigned nHzDelay, TKernelTimerHandler *pHandler,
  void *pParam, void *pContext)
{
  // TODO
	unsigned r = TimerStartKernelTimer (TimerGet (), nHzDelay, pHandler, pParam, pContext);
  printf("%u <- %u | %p %p %p\n", r, nHzDelay, pHandler, pParam, pContext);
  return r;
}

void CancelKernelTimer(unsigned hTimer)
{
  // TODO
	TimerCancelKernelTimer (TimerGet (), hTimer);
}

/*
int SetPowerStateOn(unsigned nDeviceId)
{
  bool succeeded = set_power_state(nDeviceId, 3); // on | wait
  return succeeded;
}

int GetMACAddress(unsigned char Buffer[6])
{
  get_mac_addr(Buffer);
  return 1;
}
*/

void DebugHexdump(const void *pBuffer, unsigned nBufLen, const char *pSource)
{
  printf("%s: hex dump of %u byte%s\n",
    pSource, nBufLen, nBufLen == 1 ? "" : "s");
  for (size_t i = 0; i < nBufLen; i += 16) {
    printf("%p: ", pBuffer + i);
    for (size_t j = 0; j < 16; j++)
      printf("%02x%c", *(uint8_t *)(pBuffer + i + j),
        j == 7 ? '-' : (j == 15 ? '\n' : ' '));
  }
}
