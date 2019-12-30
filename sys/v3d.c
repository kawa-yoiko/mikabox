#include "v3d.h"
#include "main.h"
#include "common.h"
#include "prop_tag.h"
#include "printf/printf.h"

#define GPU_BUS_ADDR  0x40000000  // TODO: Unify all occurrences

#define v3d_reg(_offs) (volatile uint32_t *)(PERI_BASE + 0xc00000 + (_offs))

#define V3D_IDENT0  v3d_reg(0x000)  // V3D Identification 0 (V3D block identity)
#define V3D_IDENT1  v3d_reg(0x004)  // V3D Identification 1 (V3D Configuration A)
#define V3D_IDENT2  v3d_reg(0x008)  // V3D Identification 2 (V3D Configuration B)

#define V3D_SCRATCH v3d_reg(0x010)  // Scratch Register

#define V3D_L2CACTL v3d_reg(0x020)  // 2 Cache Control
#define V3D_SLCACTL v3d_reg(0x024)  // Slices Cache Control

#define V3D_INTCTL  v3d_reg(0x030)  // Interrupt Control
#define V3D_INTENA  v3d_reg(0x034)  // Interrupt Enables
#define V3D_INTDIS  v3d_reg(0x038)  // Interrupt Disables

#define V3D_CT0CS   v3d_reg(0x100)  // Control List Executor Thread 0 Control and Status.
#define V3D_CT1CS   v3d_reg(0x104)  // Control List Executor Thread 1 Control and Status.
#define V3D_CT0EA   v3d_reg(0x108)  // Control List Executor Thread 0 End Address.
#define V3D_CT1EA   v3d_reg(0x10c)  // Control List Executor Thread 1 End Address.
#define V3D_CT0CA   v3d_reg(0x110)  // Control List Executor Thread 0 Current Address.
#define V3D_CT1CA   v3d_reg(0x114)  // Control List Executor Thread 1 Current Address.
#define V3D_CT00RA0 v3d_reg(0x118)  // Control List Executor Thread 0 Return Address.
#define V3D_CT01RA0 v3d_reg(0x11c)  // Control List Executor Thread 1 Return Address.
#define V3D_CT0LC   v3d_reg(0x120)  // Control List Executor Thread 0 List Counter
#define V3D_CT1LC   v3d_reg(0x124)  // Control List Executor Thread 1 List Counter
#define V3D_CT0PC   v3d_reg(0x128)  // Control List Executor Thread 0 Primitive List Counter
#define V3D_CT1PC   v3d_reg(0x12c)  // Control List Executor Thread 1 Primitive List Counter

#define V3D_PCS     v3d_reg(0x130)  // V3D Pipeline Control and Status
#define V3D_BFC     v3d_reg(0x134)  // Binning Mode Flush Count
#define V3D_RFC     v3d_reg(0x138)  // Rendering Mode Frame Count

#define V3D_BPCA    v3d_reg(0x300)  // Current Address of Binning Memory Pool
#define V3D_BPCS    v3d_reg(0x304)  // Remaining Size of Binning Memory Pool
#define V3D_BPOA    v3d_reg(0x308)  // Address of Overspill Binning Memory Block
#define V3D_BPOS    v3d_reg(0x30c)  // Size of Overspill Binning Memory Block
#define V3D_BXCF    v3d_reg(0x310)  // Binner Debug

#define V3D_SQRSV0  v3d_reg(0x410)  // Reserve QPUs 0-7
#define V3D_SQRSV1  v3d_reg(0x414)  // Reserve QPUs 8-15
#define V3D_SQCNTL  v3d_reg(0x418)  // QPU Scheduler Control

#define V3D_SRQPC   v3d_reg(0x430)  // QPU User Program Request Program Address
#define V3D_SRQUA   v3d_reg(0x434)  // QPU User Program Request Uniforms Address
#define V3D_SRQUL   v3d_reg(0x438)  // QPU User Program Request Uniforms Length
#define V3D_SRQCS   v3d_reg(0x43c)  // QPU User Program Request Control and Status

#define V3D_VPACNTL v3d_reg(0x500)  // VPM Allocator Control
#define V3D_VPMBASE v3d_reg(0x504)  // VPM base (user  memory reservation

#define V3D_PCTRC   v3d_reg(0x670)  // Performance Counter Clear
#define V3D_PCTRE   v3d_reg(0x674)  // Performance Counter Enables

#define V3D_PCTR0   v3d_reg(0x680)  // Performance Counter Count 0
#define V3D_PCTRS0  v3d_reg(0x684)  // Performance Counter Mapping 0
#define V3D_PCTR1   v3d_reg(0x688)  // Performance Counter Count 1
#define V3D_PCTRS1  v3d_reg(0x68c)  // Performance Counter Mapping 1
#define V3D_PCTR2   v3d_reg(0x690)  // Performance Counter Count 2
#define V3D_PCTRS2  v3d_reg(0x694)  // Performance Counter Mapping 2
#define V3D_PCTR3   v3d_reg(0x698)  // Performance Counter Count 3
#define V3D_PCTRS3  v3d_reg(0x69c)  // Performance Counter Mapping 3
#define V3D_PCTR4   v3d_reg(0x6a0)  // Performance Counter Count 4
#define V3D_PCTRS4  v3d_reg(0x6a4)  // Performance Counter Mapping 4
#define V3D_PCTR5   v3d_reg(0x6a8)  // Performance Counter Count 5
#define V3D_PCTRS5  v3d_reg(0x6ac)  // Performance Counter Mapping 5
#define V3D_PCTR6   v3d_reg(0x6b0)  // Performance Counter Count 6
#define V3D_PCTRS6  v3d_reg(0x6b4)  // Performance Counter Mapping 6
#define V3D_PCTR7   v3d_reg(0x6b8)  // Performance Counter Count 7
#define V3D_PCTRS7  v3d_reg(0x6bc)  // Performance Counter Mapping 7 
#define V3D_PCTR8   v3d_reg(0x6c0)  // Performance Counter Count 8
#define V3D_PCTRS8  v3d_reg(0x6c4)  // Performance Counter Mapping 8
#define V3D_PCTR9   v3d_reg(0x6c8)  // Performance Counter Count 9
#define V3D_PCTRS9  v3d_reg(0x6cc)  // Performance Counter Mapping 9
#define V3D_PCTR10  v3d_reg(0x6d0)  // Performance Counter Count 10
#define V3D_PCTRS10 v3d_reg(0x6d4)  // Performance Counter Mapping 10
#define V3D_PCTR11  v3d_reg(0x6d8)  // Performance Counter Count 11
#define V3D_PCTRS11 v3d_reg(0x6dc)  // Performance Counter Mapping 11
#define V3D_PCTR12  v3d_reg(0x6e0)  // Performance Counter Count 12
#define V3D_PCTRS12 v3d_reg(0x6e4)  // Performance Counter Mapping 12
#define V3D_PCTR13  v3d_reg(0x6e8)  // Performance Counter Count 13
#define V3D_PCTRS13 v3d_reg(0x6ec)  // Performance Counter Mapping 13
#define V3D_PCTR14  v3d_reg(0x6f0)  // Performance Counter Count 14
#define V3D_PCTRS14 v3d_reg(0x6f4)  // Performance Counter Mapping 14
#define V3D_PCTR15  v3d_reg(0x6f8)  // Performance Counter Count 15
#define V3D_PCTRS15 v3d_reg(0x6fc)  // Performance Counter Mapping 15

#define V3D_DBGE    v3d_reg(0xf00)  // PSE Error Signals
#define V3D_FDBGO   v3d_reg(0xf04)  // FEP Overrun Error Signals
#define V3D_FDBGB   v3d_reg(0xf08)  // FEP Interface Ready and Stall Signals, FEP Busy Signals
#define V3D_FDBGR   v3d_reg(0xf0c)  // FEP Internal Ready Signals
#define V3D_FDBGS   v3d_reg(0xf10)  // FEP Internal Stall Input Signals

#define V3D_ERRSTAT v3d_reg(0xf20)  // Miscellaneous Error Signals (VPM, VDW, VCD, VCM, L2C)

void v3d_init()
{
  set_clock_rate(5, 250 * 1000 * 1000);
  enable_qpu();
  if (*V3D_IDENT0 != 0x02443356) {
    printf("Cannot enable QPUs, what can go wrong?\n");
    return;
  }
  printf("QPUs enabled\n");
}

void v3d_ctx_init(v3d_ctx *ctx, uint32_t w, uint32_t h, void *bufaddr)
{
  ctx->w = w;
  ctx->h = h;
  ctx->bufaddr = (uint32_t)bufaddr | GPU_BUS_ADDR;

  uint32_t handle = gpumem_alloc(0x800000, 0x1000, MEM_FLAG_COHERENT | MEM_FLAG_ZERO);
  uint32_t p = gpumem_lock(handle);
  ctx->rhandle = handle;
  ctx->rbusaddr = p;
  ctx->rarmaddr = p & ~GPU_BUS_ADDR;
  printf("%08x %08x\n", ctx->rbusaddr, ctx->rarmaddr);
}
