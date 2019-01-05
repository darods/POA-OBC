#if !TARGET_IPHONE_SIMULATOR


/*----------------------------------------------------------------------------------------
 File Inclusions
 ----------------------------------------------------------------------------------------*/
#include "Kakao_ImgProc_Modules.h"
#include "Kakao_ImgProc_SIMD.h"


/*----------------------------------------------------------------------------------------
 Constant Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Macro Definitions
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Type Definitions
 ----------------------------------------------------------------------------------------*/
typedef void (*fpgetNxMMean)(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum);
typedef void (*fpgetNxMVar)(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
typedef void (*fpgetNxMDiffVar)(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum);


/*----------------------------------------------------------------------------------------
 Static Function
 ----------------------------------------------------------------------------------------*/
static void _GetMean4x4_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum);
static void _GetMean8x8_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum);
static void _GetMean16x16_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum);
static void _GetVar4x4_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
static void _GetVar8x8_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
static void _GetVar16x16_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum);
static void _GetDiffVar4x4_NEON(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                             IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum);
static void _GetDiffVar8x8_NEON(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                             IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum);
static void _GetDiffVar16x16_NEON(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                             IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum);
static void _GetMean_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nWidth, IN const INT32 nHeight,
                             OUT UINT32 *pSum, fpgetNxMMean fpMean, IN const INT32 nBlockSize);
static void _GetVariance_NEON(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const INT32 nWidth, IN const INT32 nHeight,
                             OUT UINT32 *pSqSum, OUT UINT32 *pSum, fpgetNxMVar fpVar, IN const INT32 nBlockSize);
static void _GetDiffVariance_NEON(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1,
                             IN const INT32 nWidth, IN const INT32 nHeight, OUT UINT32 *pSqSum, OUT INT32 *pSum,
                             fpgetNxMDiffVar fpVar, IN const INT32 nBlockSize);


/*----------------------------------------------------------------------------------------
 Static Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Global Variable
 ----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
 Function Implementation
 ----------------------------------------------------------------------------------------*/
#if ENABLE_ISET(CPU_ARMX | ISET_NEON)
void OneRow_Copy_NEON(OUT UINT8 *pDst, IN const UINT8 *pSrc, IN INT32 nWidth)
{
    #ifndef __arm64__
    asm volatile (
                  ".p2align  2                               \n"
                  "1:                                        \n"
                  "vld4.u8    {d0, d1, d2, d3}, [%1]!        \n"  // load 32
                  "subs       %2, %2, #32                    \n"  // 32 processed per loop
                  "vst4.u8    {d0, d1, d2, d3}, [%0]!        \n"  // store 32
                  "bgt        1b                             \n"
                  : "+r"(pDst),                 // %0
                  "+r"(pSrc),                   // %1
                  "+r"(nWidth)                  // %2  // Output registers
                  :                             // Input registers
                  :                             // Clobber List
                  );
    #else
    asm volatile (
                  ".p2align  2                                          \n"
                  "1:                                                   \n"
                  "ld4          {v0.8b, v1.8b, v2.8b, v3.8b}, [%1], #32 \n"  // load 32
                  "subs         %2, %2, #32                             \n"  // 32 processed per loop
                  "st4          {v0.8b, v1.8b, v2.8b, v3.8b}, [%0], #32 \n"  // store 32
                  "b.gt         1b                                      \n"
                  : "+r"(pDst),                 // %0
                  "+r"(pSrc),                   // %1
                  "+r"(nWidth)                  // %2  // Output registers
                  :                             // Input registers
                  :                             // Clobber List
                  );
    #endif
}


void OneRow_InterleaveUV_NEON(IN const UINT8 *pSrcUV, OUT UINT8 *pDstU, OUT UINT8 *pDstV, IN INT32 nWidth)
{
    #ifndef __arm64__
    asm volatile (
                  ".p2align  2                     \n"
                  "1:                              \n"
                  "vld2.u8    {q0, q1}, [%0]!      \n"    // load 16 pairs of UV
                  "subs       %3, %3, #16          \n"    // 16 processed per loop
                  "vst1.u8    {q0}, [%1]!          \n"    // store U
                  "vst1.u8    {q1}, [%2]!          \n"    // store V
                  "bgt        1b                   \n"
                  : "+r"(pSrcUV),                         // %0
                  "+r"(pDstU),                            // %1
                  "+r"(pDstV),                            // %2
                  "+r"(nWidth)                             // %3   Output registers
                  :                                       // Input registers
                  : "memory", "cc", "q0", "q1"            // Clobber List
                  );
    #else
    asm volatile (
                  ".p2align  2                                          \n"
                  "1:                                                   \n"
                  "ld2          {v0.16b, v1.16b}, [%0], #32             \n"  // load 32
                  "subs         %3, %3, #16                             \n"  // 16 processed per loop
                  "st1          {v0.16b}, [%1], #16                     \n"  // store U
                  "st1          {v1.16b}, [%2], #16                     \n"  // store V
                  "b.gt         1b                                      \n"
                  : "+r"(pSrcUV),                   // %0
                  "+r"(pDstU),                      // %1
                  "+r"(pDstV),                      // %2
                  "+r"(nWidth)                      // %3  // Output registers
                  :                                 // Input registers
                  :                                 // Clobber List
                  );
    #endif
}


void OneRow_Subsample_Even_NEON(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN INT32 nWidth)
{
    #ifndef __arm64__
    asm volatile (
                  ".p2align  2                     \n"
                  "1:                              \n"
                  "vld2.u8    {q0, q1}, [%0]!      \n"    // load 16 pairs of Pixel
                  "subs       %2, %2, #16          \n"    // 16 processed per loop
                  "vst1.u8    {q0}, [%1]!          \n"    // store Even Pixel
                  "bgt        1b                   \n"
                  : "+r"(pSrc),                           // %0
                  "+r"(pDst),                             // %1
                  "+r"(nWidth)                            // %2   Output registers
                  :                                       // Input registers
                  : "memory", "cc", "q0", "q1"            // Clobber List
                  );
    #else
    asm volatile (
                  ".p2align  2                                          \n"
                  "1:                                                   \n"
                  "ld2          {v0.16b, v1.16b}, [%0], #32             \n"  // load 32
                  "subs         %2, %2, #16                             \n"  // 16 processed per loop
                  "st1          {v0.16b}, [%1], #16                     \n"  // store Even Pixel
                  "b.gt         1b                                      \n"
                  : "+r"(pSrc),                     // %0
                  "+r"(pDst),                       // %1
                  "+r"(nWidth)                      // %2  // Output registers
                  :                                 // Input registers
                  :                                 // Clobber List
                  );
    #endif
}


void OneRow_Subsample_Odd_NEON(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN INT32 nWidth)
{
    #ifndef __arm64__
    asm volatile (
                  ".p2align  2                     \n"
                  "1:                              \n"
                  "vld2.u8    {q0, q1}, [%0]!      \n"    // load 16 pairs of Pixel
                  "subs       %2, %2, #16          \n"    // 16 processed per loop
                  "vst1.u8    {q1}, [%1]!          \n"    // store Odd Pixel
                  "bgt        1b                   \n"
                  : "+r"(pSrc),                           // %0
                  "+r"(pDst),                             // %1
                  "+r"(nWidth)                            // %2   Output registers
                  :                                       // Input registers
                  : "memory", "cc", "q0", "q1"            // Clobber List
                  );
    #else
    asm volatile (
                  ".p2align  2                                          \n"
                  "1:                                                   \n"
                  "ld2          {v0.16b, v1.16b}, [%0], #32             \n"  // load 32
                  "subs         %2, %2, #16                             \n"  // 16 processed per loop
                  "st1          {v1.16b}, [%1], #16                     \n"  // store Odd Pixel
                  "b.gt         1b                                      \n"
                  : "+r"(pSrc),                     // %0
                  "+r"(pDst),                       // %1
                  "+r"(nWidth)                      // %2  // Output registers
                  :                                 // Input registers
                  :                                 // Clobber List
                  );
    #endif
}


void OneRow_Subsample_1_4_NEON(const UINT8 *pSrc, UINT8 *pDst, INT32 nWidth)
{
    #ifndef __arm64__
    asm volatile (
                  "1:                              \n"
                  "vld2.u8    {d0, d1}, [%0]!      \n"
                  "vtrn.u8    d1, d0               \n"
                  "vshrn.u16  d0, q0, #8           \n"
                  "vst1.u32   {d0[1]}, [%1]!       \n"
                  "subs       %2, #4               \n"
                  "bgt        1b                   \n"
                  : "+r"(pSrc),        // %0
                  "+r"(pDst),          // %1
                  "+r"(nWidth)         // %2
                  :
                  : "q0", "q1", "memory", "cc"
                  );
    #else
    asm volatile (
                  "1:                                          \n"
                  "ld4     {v0.8b,v1.8b,v2.8b,v3.8b}, [%0], #32          \n"  // src line 0
                  "subs       %2, %2, #8                     \n"  // 8 processed per loop
                  "st1     {v2.8b}, [%1], #8                 \n"
                  "b.gt       1b                             \n"
                  : "+r"(pSrc),          // %0
                  "+r"(pDst),          // %1
                  "+r"(nWidth)         // %2
                  :
                  : "v0", "v1", "v2", "v3", "memory", "cc"
                  );
    #endif
}


void OneRow_ScaleBilinear_NEON(UINT8* pDst,
                          const UINT8* pSrc, ptrdiff_t nSrcStride,
                          INT32 nDstWidth, INT32 nSrcYFraction)
{
    #ifndef __arm64__
    asm volatile (
                  "cmp          %4, #0                       \n"
                  "beq          2f                           \n"
                  "add          %2, %1                       \n"
                  "cmp          %4, #128                     \n"
                  "beq          3f                           \n"
                  
                  "vdup.8       d5, %4                       \n"
                  "rsb          %4, #256                     \n"
                  "vdup.8       d4, %4                       \n"
                  "1:                                        \n"
                  "vld1.u8      {q0}, [%1]!                  \n"
                  "vld1.u8      {q1}, [%2]!                  \n"
                  "subs         %3, #16                      \n"
                  "vmull.u8     q13, d0, d4                  \n"
                  "vmull.u8     q14, d1, d4                  \n"
                  "vmlal.u8     q13, d2, d5                  \n"
                  "vmlal.u8     q14, d3, d5                  \n"
                  "vrshrn.u16   d0, q13, #8                  \n"
                  "vrshrn.u16   d1, q14, #8                  \n"
                  "vst1.u8      {q0}, [%0]!                  \n"
                  "bgt          1b                           \n"
                  "b            4f                           \n"
                  
                  "2:                                        \n"
                  "vld1.u8      {q0}, [%1]!                  \n"
                  "subs         %3, #16                      \n"
                  "vst1.u8      {q0}, [%0]!                  \n"
                  "bgt          2b                           \n"
                  "b            4f                           \n"
                  
                  "3:                                        \n"
                  "vld1.u8      {q0}, [%1]!                  \n"
                  "vld1.u8      {q1}, [%2]!                  \n"
                  "subs         %3, #16                      \n"
                  "vrhadd.u8    q0, q1                       \n"
                  "vst1.u8      {q0}, [%0]!                  \n"
                  "bgt          3b                           \n"
                  "4:                                        \n"
                  "vst1.u8      {d1[7]}, [%0]                \n"
                  : "+r"(pDst),                         // %0
                  "+r"(pSrc),                           // %1
                  "+r"(nSrcStride),                     // %2
                  "+r"(nDstWidth),                      // %3
                  "+r"(nSrcYFraction)                   // %4
                  :
                  : "q0", "q1", "d4", "d5", "q13", "q14", "memory", "cc"
                  );
    #else
    int nYfraction = 256 - nSrcYFraction;
    asm volatile (
                  "cmp          %4, #0                       \n"
                  "b.eq         100f                         \n"
                  "add          %2, %2, %1                   \n"
                  "cmp          %4, #64                      \n"
                  "b.eq         75f                          \n"
                  "cmp          %4, #128                     \n"
                  "b.eq         50f                          \n"
                  "cmp          %4, #192                     \n"
                  "b.eq         25f                          \n"
                  
                  "dup          v5.8b, %w4                   \n"
                  "dup          v4.8b, %w5                   \n"
                  // General purpose row blend.
                  "1:                                          \n"
                  //MEMACCESS(1)
                  "ld1          {v0.16b}, [%1], #16          \n"
                  //MEMACCESS(2)
                  "ld1          {v1.16b}, [%2], #16          \n"
                  "subs         %3, %3, #16                  \n"
                  "umull        v6.8h, v0.8b, v4.8b          \n"
                  "umull2       v7.8h, v0.16b, v4.16b        \n"
                  "umlal        v6.8h, v1.8b, v5.8b          \n"
                  "umlal2       v7.8h, v1.16b, v5.16b        \n"
                  "rshrn        v0.8b, v6.8h, #8             \n"
                  "rshrn2       v0.16b, v7.8h, #8            \n"
                  //MEMACCESS(0)
                  "st1          {v0.16b}, [%0], #16          \n"
                  "b.gt         1b                           \n"
                  "b            99f                          \n"
                  
                  // Blend 25 / 75.
                  "25:                                         \n"
                  //MEMACCESS(1)
                  "ld1          {v0.16b}, [%1], #16          \n"
                  //MEMACCESS(2)
                  "ld1          {v1.16b}, [%2], #16          \n"
                  "subs         %3, %3, #16                  \n"
                  "urhadd       v0.16b, v0.16b, v1.16b       \n"
                  "urhadd       v0.16b, v0.16b, v1.16b       \n"
                  //MEMACCESS(0)
                  "st1          {v0.16b}, [%0], #16          \n"
                  "b.gt         25b                          \n"
                  "b            99f                          \n"
                  
                  // Blend 50 / 50.
                  "50:                                         \n"
                  //MEMACCESS(1)
                  "ld1          {v0.16b}, [%1], #16          \n"
                  //MEMACCESS(2)
                  "ld1          {v1.16b}, [%2], #16          \n"
                  "subs         %3, %3, #16                  \n"
                  "urhadd       v0.16b, v0.16b, v1.16b       \n"
                  //MEMACCESS(0)
                  "st1          {v0.16b}, [%0], #16          \n"
                  "b.gt         50b                          \n"
                  "b            99f                          \n"
                  
                  // Blend 75 / 25.
                  "75:                                         \n"
                  //MEMACCESS(1)
                  "ld1          {v1.16b}, [%1], #16          \n"
                  //MEMACCESS(2)
                  "ld1          {v0.16b}, [%2], #16          \n"
                  "subs         %3, %3, #16                  \n"
                  "urhadd       v0.16b, v0.16b, v1.16b       \n"
                  "urhadd       v0.16b, v0.16b, v1.16b       \n"
                  //MEMACCESS(0)
                  "st1          {v0.16b}, [%0], #16          \n"
                  "b.gt         75b                          \n"
                  "b            99f                          \n"
                  
                  // Blend 100 / 0 - Copy row unchanged.
                  "100:                                        \n"
                  //MEMACCESS(1)
                  "ld1          {v0.16b}, [%1], #16          \n"
                  "subs         %3, %3, #16                  \n"
                  //MEMACCESS(0)
                  "st1          {v0.16b}, [%0], #16          \n"
                  "b.gt         100b                         \n"
                  
                  "99:                                         \n"
                  //MEMACCESS(0)
                  "st1          {v0.b}[15], [%0]             \n"
                  : "+r"(pDst),                         // %0
                  "+r"(pSrc),                           // %1
                  "+r"(nSrcStride),                     // %2
                  "+r"(nDstWidth),                      // %3
                  "+r"(nSrcYFraction),                  // %4
                  "+r"(nYfraction)                      // %5
                  :
                  : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "memory", "cc"
                  );
    #endif
}


void OneCol_ScaleBilinear_NEON(UINT8* pDst, const UINT8* pSrc,
                          INT32 nDstWidth, INT32 x, INT32 dx)
{
    INT32           j = 0;
    
    for(j=0 ; j<nDstWidth-1 ; j+= 2)
    {
        INT32       xi = x >> 16;
        INT32       a = pSrc[xi];
        INT32       b = pSrc[xi + 1];
        
        pDst[0] = BLENDER(a, b, x & 0xffff);
        x += dx;
        xi = x >> 16;
        a = pSrc[xi];
        b = pSrc[xi + 1];
        pDst[1] = BLENDER(a, b, x & 0xffff);
        x += dx;
        pDst += 2;
    }
    
    if(nDstWidth & 1)
    {
        INT32       xi = x >> 16;
        INT32       a = pSrc[xi];
        INT32       b = pSrc[xi + 1];
        
        pDst[0] = BLENDER(a, b, x & 0xffff);
    }
}


void OneRow_MeanBlur_NEON(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride)
{
    INT32                   i = 0;
    const INT32             nLoopCnt = nWidth / 24;
    const UINT32            nDoubleSrcStride = (nSrcStride << 1);
    const UINT32            nDoubleDstStride = (nDstStride << 1);
    UINT32                  nOffset = 0;
    const UINT8             *pSrcHorPrev = (UINT8 *)(pSrc - 1);
    const UINT8             *pSrcHorNext = (UINT8 *)(pSrc + 1);
    UINT8                   *pSrcVerPrev = (UINT8 *)(pSrc - nSrcStride);
    UINT8                   *pSrcVerNext = (UINT8 *)(pSrc + nSrcStride);
    
    for(i=nLoopCnt ; i>=0 ; i--)
    {
        {
            uint8x8x3_t             nSrcHorPrev0, nSrcHorCurr0, nSrcHorNext0;
            uint8x8x3_t             nSrcHorPrev1, nSrcHorCurr1, nSrcHorNext1;
            uint8x8x3_t             nSrcHorPrev2, nSrcHorCurr2, nSrcHorNext2;
            uint8x8x3_t             nTmpDst0, nTmpDst1, nTmpDst2;
            uint8x8_t               nTmpSum0, nTmpSum1, nTmpSum2;
            
            __builtin_prefetch(pSrcHorPrev + (nWidth << 4));
            __builtin_prefetch(pSrc + (nWidth << 4));
            __builtin_prefetch(pSrcHorNext + (nWidth << 4));
            
            nSrcHorPrev0 = vld3_u8(pSrcHorPrev + nOffset);
            nSrcHorCurr0 = vld3_u8(pSrc + nOffset);
            nSrcHorNext0 = vld3_u8(pSrcHorNext + nOffset);
            nSrcHorPrev1 = vld3_u8(pSrcHorPrev + nSrcStride + nOffset);
            nSrcHorCurr1 = vld3_u8(pSrc + nSrcStride + nOffset);
            nSrcHorNext1 = vld3_u8(pSrcHorNext + nSrcStride + nOffset);
            nSrcHorPrev2 = vld3_u8(pSrcHorPrev + nDoubleSrcStride + nOffset);
            nSrcHorCurr2 = vld3_u8(pSrc + nDoubleSrcStride + nOffset);
            nSrcHorNext2 = vld3_u8(pSrcHorNext + nDoubleSrcStride + nOffset);
            
            nTmpSum0 = vhadd_u8(nSrcHorPrev0.val[0], nSrcHorCurr0.val[0]);
            nTmpDst0.val[0] = vhadd_u8(nTmpSum0, nSrcHorNext0.val[0]);
            nTmpSum1 = vhadd_u8(nSrcHorPrev0.val[1], nSrcHorCurr0.val[1]);
            nTmpDst0.val[1] = vhadd_u8(nTmpSum1, nSrcHorNext0.val[1]);
            nTmpSum2 = vhadd_u8(nSrcHorPrev0.val[2], nSrcHorCurr0.val[2]);
            nTmpDst0.val[2] = vhadd_u8(nTmpSum2, nSrcHorNext0.val[2]);
            vst3_u8(pDst + nOffset, nTmpDst0);
            
            nTmpSum0 = vhadd_u8(nSrcHorPrev1.val[0], nSrcHorCurr1.val[0]);
            nTmpDst1.val[0] = vhadd_u8(nTmpSum0, nSrcHorNext1.val[0]);
            nTmpSum1 = vhadd_u8(nSrcHorPrev1.val[1], nSrcHorCurr1.val[1]);
            nTmpDst1.val[1] = vhadd_u8(nTmpSum1, nSrcHorNext1.val[1]);
            nTmpSum2 = vhadd_u8(nSrcHorPrev1.val[2], nSrcHorCurr1.val[2]);
            nTmpDst1.val[2] = vhadd_u8(nTmpSum2, nSrcHorNext1.val[2]);
            vst3_u8(pDst + nDstStride + nOffset, nTmpDst1);
            
            nTmpSum0 = vhadd_u8(nSrcHorPrev2.val[0], nSrcHorCurr2.val[0]);
            nTmpDst2.val[0] = vhadd_u8(nTmpSum0, nSrcHorNext2.val[0]);
            nTmpSum1 = vhadd_u8(nSrcHorPrev2.val[1], nSrcHorCurr2.val[1]);
            nTmpDst2.val[1] = vhadd_u8(nTmpSum1, nSrcHorNext2.val[1]);
            nTmpSum2 = vhadd_u8(nSrcHorPrev2.val[2], nSrcHorCurr2.val[2]);
            nTmpDst2.val[2] = vhadd_u8(nTmpSum2, nSrcHorNext2.val[2]);
            vst3_u8(pDst + nDoubleDstStride + nOffset, nTmpDst2);
        }
        
        {
            uint8x8_t               nSrcVerPrev0, nSrcVerCurr0, nSrcVerNext0;
            uint8x8_t               nSrcVerPrev1, nSrcVerCurr1, nSrcVerNext1;
            uint8x8_t               nSrcVerPrev2, nSrcVerCurr2, nSrcVerNext2;
            uint8x8_t               nTmpSum0, nTmpSum1, nTmpSum2;
            
            __builtin_prefetch(pSrcVerPrev + (nWidth << 4));
            __builtin_prefetch(pSrc + (nWidth << 4));
            __builtin_prefetch(pSrcVerNext + (nWidth << 4));
            
            nSrcVerPrev0 = vld1_u8(pSrcVerPrev + nOffset);
            nSrcVerCurr0 = vld1_u8(pSrc + nOffset);
            nSrcVerNext0 = vld1_u8(pSrcVerNext + nOffset);
            nSrcVerPrev1 = vld1_u8(pSrcVerPrev + nOffset + 8);
            nSrcVerCurr1 = vld1_u8(pSrc + nOffset + 8);
            nSrcVerNext1 = vld1_u8(pSrcVerNext + nOffset + 8);
            nSrcVerPrev2 = vld1_u8(pSrcVerPrev + nOffset + 16);
            nSrcVerCurr2 = vld1_u8(pSrc + nOffset + 16);
            nSrcVerNext2 = vld1_u8(pSrcVerNext + nOffset + 16);
            
            nTmpSum0 = vhadd_u8(nSrcVerPrev0, nSrcVerCurr0);
            vst1_u8(pDst + nOffset, vhadd_u8(nTmpSum0, nSrcVerNext0));
            
            nTmpSum1 = vhadd_u8(nSrcVerPrev1, nSrcVerCurr1);
            vst1_u8(pDst + nOffset + 8, vhadd_u8(nTmpSum1, nSrcVerNext1));
            
            nTmpSum2 = vhadd_u8(nSrcVerPrev2, nSrcVerCurr2);
            vst1_u8(pDst + nOffset + 16, vhadd_u8(nTmpSum2, nSrcVerNext2));
        }
        
        nOffset += 24;
    }
    
    return;
}


void OneRow_MeanBlur_Hor_NEON(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    INT32                   nLoopCnt = 0;
    UINT8                   *pSrcPrev = NULL, *pSrcNext = NULL;
    UINT32                  nOffset = 0;
    
    pSrcPrev = (UINT8 *)(pSrc - 1);
    pSrcNext = (UINT8 *)(pSrc + 1);
    
    nLoopCnt = nWidth / 24;
    
    for(i=nLoopCnt ; i>=0 ; i--)
    {
        uint8x8x3_t             nSrcPrev, nSrcCurr, nSrcNext;
        uint8x8x3_t             nTmpDst;
        
        nSrcPrev = vld3_u8(pSrcPrev + nOffset);
        nSrcCurr = vld3_u8(pSrc + nOffset);
        nSrcNext = vld3_u8(pSrcNext + nOffset);
        
        nTmpDst.val[0] = vhadd_u8(vhadd_u8(nSrcPrev.val[0], nSrcCurr.val[0]), nSrcNext.val[0]);
        nTmpDst.val[1] = vhadd_u8(vhadd_u8(nSrcPrev.val[1], nSrcCurr.val[1]), nSrcNext.val[1]);
        nTmpDst.val[2] = vhadd_u8(vhadd_u8(nSrcPrev.val[2], nSrcCurr.val[2]), nSrcNext.val[2]);
        
        vst3_u8(pDst + nOffset, nTmpDst);
        nOffset += 24;
    }
    
    return;
}


void OneRow_MeanBlur_Ver_NEON(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride)
{
    INT32                   i = 0;
    INT32                   nLoopCnt = 0;
    UINT8                   *pSrcPrev = NULL, *pSrcNext = NULL;
    UINT32                  nOffset = 0;
    
    pSrcPrev = (UINT8 *)(pSrc - nSrcStride);
    pSrcNext = (UINT8 *)(pSrc + nSrcStride);
    
    if(!(nWidth & 0x8))
    {
        // When Width is 16 byte Aligned
        nLoopCnt = (nWidth >> 4);
        
        for(i=nLoopCnt-1 ; i>=0 ; i-=2)
        {
            uint8x16_t               nSrcPrev0, nSrcCurr0, nSrcNext0;
            uint8x16_t               nSrcPrev1, nSrcCurr1, nSrcNext1;
            
            nSrcPrev0 = vld1q_u8(pSrcPrev + nOffset);
            nSrcCurr0 = vld1q_u8(pSrc + nOffset);
            nSrcNext0 = vld1q_u8(pSrcNext + nOffset);
            vst1q_u8(pDst + nOffset, vhaddq_u8(vhaddq_u8(nSrcPrev0, nSrcCurr0), nSrcNext0));
            nOffset += 16;
            
            nSrcPrev1 = vld1q_u8(pSrcPrev + nOffset);
            nSrcCurr1 = vld1q_u8(pSrc + nOffset);
            nSrcNext1 = vld1q_u8(pSrcNext + nOffset);
            vst1q_u8(pDst + nOffset, vhaddq_u8(vhaddq_u8(nSrcPrev1, nSrcCurr1), nSrcNext1));
            nOffset += 16;
        }
    }
    else
    {
        // When Width is 8 byte Aligned
        nLoopCnt = (nWidth >> 3);
        
        for(i=nLoopCnt-1 ; i>=0 ; i-=2)
        {
            uint8x8_t               nSrcVerPrev0, nSrcVerCurr0, nSrcVerNext0;
            uint8x8_t               nSrcVerPrev1, nSrcVerCurr1, nSrcVerNext1;
            uint8x8_t               nTmpSum0, nTmpSum1;
            
            nSrcVerPrev0 = vld1_u8(pSrcPrev + nOffset);
            nSrcVerCurr0 = vld1_u8(pSrc + nOffset);
            nSrcVerNext0 = vld1_u8(pSrcNext + nOffset);
            nSrcVerPrev1 = vld1_u8(pSrcPrev + nOffset + 8);
            nSrcVerCurr1 = vld1_u8(pSrc + nOffset + 8);
            nSrcVerNext1 = vld1_u8(pSrcNext + nOffset + 8);
            
            nTmpSum0 = vhadd_u8(nSrcVerPrev0, nSrcVerCurr0);
            vst1_u8(pDst + nOffset, vhadd_u8(nTmpSum0, nSrcVerNext0));
            
            nTmpSum1 = vhadd_u8(nSrcVerPrev1, nSrcVerCurr1);
            vst1_u8(pDst + nOffset + 8, vhadd_u8(nTmpSum1, nSrcVerNext1));
        }
    }
    
    return;
}


void OneRow_Mosaic4_NEON(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr)
{
    INT32                   i = 0;
    UINT32                  nLooCnt = nWidth >> 4;
    INT32                   nLocalOffset = 0;
    UINT8                   *pSrcLine1 = NULL, *pSrcLine2 = NULL, *pSrcLine3 = NULL;
    UINT8                   *pDstLine1 = NULL, *pDstLine2 = NULL, *pDstLine3 = NULL;
    
    pSrcLine1 = (UINT8 *)pSrc + nSrcStride;
    pSrcLine2 = (UINT8 *)pSrc + (2 * nSrcStride);
    pSrcLine3 = (UINT8 *)pSrc + (3 * nSrcStride);
    
    pDstLine1 = (UINT8 *)pDst + nDstStride;
    pDstLine2 = (UINT8 *)pDst + (2 * nDstStride);
    pDstLine3 = (UINT8 *)pDst + (3 * nDstStride);
    
    for(i=nLooCnt ; i>0 ; i--)
    {
        uint8x16_t          nLine8x16_0, nLine8x16_1, nLine8x16_2, nLine8x16_3, nDst;
        uint16x8_t          nLine16x8_0, nLine16x8_1, nLine16x8_2, nLine16x8_3;
        uint32x4_t          nLine32x4_0, nLine32x4_1;
        uint16x4_t          nLine16x4_0;
        uint16x4x2_t        nLine16x4x2_0;
        uint8x8_t           nLine8x8_0, nLine8x8_1;
        
        nLine8x16_0 = vld1q_u8(pSrc + nLocalOffset);
        nLine8x16_1 = vld1q_u8(pSrcLine1 + nLocalOffset);
        nLine8x16_2 = vld1q_u8(pSrcLine2 + nLocalOffset);
        nLine8x16_3 = vld1q_u8(pSrcLine3 + nLocalOffset);
        
        nLine16x8_0 = vpaddlq_u8(nLine8x16_0);
        nLine16x8_1 = vpaddlq_u8(nLine8x16_1);
        nLine16x8_2 = vpadalq_u8(nLine16x8_0, nLine8x16_2);
        nLine16x8_3 = vpadalq_u8(nLine16x8_1, nLine8x16_3);
        
        nLine32x4_0 = vpaddlq_u16(nLine16x8_2);
        nLine32x4_1 = vpadalq_u16(nLine32x4_0, nLine16x8_3);
        
        nLine16x4_0 = vshrn_n_u32(nLine32x4_1, 4);
        
        nLine16x4x2_0 = vzip_u16(nLine16x4_0, nLine16x4_0);
        
        nLine8x8_0 = vreinterpret_u8_u16(vorr_u16(nLine16x4x2_0.val[0], vshl_n_u16(nLine16x4x2_0.val[0], 8)));
        nLine8x8_1 = vreinterpret_u8_u16(vorr_u16(nLine16x4x2_0.val[1], vshl_n_u16(nLine16x4x2_0.val[1], 8)));
        nDst = vcombine_u8(nLine8x8_0, nLine8x8_1);
        
        vst1q_u8(pDst + nLocalOffset, nDst);
        vst1q_u8(pDstLine1 + nLocalOffset, nDst);
        vst1q_u8(pDstLine2 + nLocalOffset, nDst);
        vst1q_u8(pDstLine3 + nLocalOffset, nDst);
        
        nLocalOffset += 16;
    }
    
    return;
}


void OneRow_Mosaic8_NEON(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr)
{
#if 1
    UINT32                  i = 0, j = 0;
    UINT8                   nAvgPix0 = 0, nAvgPix1 = 0;
    UINT8                   *pLocalDst = NULL;
    UINT32                  nLineTable[8] = {0, };
    const UINT32            nMosaicSize = 8;
    uint8x8_t               nVecAvgPix0, nVecAvgPix1;
    uint8x16_t              nVecMergedPix;
    
    SET_UNUSED_VARIABLE(pSrc);
    
    for(i=0 ; i<nMosaicSize ; i++)
        nLineTable[i] = i * nDstStride;
    
    for(i=0 ; i<nWidth ; i+=(nMosaicSize << 1))
    {
        nAvgPix0 = (CALCULATE_INTEGRAL_SUM((*pBaseRectAddr), (i))) >> 6;
        nVecAvgPix0 = vdup_n_u8(nAvgPix0);
        
        nAvgPix1 = (CALCULATE_INTEGRAL_SUM((*pBaseRectAddr), (i + nMosaicSize))) >> 6;
        nVecAvgPix1 = vdup_n_u8(nAvgPix1);
        
        nVecMergedPix = vcombine_u8(nVecAvgPix0, nVecAvgPix1);
        
        pLocalDst = pDst + i;
        for(j=0 ; j<nMosaicSize ; j++)
            vst1q_u8(pLocalDst + nLineTable[j], nVecMergedPix);
    }
    
    return;
#else
    INT32                   i = 0;
    UINT32                  nLooCnt = nWidth >> 4;
    INT32                   nLocalOffset = 0;
    UINT8                   *pSrcLine1 = NULL, *pSrcLine2 = NULL, *pSrcLine3 = NULL;
    UINT8                   *pSrcLine4 = NULL, *pSrcLine5 = NULL, *pSrcLine6 = NULL, *pSrcLine7 = NULL;
    UINT8                   *pDstLine1 = NULL, *pDstLine2 = NULL, *pDstLine3 = NULL;
    UINT8                   *pDstLine4 = NULL, *pDstLine5 = NULL, *pDstLine6 = NULL, *pDstLine7 = NULL;
    Kakao_Status            nRet = KAKAO_STAT_FAIL;
    
    pSrcLine1 = (UINT8 *)pSrc + nStride;
    pSrcLine2 = (UINT8 *)pSrc + (2 * nStride);
    pSrcLine3 = (UINT8 *)pSrc + (3 * nStride);
    pSrcLine4 = (UINT8 *)pSrc + (4 * nStride);
    pSrcLine5 = (UINT8 *)pSrc + (5 * nStride);
    pSrcLine6 = (UINT8 *)pSrc + (6 * nStride);
    pSrcLine7 = (UINT8 *)pSrc + (7 * nStride);
    
    pDstLine1 = (UINT8 *)pDst + nStride;
    pDstLine2 = (UINT8 *)pDst + (2 * nStride);
    pDstLine3 = (UINT8 *)pDst + (3 * nStride);
    pDstLine4 = (UINT8 *)pDst + (4 * nStride);
    pDstLine5 = (UINT8 *)pDst + (5 * nStride);
    pDstLine6 = (UINT8 *)pDst + (6 * nStride);
    pDstLine7 = (UINT8 *)pDst + (7 * nStride);
    
    for(i=nLooCnt ; i>0 ; i--)
    {
        uint8x16_t          nLine8x16_0, nLine8x16_1, nLine8x16_2, nLine8x16_3;
        uint8x16_t          nLine8x16_4, nLine8x16_5, nLine8x16_6, nLine8x16_7;
        uint8x16_t          nDst0, nDst1;
        uint16x8_t          nLine16x8_0, nLine16x8_1, nLine16x8_2, nLine16x8_3;
        uint16x8_t          nLine16x8_4, nLine16x8_5, nLine16x8_6, nLine16x8_7;
        uint32x4_t          nLine32x4_0, nLine32x4_1, nLine32x4_2, nLine32x4_3;
        uint64x2_t          nLine64x2_0, nLine64x2_1;
        uint8x8_t           nLine8x8_0, nLine8x8_1;
        
        nLine8x16_0 = vld1q_u8(pSrc + nLocalOffset);
        nLine8x16_1 = vld1q_u8(pSrcLine1 + nLocalOffset);
        nLine8x16_2 = vld1q_u8(pSrcLine2 + nLocalOffset);
        nLine8x16_3 = vld1q_u8(pSrcLine3 + nLocalOffset);
        nLine8x16_4 = vld1q_u8(pSrcLine4 + nLocalOffset);
        nLine8x16_5 = vld1q_u8(pSrcLine5 + nLocalOffset);
        nLine8x16_6 = vld1q_u8(pSrcLine6 + nLocalOffset);
        nLine8x16_7 = vld1q_u8(pSrcLine7 + nLocalOffset);
        
        nLine16x8_0 = vpaddlq_u8(nLine8x16_0);
        nLine16x8_1 = vpaddlq_u8(nLine8x16_1);
        nLine16x8_2 = vpadalq_u8(nLine16x8_0, nLine8x16_2);
        nLine16x8_3 = vpadalq_u8(nLine16x8_1, nLine8x16_3);
        
        nLine16x8_4 = vpaddlq_u8(nLine8x16_4);
        nLine16x8_5 = vpaddlq_u8(nLine8x16_5);
        nLine16x8_6 = vpadalq_u8(nLine16x8_4, nLine8x16_6);
        nLine16x8_7 = vpadalq_u8(nLine16x8_5, nLine8x16_7);
        
        nLine32x4_0 = vpaddlq_u16(nLine16x8_2);
        nLine32x4_1 = vpadalq_u16(nLine32x4_0, nLine16x8_3);
        
        nLine32x4_2 = vpaddlq_u16(nLine16x8_6);
        nLine32x4_3 = vpadalq_u16(nLine32x4_2, nLine16x8_7);
        
        nLine64x2_0 = vpaddlq_u32(nLine32x4_1);
        nLine64x2_1 = vpadalq_u32(nLine64x2_0, nLine32x4_3);
        
        nLine64x2_0 = vrshrq_n_u64(nLine64x2_1, 6);
        
        nDst0 = vreinterpretq_u8_u64(nLine64x2_0);
        
        nLine8x8_0 = vdup_lane_u8(vget_low_u8(nDst0), 0);
        nLine8x8_1 = vdup_lane_u8(vget_high_u8(nDst0), 0);
        nDst1 = vcombine_u8(nLine8x8_0, nLine8x8_1);
        
        vst1q_u8(pDst + nLocalOffset, nDst1);
        vst1q_u8(pDstLine1 + nLocalOffset, nDst1);
        vst1q_u8(pDstLine2 + nLocalOffset, nDst1);
        vst1q_u8(pDstLine3 + nLocalOffset, nDst1);
        vst1q_u8(pDstLine4 + nLocalOffset, nDst1);
        vst1q_u8(pDstLine5 + nLocalOffset, nDst1);
        vst1q_u8(pDstLine6 + nLocalOffset, nDst1);
        vst1q_u8(pDstLine7 + nLocalOffset, nDst1);
        
        nLocalOffset += 16;
    }
    
    return;
#endif
}


void OneRow_Mosaic16_NEON(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const RectImgAddr *pBaseRectAddr)
{
    UINT32                  i = 0, j = 0;
    UINT8                   nAvgPix = 0;
    UINT8                   *pLocalDst = NULL;
    UINT32                  nLineTable[16] = {0, };
    const UINT32            nMosaicSize = 16;
    uint8x16_t              nVecAvgPix;
    
    SET_UNUSED_VARIABLE(pSrc);
    
    for(i=0 ; i<nMosaicSize ; i++)
        nLineTable[i] = i * nDstStride;
    
    for(i=0 ; i<nWidth ; i+=nMosaicSize)
    {
        nAvgPix = (CALCULATE_INTEGRAL_SUM((*pBaseRectAddr), i)) >> 8;
        nVecAvgPix = vdupq_n_u8(nAvgPix);
        
        pLocalDst = pDst + i;
        for(j=0 ; j<nMosaicSize ; j++)
            vst1q_u8(pLocalDst + nLineTable[j], nVecAvgPix);
    }
    
    return;
}


void OneRow_Inverse_NEON(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    UINT32                  nOffset = 0;
    const INT32             nLoopCnt = (nWidth >> 4) - 1;
    uint8x16_t              nNum255 = vdupq_n_u8(255);
    
    for(i=nLoopCnt ; i>=0 ; i--)
    {
        uint8x16_t          nSrc, nDst;
        
        nSrc = vld1q_u8(pSrc + nOffset);
        nDst = vsubq_u8(nNum255, nSrc);
        vst1q_u8(pDst + nOffset, nDst);
        
        nOffset += 16;
    }
    
    return;
}


void OneRow_Clip_NEON(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nMinClipVal, IN const INT32 nMaxClipVal)
{
    const INT32             nLoopCnt = (nWidth >> 4) - 1;
    UINT32                  nOffset = 0;
    INT32                   i = 0;
    const uint8x16_t        nMinVal = vdupq_n_u8(nMinClipVal);
    const uint8x16_t        nMaxVal = vdupq_n_u8(nMaxClipVal);
    
    for(i=nLoopCnt ; i>=0 ; i-=2)
    {
        uint8x16x2_t        nSrc = vld2q_u8(pSrc + nOffset);
        
        nSrc.val[0] = vminq_u8(nSrc.val[0], nMaxVal);
        nSrc.val[1] = vminq_u8(nSrc.val[1], nMaxVal);
        
        nSrc.val[0] = vmaxq_u8(nSrc.val[0], nMinVal);
        nSrc.val[1] = vmaxq_u8(nSrc.val[1], nMinVal);
        
        vst2q_u8(pDst + nOffset, nSrc);
        
        nOffset += 32;
    }
    
    return;
}


void CustomHistogramStretch_NEON(IN const Filter_T *pFilter, IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nHeight,
                                                IN const INT32 nSrcStride, IN const INT32 nDstStride, IN const UINT8 *pLookupTbl, IN const UINT32 nThreadIdx)
{
    INT32                   i = 0, j = 0, l = 0;
    UINT32                  nOffset = 0;
    uint8x8_t               nNumTbl[8];
    uint8x8x4_t             nLookupTbl[8];
    
    // Make Const Num Table
    for(l=1 ; l<8 ; l++)
        nNumTbl[l] = vdup_n_u8(32 * l);
    
    // Load Lookup Table to NEON Register
    for(l=0 ; l<8 ; l++)
    {
        nLookupTbl[l].val[0] = vld1_u8(pLookupTbl + nOffset);
        nLookupTbl[l].val[1] = vld1_u8(pLookupTbl + nOffset + 8);
        nLookupTbl[l].val[2] = vld1_u8(pLookupTbl + nOffset + 16);
        nLookupTbl[l].val[3] = vld1_u8(pLookupTbl + nOffset + 24);
        
        nOffset += 32;
    }
    
    for(j=0 ; j<nHeight ; j++)
    {
        INT32               nLocalSrcPos = (j * nSrcStride);
        INT32               nLocalDstPos = (j * nDstStride);
        
        for(i=0 ; i<nWidth ; i+=8)
        {
            uint8x8_t       nSrc[8];
            uint8x8_t       nDst = vdup_n_u8(0);
            
            // Load Source and Make Tables
            nSrc[0] = vld1_u8(pSrc + nLocalSrcPos);
            for(l=1 ; l<8 ; l++)
                nSrc[l] = vsub_u8(nSrc[0], nNumTbl[l]);
            
            for(l=0 ; l<8 ; l++)
                nDst = vtbx4_u8(nDst, nLookupTbl[l], nSrc[l]);
            
            vst1_u8(pDst + nLocalDstPos, nDst);
            
            nLocalSrcPos += 8;
            nLocalDstPos += 8;
        }
    }
    
    return;
}


void OneRow_DiffImg_NEON(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = (pSrc0[i] - pSrc1[i]);
    
    return;
}


void OneRow_Multiply_NEON(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = (UINT8)(((UINT32)pSrc0[i] * (UINT32)pSrc1[i]) >> 8);
    
    return;
}


void OneRow_ContrastShift_NEON(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
    {
        if(pSrc0[i] > 128)
            pDst[i] = 255 - (UINT8)((UINT16)((UINT16)(pSrc1[i] * (UINT16)pSrc1[i])) >> 7);
        else
            pDst[i] = (UINT8)((UINT16)((UINT16)(pSrc0[i] * (UINT16)pSrc0[i])) >> 7);
    }
    
    return;
}


void OneRow_PropMix_NEON(IN const UINT8 *pSrc0, IN const UINT8 *pSrc1, IN const F32 *pCoeffi, OUT UINT8 *pDst, IN const INT32 nWidth)
{
    INT32                   i = 0;
    
    for(i=0 ; i<nWidth ; i++)
        pDst[i] = (pCoeffi[i] * (F32)pSrc0[i]) + ((1.0f - pCoeffi[i]) * (F32)pSrc1[i]);
    
    return;
}


void OneRow_BlurNoiseReduction_NEON(IN const UINT8 *pSrc, OUT UINT8 *pDst, IN const INT32 nWidth, IN const INT32 nSrcStride, IN const INT32 nDstStride)
{
    INT32                   i = 0;
    UINT8                   *pSrcPrev = NULL, *pSrcNext0 = NULL, *pSrcNext1 = NULL, *pSrcNext2 = NULL;
    UINT8                   *pDst0 = NULL, *pDst1 = NULL, *pDst2 = NULL;
    UINT32                  nValue[5] = {0, };
    
    pSrcPrev = (UINT8 *)(pSrc - nSrcStride);
    pSrcNext0 = (UINT8 *)(pSrc + nSrcStride);
    pSrcNext1 = (UINT8 *)(pSrc + (2 * nSrcStride));
    pSrcNext2 = (UINT8 *)(pSrc + (3 * nSrcStride));
    
    pDst0 = pDst;
    pDst1 = pDst + nDstStride;
    pDst2 = pDst + (2 * nDstStride);
    
    for(i=0 ; i<nWidth ; i++)
    {
        UINT32              nTmpDst0 = 0, nTmpDst1 = 0, nTmpDst2 = 0;
        
        nValue[0] = (UINT32)(pSrcPrev[i - 1]);
        nValue[0] += (UINT32)(pSrcPrev[i]);
        nValue[0] += (UINT32)(pSrcPrev[i + 1]);
        
        nValue[1] = (UINT32)(pSrc[i - 1]);
        nValue[1] += (UINT32)(pSrc[i]);
        nValue[1] += (UINT32)(pSrc[i + 1]);
        
        nValue[2] = (UINT32)(pSrcNext0[i - 1]);
        nValue[2] += (UINT32)(pSrcNext0[i]);
        nValue[2] += (UINT32)(pSrcNext0[i + 1]);
        
        nValue[3] = (UINT32)(pSrcNext1[i - 1]);
        nValue[3] += (UINT32)(pSrcNext1[i]);
        nValue[3] += (UINT32)(pSrcNext1[i + 1]);
        
        nValue[4] = (UINT32)(pSrcNext2[i - 1]);
        nValue[4] += (UINT32)(pSrcNext2[i]);
        nValue[4] += (UINT32)(pSrcNext2[i + 1]);
        
        nTmpDst0 = ((nValue[0] + nValue[1] + nValue[2]) / 9);
        nTmpDst1 = ((nValue[1] + nValue[2] + nValue[3]) / 9);
        nTmpDst2 = ((nValue[2] + nValue[3] + nValue[4]) / 9);
        
        if(ABSM(((UINT32)pSrc[i]) - nTmpDst0) < 5)
            *pDst0++ = (UINT8)(CLIP3(PIXEL_MIN, PIXEL_MAX, nTmpDst0));
        else
            *pDst0++ = pSrc[i];
        
        if(ABSM(((UINT32)pSrcNext0[i]) - nTmpDst1) < 5)
            *pDst1++ = (UINT8)(CLIP3(PIXEL_MIN, PIXEL_MAX, nTmpDst1));
        else
            *pDst1++ = pSrcNext0[i];
        
        if(ABSM(((UINT32)pSrcNext1[i]) - nTmpDst2) < 5)
            *pDst2++ = (UINT8)(CLIP3(PIXEL_MIN, PIXEL_MAX, nTmpDst2));
        else
            *pDst2++ = pSrcNext1[i];
    }
    
    return;
}


static void _GetMean4x4_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum)
{
    uint8x8_t           nDataU08_0, nDataU08_1, nDataU08_2, nDataU08_3;
    uint16x8_t          nSumData0, nSumData1, nSumDataVec;
    uint16x4_t          nSumData_L;
    uint64x1_t          nSumDataTotal;
    
    nDataU08_0 = vld1_u8(pSrc); pSrc += nSrcStride;
    nDataU08_1 = vld1_u8(pSrc); pSrc += nSrcStride;
    nDataU08_2 = vld1_u8(pSrc); pSrc += nSrcStride;
    nDataU08_3 = vld1_u8(pSrc);
    
    nSumData0 = vaddl_u8(nDataU08_0, nDataU08_1);
    nSumData1 = vaddl_u8(nDataU08_2, nDataU08_3);
    nSumDataVec = vaddq_u16(nSumData0, nSumData1);
    nSumData_L = vget_low_u16(nSumDataVec);
    
    nSumDataTotal = vpaddl_u32(vpaddl_u16(nSumData_L));
    
    *pSum = (UINT32)(vget_lane_u64(nSumDataTotal, 0));
}


static void _GetMean8x8_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum)
{
    uint8x8_t           nDataU08_0, nDataU08_1, nDataU08_2, nDataU08_3;
    uint8x8_t           nDataU08_4, nDataU08_5, nDataU08_6, nDataU08_7;
    uint16x8_t          nSumData0, nSumData1, nSumDataVec;
    uint16x4_t          nSumData_L, nSumData_H;
    uint64x1_t          nSumDataTotal_L, nSumDataTotal_H;
    UINT32              nTotalSum = 0;
    
    nDataU08_0 = vld1_u8(pSrc); pSrc += nSrcStride;
    nDataU08_1 = vld1_u8(pSrc); pSrc += nSrcStride;
    nDataU08_2 = vld1_u8(pSrc); pSrc += nSrcStride;
    nDataU08_3 = vld1_u8(pSrc); pSrc += nSrcStride;
    nDataU08_4 = vld1_u8(pSrc); pSrc += nSrcStride;
    nDataU08_5 = vld1_u8(pSrc); pSrc += nSrcStride;
    nDataU08_6 = vld1_u8(pSrc); pSrc += nSrcStride;
    nDataU08_7 = vld1_u8(pSrc);
    
    // For Upper 8x4
    nSumData0 = vaddl_u8(nDataU08_0, nDataU08_1);
    nSumData1 = vaddl_u8(nDataU08_2, nDataU08_3);
    nSumDataVec = vaddq_u16(nSumData0, nSumData1);
    nSumData_L = vget_low_u16(nSumDataVec);
    nSumData_H = vget_high_u16(nSumDataVec);
    nSumDataTotal_L = vpaddl_u32(vpaddl_u16(nSumData_L));
    nSumDataTotal_H = vpaddl_u32(vpaddl_u16(nSumData_H));
    nTotalSum += (UINT32)(vget_lane_u64(nSumDataTotal_L, 0));
    nTotalSum += (UINT32)(vget_lane_u64(nSumDataTotal_H, 0));

    // For Bottom 8x4
    nSumData0 = vaddl_u8(nDataU08_4, nDataU08_5);
    nSumData1 = vaddl_u8(nDataU08_6, nDataU08_7);
    nSumDataVec = vaddq_u16(nSumData0, nSumData1);
    nSumData_L = vget_low_u16(nSumDataVec);
    nSumData_L = vget_high_u16(nSumDataVec);
    nSumDataTotal_L = vpaddl_u32(vpaddl_u16(nSumData_L));
    nSumDataTotal_H = vpaddl_u32(vpaddl_u16(nSumData_H));
    nTotalSum += (UINT32)(vget_lane_u64(nSumDataTotal_L, 0));
    nTotalSum += (UINT32)(vget_lane_u64(nSumDataTotal_H, 0));
    
    *pSum = nTotalSum;
}


static void _GetMean16x16_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSum)
{
    uint8x8_t           nDataU08_0, nDataU08_1, nDataU08_2, nDataU08_3;
    uint8x8_t           nDataU08_4, nDataU08_5, nDataU08_6, nDataU08_7;
    uint16x8_t          nSumData0, nSumData1, nSumDataVec;
    uint16x4_t          nSumData_L, nSumData_H;
    uint64x1_t          nSumDataTotal_L, nSumDataTotal_H;
    UINT32              nTotalSum = 0;
    INT32               i = 0;
    
    for(i=0 ; i<4 ; i++)
    {
        nDataU08_0 = vld1_u8(pSrc);
        nDataU08_1 = vld1_u8(pSrc + 8); pSrc += nSrcStride;
        nDataU08_2 = vld1_u8(pSrc);
        nDataU08_3 = vld1_u8(pSrc + 8); pSrc += nSrcStride;
        nDataU08_4 = vld1_u8(pSrc);
        nDataU08_5 = vld1_u8(pSrc + 8); pSrc += nSrcStride;
        nDataU08_6 = vld1_u8(pSrc);
        nDataU08_7 = vld1_u8(pSrc + 8); pSrc += nSrcStride;
        
        // For Upper 16x4
        nSumData0 = vaddl_u8(nDataU08_0, nDataU08_1);
        nSumData1 = vaddl_u8(nDataU08_2, nDataU08_3);
        nSumDataVec = vaddq_u16(nSumData0, nSumData1);
        nSumData_L = vget_low_u16(nSumDataVec);
        nSumData_L = vget_high_u16(nSumDataVec);
        nSumDataTotal_L = vpaddl_u32(vpaddl_u16(nSumData_L));
        nSumDataTotal_H = vpaddl_u32(vpaddl_u16(nSumData_H));
        nTotalSum += (UINT32)(vget_lane_u64(nSumDataTotal_L, 0));
        nTotalSum += (UINT32)(vget_lane_u64(nSumDataTotal_H, 0));
        
        // For Bottom 16x2
        nSumData0 = vaddl_u8(nDataU08_4, nDataU08_5);
        nSumData1 = vaddl_u8(nDataU08_6, nDataU08_7);
        nSumDataVec = vaddq_u16(nSumData0, nSumData1);
        nSumData_L = vget_low_u16(nSumDataVec);
        nSumData_L = vget_high_u16(nSumDataVec);
        nSumDataTotal_L = vpaddl_u32(vpaddl_u16(nSumData_L));
        nSumDataTotal_H = vpaddl_u32(vpaddl_u16(nSumData_H));
        nTotalSum += (UINT32)(vget_lane_u64(nSumDataTotal_L, 0));
        nTotalSum += (UINT32)(vget_lane_u64(nSumDataTotal_H, 0));
    }
    
    *pSum = nTotalSum;
}


static void _GetVar4x4_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum)
{
    uint8x8_t           nDataU08_0, nDataU08_1, nDataU08_2, nDataU08_3;
    uint16x8_t          nSumData0, nSumData1, nSumDataVec;
    uint16x4_t          nSqData0, nSqData1, nSqData2, nSqData3;
    uint16x4_t          nSumData_L;
    uint64x1_t          nSumDataTotal;
    uint64x1_t          nSqSumDataTotal0, nSqSumDataTotal1, nSqSumDataTotal2, nSqSumDataTotal3;
    
    nDataU08_0 = vld1_u8(pSrc); pSrc += nSrcStride;
    nDataU08_1 = vld1_u8(pSrc); pSrc += nSrcStride;
    nDataU08_2 = vld1_u8(pSrc); pSrc += nSrcStride;
    nDataU08_3 = vld1_u8(pSrc);
    
    // Get Sum
    nSumData0 = vaddl_u8(nDataU08_0, nDataU08_1);
    nSumData1 = vaddl_u8(nDataU08_2, nDataU08_3);
    nSumDataVec = vaddq_u16(nSumData0, nSumData1);
    nSumData_L = vget_low_u16(nSumDataVec);
    nSumDataTotal = vpaddl_u32(vpaddl_u16(nSumData_L));
    
    // Get Squared Sum
    nSqData0 = vget_low_u16(vmull_u8(nDataU08_0, nDataU08_0));
    nSqData1 = vget_low_u16(vmull_u8(nDataU08_1, nDataU08_1));
    nSqData2 = vget_low_u16(vmull_u8(nDataU08_2, nDataU08_2));
    nSqData3 = vget_low_u16(vmull_u8(nDataU08_3, nDataU08_3));
    nSqSumDataTotal0 = vpaddl_u32(vpaddl_u16(nSqData0));
    nSqSumDataTotal1 = vpaddl_u32(vpaddl_u16(nSqData1));
    nSqSumDataTotal2 = vpaddl_u32(vpaddl_u16(nSqData2));
    nSqSumDataTotal3 = vpaddl_u32(vpaddl_u16(nSqData3));
    
    *pSum = (UINT32)(vget_lane_u64(nSumDataTotal, 0));
    *pSqSum = (UINT32)(vget_lane_u64(nSqSumDataTotal0, 0)) + (UINT32)(vget_lane_u64(nSqSumDataTotal1, 0))
            + (UINT32)(vget_lane_u64(nSqSumDataTotal2, 0)) + (UINT32)(vget_lane_u64(nSqSumDataTotal3, 0));
}


static void _GetVar8x8_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum)
{
    uint8x8_t           nDataU08_0, nDataU08_1, nDataU08_2, nDataU08_3;
    uint16x8_t          nSumData0, nSumData1, nSumDataVec;
    uint16x8_t          nSqData0, nSqData1, nSqData2, nSqData3;
    uint16x4_t          nSqData0_L, nSqData1_L, nSqData2_L, nSqData3_L;
    uint16x4_t          nSqData0_H, nSqData1_H, nSqData2_H, nSqData3_H;
    uint16x4_t          nSumData_L, nSumData_H;
    uint64x1_t          nSumDataTotal_L, nSumDataTotal_H;
    uint64x1_t          nSqSumDataTotal0_L, nSqSumDataTotal1_L, nSqSumDataTotal2_L, nSqSumDataTotal3_L;
    uint64x1_t          nSqSumDataTotal0_H, nSqSumDataTotal1_H, nSqSumDataTotal2_H, nSqSumDataTotal3_H;
    INT32               i = 0;
    
    *pSum = 0;
    *pSqSum = 0;

    for(i=0 ; i<2 ; i++)
    {
        nDataU08_0 = vld1_u8(pSrc); pSrc += nSrcStride;
        nDataU08_1 = vld1_u8(pSrc); pSrc += nSrcStride;
        nDataU08_2 = vld1_u8(pSrc); pSrc += nSrcStride;
        nDataU08_3 = vld1_u8(pSrc); pSrc += nSrcStride;
        
        // Get Sum
        nSumData0 = vaddl_u8(nDataU08_0, nDataU08_1);
        nSumData1 = vaddl_u8(nDataU08_2, nDataU08_3);
        nSumDataVec = vaddq_u16(nSumData0, nSumData1);
        nSumData_L = vget_low_u16(nSumDataVec);
        nSumData_H = vget_high_u16(nSumDataVec);
        nSumDataTotal_L = vpaddl_u32(vpaddl_u16(nSumData_L));
        nSumDataTotal_H = vpaddl_u32(vpaddl_u16(nSumData_H));
        *pSum += ((UINT32)(vget_lane_u64(nSumDataTotal_L, 0)) + (UINT32)(vget_lane_u64(nSumDataTotal_H, 0)));
        
        // Get Squared Sum
        nSqData0 = vmull_u8(nDataU08_0, nDataU08_0);
        nSqData1 = vmull_u8(nDataU08_1, nDataU08_1);
        nSqData2 = vmull_u8(nDataU08_2, nDataU08_2);
        nSqData3 = vmull_u8(nDataU08_3, nDataU08_3);
        nSqData0_L = vget_low_u16(nSqData0);
        nSqData1_L = vget_low_u16(nSqData1);
        nSqData2_L = vget_low_u16(nSqData2);
        nSqData3_L = vget_low_u16(nSqData3);
        nSqData0_H = vget_high_u16(nSqData0);
        nSqData1_H = vget_high_u16(nSqData1);
        nSqData2_H = vget_high_u16(nSqData2);
        nSqData3_H = vget_high_u16(nSqData3);
     
        nSqSumDataTotal0_L = vpaddl_u32(vpaddl_u16(nSqData0_L));
        nSqSumDataTotal1_L = vpaddl_u32(vpaddl_u16(nSqData1_L));
        nSqSumDataTotal2_L = vpaddl_u32(vpaddl_u16(nSqData2_L));
        nSqSumDataTotal3_L = vpaddl_u32(vpaddl_u16(nSqData3_L));
        nSqSumDataTotal0_H = vpaddl_u32(vpaddl_u16(nSqData0_H));
        nSqSumDataTotal1_H = vpaddl_u32(vpaddl_u16(nSqData1_H));
        nSqSumDataTotal2_H = vpaddl_u32(vpaddl_u16(nSqData2_H));
        nSqSumDataTotal3_H = vpaddl_u32(vpaddl_u16(nSqData3_H));
        
        *pSqSum += (UINT32)(vget_lane_u64(nSqSumDataTotal0_L, 0)) + (UINT32)(vget_lane_u64(nSqSumDataTotal1_L, 0))
                 + (UINT32)(vget_lane_u64(nSqSumDataTotal2_L, 0)) + (UINT32)(vget_lane_u64(nSqSumDataTotal3_L, 0))
                 + (UINT32)(vget_lane_u64(nSqSumDataTotal0_H, 0)) + (UINT32)(vget_lane_u64(nSqSumDataTotal1_H, 0))
                 + (UINT32)(vget_lane_u64(nSqSumDataTotal2_H, 0)) + (UINT32)(vget_lane_u64(nSqSumDataTotal3_H, 0));
    }
}


static void _GetVar16x16_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, OUT UINT32 *pSqSum, OUT UINT32 *pSum)
{
    uint8x8_t           nDataU08_0, nDataU08_1, nDataU08_2, nDataU08_3;
    uint16x8_t          nSumData0, nSumData1, nSumDataVec;
    uint16x8_t          nSqData0, nSqData1, nSqData2, nSqData3;
    uint16x4_t          nSqData0_L, nSqData1_L, nSqData2_L, nSqData3_L;
    uint16x4_t          nSqData0_H, nSqData1_H, nSqData2_H, nSqData3_H;
    uint16x4_t          nSumData_L, nSumData_H;
    uint64x1_t          nSumDataTotal_L, nSumDataTotal_H;
    uint64x1_t          nSqSumDataTotal0_L, nSqSumDataTotal1_L, nSqSumDataTotal2_L, nSqSumDataTotal3_L;
    uint64x1_t          nSqSumDataTotal0_H, nSqSumDataTotal1_H, nSqSumDataTotal2_H, nSqSumDataTotal3_H;
    INT32               i = 0, j = 0;
    
    *pSum = 0;
    *pSqSum = 0;

    for(j=0 ; j<4 ; j++)
    {
        const UINT8     *pLocalSrc = pSrc;
        
        for(i=0 ; i<2 ; i++)
        {
            nDataU08_0 = vld1_u8(pLocalSrc); pLocalSrc += nSrcStride;
            nDataU08_1 = vld1_u8(pLocalSrc); pLocalSrc += nSrcStride;
            nDataU08_2 = vld1_u8(pLocalSrc); pLocalSrc += nSrcStride;
            nDataU08_3 = vld1_u8(pLocalSrc);
            
            // Get Sum
            nSumData0 = vaddl_u8(nDataU08_0, nDataU08_1);
            nSumData1 = vaddl_u8(nDataU08_2, nDataU08_3);
            nSumDataVec = vaddq_u16(nSumData0, nSumData1);
            nSumData_L = vget_low_u16(nSumDataVec);
            nSumData_H = vget_high_u16(nSumDataVec);
            nSumDataTotal_L = vpaddl_u32(vpaddl_u16(nSumData_L));
            nSumDataTotal_H = vpaddl_u32(vpaddl_u16(nSumData_H));
            *pSum += ((UINT32)(vget_lane_u64(nSumDataTotal_L, 0)) + (UINT32)(vget_lane_u64(nSumDataTotal_H, 0)));
            
            // Get Squared Sum
            nSqData0 = vmull_u8(nDataU08_0, nDataU08_0);
            nSqData1 = vmull_u8(nDataU08_1, nDataU08_1);
            nSqData2 = vmull_u8(nDataU08_2, nDataU08_2);
            nSqData3 = vmull_u8(nDataU08_3, nDataU08_3);
            nSqData0_L = vget_low_u16(nSqData0);
            nSqData1_L = vget_low_u16(nSqData1);
            nSqData2_L = vget_low_u16(nSqData2);
            nSqData3_L = vget_low_u16(nSqData3);
            nSqData0_H = vget_high_u16(nSqData0);
            nSqData1_H = vget_high_u16(nSqData1);
            nSqData2_H = vget_high_u16(nSqData2);
            nSqData3_H = vget_high_u16(nSqData3);
            
            nSqSumDataTotal0_L = vpaddl_u32(vpaddl_u16(nSqData0_L));
            nSqSumDataTotal1_L = vpaddl_u32(vpaddl_u16(nSqData1_L));
            nSqSumDataTotal2_L = vpaddl_u32(vpaddl_u16(nSqData2_L));
            nSqSumDataTotal3_L = vpaddl_u32(vpaddl_u16(nSqData3_L));
            nSqSumDataTotal0_H = vpaddl_u32(vpaddl_u16(nSqData0_H));
            nSqSumDataTotal1_H = vpaddl_u32(vpaddl_u16(nSqData1_H));
            nSqSumDataTotal2_H = vpaddl_u32(vpaddl_u16(nSqData2_H));
            nSqSumDataTotal3_H = vpaddl_u32(vpaddl_u16(nSqData3_H));
            
            *pSqSum += (UINT32)(vget_lane_u64(nSqSumDataTotal0_L, 0)) + (UINT32)(vget_lane_u64(nSqSumDataTotal1_L, 0))
                     + (UINT32)(vget_lane_u64(nSqSumDataTotal2_L, 0)) + (UINT32)(vget_lane_u64(nSqSumDataTotal3_L, 0))
                     + (UINT32)(vget_lane_u64(nSqSumDataTotal0_H, 0)) + (UINT32)(vget_lane_u64(nSqSumDataTotal1_H, 0))
                     + (UINT32)(vget_lane_u64(nSqSumDataTotal2_H, 0)) + (UINT32)(vget_lane_u64(nSqSumDataTotal3_H, 0));
            
            pLocalSrc += 8 - (3 * nSrcStride);
        }
        
        pSrc += (4 * nSrcStride);
    }
}


static void _GetDiffVar4x4_NEON(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                                IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum)
{
    uint16x8_t          nDataU16_0, nDataU16_1, nDataU16_2, nDataU16_3;
    int16x8_t           nDataS16_0, nDataS16_1, nDataS16_2, nDataS16_3;
    int16x8_t           nSumData0, nSumData1;
    int16x4_t           nDataS16_0_L, nDataS16_1_L, nDataS16_2_L, nDataS16_3_L;
    uint32x4_t          nSqData0, nSqData1, nSqData2, nSqData3;
    int16x4_t           nSumData_L;
    int64x1_t           nSumDataTotal;
    uint64x2_t          nSqSumDataTotal0, nSqSumDataTotal1, nSqSumDataTotal2, nSqSumDataTotal3;
    
    nDataU16_0 = vsubl_u8(vld1_u8(pSrc0), vld1_u8(pSrc1)); pSrc0 += nSrcStride0; pSrc1 += nSrcStride1;
    nDataU16_1 = vsubl_u8(vld1_u8(pSrc0), vld1_u8(pSrc1)); pSrc0 += nSrcStride0; pSrc1 += nSrcStride1;
    nDataU16_2 = vsubl_u8(vld1_u8(pSrc0), vld1_u8(pSrc1)); pSrc0 += nSrcStride0; pSrc1 += nSrcStride1;
    nDataU16_3 = vsubl_u8(vld1_u8(pSrc0), vld1_u8(pSrc1));

    nDataS16_0 = vreinterpretq_s16_u16(nDataU16_0);
    nDataS16_1 = vreinterpretq_s16_u16(nDataU16_1);
    nDataS16_2 = vreinterpretq_s16_u16(nDataU16_2);
    nDataS16_3 = vreinterpretq_s16_u16(nDataU16_3);
    
    // Get Sum
    nSumData0 = vaddq_s16(nDataS16_0, nDataS16_1);
    nSumData1 = vaddq_s16(nDataS16_2, nDataS16_3);
    nSumData_L = vget_low_s16(vaddq_s16(nSumData0, nSumData1));
    nSumDataTotal = vpaddl_s32(vpaddl_s16(nSumData_L));
    
    // Get Squared Sum
    nDataS16_0_L = vget_low_s16(nDataS16_0);
    nDataS16_1_L = vget_low_s16(nDataS16_1);
    nDataS16_2_L = vget_low_s16(nDataS16_2);
    nDataS16_3_L = vget_low_s16(nDataS16_3);
    nSqData0 = vreinterpretq_u32_s32(vmull_s16(nDataS16_0_L, nDataS16_0_L));
    nSqData1 = vreinterpretq_u32_s32(vmull_s16(nDataS16_1_L, nDataS16_1_L));
    nSqData2 = vreinterpretq_u32_s32(vmull_s16(nDataS16_2_L, nDataS16_2_L));
    nSqData3 = vreinterpretq_u32_s32(vmull_s16(nDataS16_3_L, nDataS16_3_L));
    nSqSumDataTotal0 = vpaddlq_u32(nSqData0);
    nSqSumDataTotal1 = vpaddlq_u32(nSqData1);
    nSqSumDataTotal2 = vpaddlq_u32(nSqData2);
    nSqSumDataTotal3 = vpaddlq_u32(nSqData3);
    nSqSumDataTotal0 = vaddq_u64(nSqSumDataTotal0, nSqSumDataTotal1);
    nSqSumDataTotal1 = vaddq_u64(nSqSumDataTotal2, nSqSumDataTotal3);
    
    *pSum = (INT16)(vget_lane_u64(nSumDataTotal, 0));
    *pSqSum = (UINT32)(vgetq_lane_u64(nSqSumDataTotal0, 0)) + (UINT32)(vgetq_lane_u64(nSqSumDataTotal0, 1))
            + (UINT32)(vgetq_lane_u64(nSqSumDataTotal1, 0)) + (UINT32)(vgetq_lane_u64(nSqSumDataTotal1, 1));
}


static void _GetDiffVar8x8_NEON(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                             IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum)
{
    uint16x8_t          nDataU16_0, nDataU16_1, nDataU16_2, nDataU16_3;
    int16x8_t           nDataS16_0, nDataS16_1, nDataS16_2, nDataS16_3;
    int16x8_t           nSumData0, nSumData1, nSumData;
    int16x4_t           nDataS16_0_L, nDataS16_1_L, nDataS16_2_L, nDataS16_3_L;
    int16x4_t           nDataS16_0_H, nDataS16_1_H, nDataS16_2_H, nDataS16_3_H;
    uint32x4_t          nSqData0_L, nSqData1_L, nSqData2_L, nSqData3_L;
    uint32x4_t          nSqData0_H, nSqData1_H, nSqData2_H, nSqData3_H;
    int16x4_t           nSumData_L, nSumData_H;
    int64x1_t           nSumDataTotal_L, nSumDataTotal_H;
    uint64x2_t          nSqSumDataTotal0_L, nSqSumDataTotal1_L, nSqSumDataTotal2_L, nSqSumDataTotal3_L;
    uint64x2_t          nSqSumDataTotal0_H, nSqSumDataTotal1_H, nSqSumDataTotal2_H, nSqSumDataTotal3_H;
    INT32               i = 0;
    
    *pSum = 0;
    *pSqSum = 0;

    for(i=0 ; i<2 ; i++)
    {
        nDataU16_0 = vsubl_u8(vld1_u8(pSrc0), vld1_u8(pSrc1)); pSrc0 += nSrcStride0; pSrc1 += nSrcStride1;
        nDataU16_1 = vsubl_u8(vld1_u8(pSrc0), vld1_u8(pSrc1)); pSrc0 += nSrcStride0; pSrc1 += nSrcStride1;
        nDataU16_2 = vsubl_u8(vld1_u8(pSrc0), vld1_u8(pSrc1)); pSrc0 += nSrcStride0; pSrc1 += nSrcStride1;
        nDataU16_3 = vsubl_u8(vld1_u8(pSrc0), vld1_u8(pSrc1)); pSrc0 += nSrcStride0; pSrc1 += nSrcStride1;
        
        nDataS16_0 = vreinterpretq_s16_u16(nDataU16_0);
        nDataS16_1 = vreinterpretq_s16_u16(nDataU16_1);
        nDataS16_2 = vreinterpretq_s16_u16(nDataU16_2);
        nDataS16_3 = vreinterpretq_s16_u16(nDataU16_3);
        
        // Get Sum
        nSumData0 = vaddq_s16(nDataS16_0, nDataS16_1);
        nSumData1 = vaddq_s16(nDataS16_2, nDataS16_3);
        nSumData = vaddq_s16(nSumData0, nSumData1);
        nSumData_L = vget_low_s16(nSumData);
        nSumData_H = vget_high_s16(nSumData);
        nSumDataTotal_L = vpaddl_s32(vpaddl_s16(nSumData_L));
        nSumDataTotal_H = vpaddl_s32(vpaddl_s16(nSumData_H));
        
        // Get Squared Sum
        nDataS16_0_L = vget_low_s16(nDataS16_0);
        nDataS16_1_L = vget_low_s16(nDataS16_1);
        nDataS16_2_L = vget_low_s16(nDataS16_2);
        nDataS16_3_L = vget_low_s16(nDataS16_3);
        nDataS16_0_H = vget_high_s16(nDataS16_0);
        nDataS16_1_H = vget_high_s16(nDataS16_1);
        nDataS16_2_H = vget_high_s16(nDataS16_2);
        nDataS16_3_H = vget_high_s16(nDataS16_3);
        nSqData0_L = vreinterpretq_u32_s32(vmull_s16(nDataS16_0_L, nDataS16_0_L));
        nSqData1_L = vreinterpretq_u32_s32(vmull_s16(nDataS16_1_L, nDataS16_1_L));
        nSqData2_L = vreinterpretq_u32_s32(vmull_s16(nDataS16_2_L, nDataS16_2_L));
        nSqData3_L = vreinterpretq_u32_s32(vmull_s16(nDataS16_3_L, nDataS16_3_L));
        nSqData0_H = vreinterpretq_u32_s32(vmull_s16(nDataS16_0_H, nDataS16_0_H));
        nSqData1_H = vreinterpretq_u32_s32(vmull_s16(nDataS16_1_H, nDataS16_1_H));
        nSqData2_H = vreinterpretq_u32_s32(vmull_s16(nDataS16_2_H, nDataS16_2_H));
        nSqData3_H = vreinterpretq_u32_s32(vmull_s16(nDataS16_3_H, nDataS16_3_H));
        nSqSumDataTotal0_L = vpaddlq_u32(nSqData0_L);
        nSqSumDataTotal1_L = vpaddlq_u32(nSqData1_L);
        nSqSumDataTotal2_L = vpaddlq_u32(nSqData2_L);
        nSqSumDataTotal3_L = vpaddlq_u32(nSqData3_L);
        nSqSumDataTotal0_H = vpaddlq_u32(nSqData0_H);
        nSqSumDataTotal1_H = vpaddlq_u32(nSqData1_H);
        nSqSumDataTotal2_H = vpaddlq_u32(nSqData2_H);
        nSqSumDataTotal3_H = vpaddlq_u32(nSqData3_H);
        nSqSumDataTotal0_L = vaddq_u64(nSqSumDataTotal0_L, nSqSumDataTotal1_L);
        nSqSumDataTotal1_L = vaddq_u64(nSqSumDataTotal2_L, nSqSumDataTotal3_L);
        nSqSumDataTotal0_H = vaddq_u64(nSqSumDataTotal0_H, nSqSumDataTotal1_H);
        nSqSumDataTotal1_H = vaddq_u64(nSqSumDataTotal2_H, nSqSumDataTotal3_H);
        
        *pSum += (INT16)(vget_lane_u64(nSumDataTotal_L, 0)) + (INT16)(vget_lane_u64(nSumDataTotal_H, 0));
        *pSqSum += ((UINT32)(vgetq_lane_u64(nSqSumDataTotal0_L, 0)) + (UINT32)(vgetq_lane_u64(nSqSumDataTotal0_L, 1))
                 + (UINT32)(vgetq_lane_u64(nSqSumDataTotal1_L, 0)) + (UINT32)(vgetq_lane_u64(nSqSumDataTotal1_L, 1))
                 + (UINT32)(vgetq_lane_u64(nSqSumDataTotal0_H, 0)) + (UINT32)(vgetq_lane_u64(nSqSumDataTotal0_H, 1))
                 + (UINT32)(vgetq_lane_u64(nSqSumDataTotal1_H, 0)) + (UINT32)(vgetq_lane_u64(nSqSumDataTotal1_H, 1)));
    }
}


static void _GetDiffVar16x16_NEON(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0,
                               IN const UINT8 *pSrc1, IN const INT32 nSrcStride1, OUT UINT32 *pSqSum, OUT INT32 *pSum)
{
    uint16x8_t          nDataU16_0, nDataU16_1, nDataU16_2, nDataU16_3;
    int16x8_t           nDataS16_0, nDataS16_1, nDataS16_2, nDataS16_3;
    int16x8_t           nSumData0, nSumData1, nSumData;
    int16x4_t           nDataS16_0_L, nDataS16_1_L, nDataS16_2_L, nDataS16_3_L;
    int16x4_t           nDataS16_0_H, nDataS16_1_H, nDataS16_2_H, nDataS16_3_H;
    uint32x4_t          nSqData0_L, nSqData1_L, nSqData2_L, nSqData3_L;
    uint32x4_t          nSqData0_H, nSqData1_H, nSqData2_H, nSqData3_H;
    int16x4_t           nSumData_L, nSumData_H;
    int64x1_t           nSumDataTotal_L, nSumDataTotal_H;
    uint64x2_t          nSqSumDataTotal0_L, nSqSumDataTotal1_L, nSqSumDataTotal2_L, nSqSumDataTotal3_L;
    uint64x2_t          nSqSumDataTotal0_H, nSqSumDataTotal1_H, nSqSumDataTotal2_H, nSqSumDataTotal3_H;
    INT32               i = 0, j = 0;
    
    *pSum = 0;
    *pSqSum = 0;
    
    for(j=0 ; j<2 ; j++)
    {
        const UINT8     *pLocalSrc0 = pSrc0;
        const UINT8     *pLocalSrc1 = pSrc1;
        
        for(i=0 ; i<4 ; i++)
        {
            nDataU16_0 = vsubl_u8(vld1_u8(pLocalSrc0), vld1_u8(pLocalSrc1)); pLocalSrc0 += nSrcStride0; pLocalSrc1 += nSrcStride1;
            nDataU16_1 = vsubl_u8(vld1_u8(pLocalSrc0), vld1_u8(pLocalSrc1)); pLocalSrc0 += nSrcStride0; pLocalSrc1 += nSrcStride1;
            nDataU16_2 = vsubl_u8(vld1_u8(pLocalSrc0), vld1_u8(pLocalSrc1)); pLocalSrc0 += nSrcStride0; pLocalSrc1 += nSrcStride1;
            nDataU16_3 = vsubl_u8(vld1_u8(pLocalSrc0), vld1_u8(pLocalSrc1)); pLocalSrc0 += nSrcStride0; pLocalSrc1 += nSrcStride1;
            
            nDataS16_0 = vreinterpretq_s16_u16(nDataU16_0);
            nDataS16_1 = vreinterpretq_s16_u16(nDataU16_1);
            nDataS16_2 = vreinterpretq_s16_u16(nDataU16_2);
            nDataS16_3 = vreinterpretq_s16_u16(nDataU16_3);
            
            // Get Sum
            nSumData0 = vaddq_s16(nDataS16_0, nDataS16_1);
            nSumData1 = vaddq_s16(nDataS16_2, nDataS16_3);
            nSumData = vaddq_s16(nSumData0, nSumData1);
            nSumData_L = vget_low_s16(nSumData);
            nSumData_H = vget_high_s16(nSumData);
            nSumDataTotal_L = vpaddl_s32(vpaddl_s16(nSumData_L));
            nSumDataTotal_H = vpaddl_s32(vpaddl_s16(nSumData_H));
            
            // Get Squared Sum
            nDataS16_0_L = vget_low_s16(nDataS16_0);
            nDataS16_1_L = vget_low_s16(nDataS16_1);
            nDataS16_2_L = vget_low_s16(nDataS16_2);
            nDataS16_3_L = vget_low_s16(nDataS16_3);
            nDataS16_0_H = vget_high_s16(nDataS16_0);
            nDataS16_1_H = vget_high_s16(nDataS16_1);
            nDataS16_2_H = vget_high_s16(nDataS16_2);
            nDataS16_3_H = vget_high_s16(nDataS16_3);
            nSqData0_L = vreinterpretq_u32_s32(vmull_s16(nDataS16_0_L, nDataS16_0_L));
            nSqData1_L = vreinterpretq_u32_s32(vmull_s16(nDataS16_1_L, nDataS16_1_L));
            nSqData2_L = vreinterpretq_u32_s32(vmull_s16(nDataS16_2_L, nDataS16_2_L));
            nSqData3_L = vreinterpretq_u32_s32(vmull_s16(nDataS16_3_L, nDataS16_3_L));
            nSqData0_H = vreinterpretq_u32_s32(vmull_s16(nDataS16_0_H, nDataS16_0_H));
            nSqData1_H = vreinterpretq_u32_s32(vmull_s16(nDataS16_1_H, nDataS16_1_H));
            nSqData2_H = vreinterpretq_u32_s32(vmull_s16(nDataS16_2_H, nDataS16_2_H));
            nSqData3_H = vreinterpretq_u32_s32(vmull_s16(nDataS16_3_H, nDataS16_3_H));
            nSqSumDataTotal0_L = vpaddlq_u32(nSqData0_L);
            nSqSumDataTotal1_L = vpaddlq_u32(nSqData1_L);
            nSqSumDataTotal2_L = vpaddlq_u32(nSqData2_L);
            nSqSumDataTotal3_L = vpaddlq_u32(nSqData3_L);
            nSqSumDataTotal0_H = vpaddlq_u32(nSqData0_H);
            nSqSumDataTotal1_H = vpaddlq_u32(nSqData1_H);
            nSqSumDataTotal2_H = vpaddlq_u32(nSqData2_H);
            nSqSumDataTotal3_H = vpaddlq_u32(nSqData3_H);
            nSqSumDataTotal0_L = vaddq_u64(nSqSumDataTotal0_L, nSqSumDataTotal1_L);
            nSqSumDataTotal1_L = vaddq_u64(nSqSumDataTotal2_L, nSqSumDataTotal3_L);
            nSqSumDataTotal0_H = vaddq_u64(nSqSumDataTotal0_H, nSqSumDataTotal1_H);
            nSqSumDataTotal1_H = vaddq_u64(nSqSumDataTotal2_H, nSqSumDataTotal3_H);
            
            *pSum += (INT16)(vget_lane_u64(nSumDataTotal_L, 0)) + (INT16)(vget_lane_u64(nSumDataTotal_H, 0));
            *pSqSum += ((UINT32)(vgetq_lane_u64(nSqSumDataTotal0_L, 0)) + (UINT32)(vgetq_lane_u64(nSqSumDataTotal0_L, 1))
                      + (UINT32)(vgetq_lane_u64(nSqSumDataTotal1_L, 0)) + (UINT32)(vgetq_lane_u64(nSqSumDataTotal1_L, 1))
                      + (UINT32)(vgetq_lane_u64(nSqSumDataTotal0_H, 0)) + (UINT32)(vgetq_lane_u64(nSqSumDataTotal0_H, 1))
                      + (UINT32)(vgetq_lane_u64(nSqSumDataTotal1_H, 0)) + (UINT32)(vgetq_lane_u64(nSqSumDataTotal1_H, 1)));
        }
        
        pSrc0 += 8;
        pSrc1 += 8;
    }
}


static void _GetMean_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nWidth, IN const INT32 nHeight,
                          OUT UINT32 *pSum, fpgetNxMMean fpMean, IN const INT32 nBlockSize)
{
    INT32                   i = 0, j = 0;
    
    *pSum = 0;
    
    for(i=0 ; i<nHeight ; i+=nBlockSize)
    {
        for(j=0 ; j<nWidth ; j+=nBlockSize)
        {
            UINT32          nLocalSum = 0;
            
            fpMean(pSrc + nSrcStride * i + j, nSrcStride, &nLocalSum);
            
            *pSum += nLocalSum;
        }
    }
}


static void _GetVariance_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nWidth, IN const INT32 nHeight,
                           OUT UINT32 *pSqSum, OUT UINT32 *pSum, fpgetNxMVar fpVar, IN const INT32 nBlockSize)
{
    INT32                   i = 0, j = 0;
    
    *pSum = 0;
    *pSqSum = 0;
    
    for(i=0 ; i<nHeight ; i+=nBlockSize)
    {
        for(j=0 ; j<nWidth ; j+=nBlockSize)
        {
            UINT32          nLocalSum = 0;
            UINT32          nLocalSqSum = 0;
            
            fpVar(pSrc + nSrcStride * i + j, nSrcStride, &nLocalSqSum, &nLocalSum);
            
            *pSum += nLocalSum;
            *pSqSum += nLocalSqSum;
        }
    }
}


static void _GetDiffVariance_NEON(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1,
                               IN const INT32 nWidth, IN const INT32 nHeight, OUT UINT32 *pSqSum, OUT INT32 *pSum, fpgetNxMDiffVar fpDiffVar, IN const INT32 nBlockSize)
{
    INT32                   i = 0, j = 0;
    
    *pSum = 0;
    *pSqSum = 0;
    
    for(i=0 ; i<nHeight ; i+=nBlockSize)
    {
        for(j=0 ; j<nWidth ; j+=nBlockSize)
        {
            INT32           nLocalSum = 0;
            UINT32          nLocalSqSum = 0;
            
            fpDiffVar(pSrc0 + nSrcStride0 * i + j, nSrcStride0,
                      pSrc1 + nSrcStride1 * i + j, nSrcStride1, &nLocalSqSum, &nLocalSum);
            
            *pSum += nLocalSum;
            *pSqSum += nLocalSqSum;
        }
    }
}


F32 Mean_MxN_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSum)
{
    const INT32             nPatchSize = nSizeX * nSizeY;
    UINT32                  nLocalSum = 0;
    
    if(16 == nPatchSize)
        _GetMean4x4_NEON(pSrc, nSrcStride, &nLocalSum);
    else if(32 == nPatchSize)
        _GetMean_NEON(pSrc, nSrcStride, nSizeX, nSizeY, &nLocalSum, _GetMean4x4_NEON, 4);
    else if(64 == nPatchSize)
        _GetMean8x8_NEON(pSrc, nSrcStride, &nLocalSum);
    else if(128 == nPatchSize)
        _GetMean_NEON(pSrc, nSrcStride, nSizeX, nSizeY, &nLocalSum, _GetMean8x8_NEON, 8);
    else if(256 == nPatchSize)
        _GetMean16x16_NEON(pSrc, nSrcStride, &nLocalSum);
    
    *pSum = nLocalSum;
    
    return (F32)nLocalSum / (F32)nPatchSize;
}


F32 Variance_MxN_NEON(IN const UINT8 *pSrc, IN const INT32 nSrcStride, IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSqSum, OUT UINT32 *pSum)
{
    const INT32             nPatchSize = nSizeX * nSizeY;
    UINT32                  nLocalSum = 0;
    
    if(16 == nPatchSize)
        _GetVar4x4_NEON(pSrc, nSrcStride, pSqSum, &nLocalSum);
    else if(32 == nPatchSize)
        _GetVariance_NEON(pSrc, nSrcStride, nSizeX, nSizeY, pSqSum, &nLocalSum, _GetVar4x4_NEON, 4);
    else if(64 == nPatchSize)
        _GetVar8x8_NEON(pSrc, nSrcStride, pSqSum, &nLocalSum);
    else if(128 == nPatchSize)
        _GetVariance_NEON(pSrc, nSrcStride, nSizeX, nSizeY, pSqSum, &nLocalSum, _GetVar8x8_NEON, 8);
    else if(256 == nPatchSize)
        _GetVar16x16_NEON(pSrc, nSrcStride, pSqSum, &nLocalSum);
    
    *pSum = nLocalSum;
    
    return ((F32)(*pSqSum) / (F32)nPatchSize) - ((F32)((*pSum) * (*pSum)) / (F32)(nPatchSize * nPatchSize));
}


F32 DiffVariance_MxN_NEON(IN const UINT8 *pSrc0, IN const INT32 nSrcStride0, IN const UINT8 *pSrc1, IN const INT32 nSrcStride1,
                          IN const INT32 nSizeX, IN const INT32 nSizeY, OUT UINT32 *pSqSum, OUT INT32 *pSum)
{
    const INT32             nPatchSize = nSizeX * nSizeY;
    INT32                   nLocalSum = 0;
    
    if(16 == nPatchSize)
        _GetDiffVar4x4_NEON(pSrc0, nSrcStride0, pSrc1, nSrcStride1, pSqSum, &nLocalSum);
    else if(32 == nPatchSize)
        _GetDiffVariance_NEON(pSrc0, nSrcStride0, pSrc1, nSrcStride1, nSizeX, nSizeY, pSqSum, &nLocalSum, _GetDiffVar4x4_NEON, 4);
    else if(64 == nPatchSize)
        _GetDiffVar8x8_NEON(pSrc0, nSrcStride0, pSrc1, nSrcStride1, pSqSum, &nLocalSum);
    else if(128 == nPatchSize)
        _GetDiffVariance_NEON(pSrc0, nSrcStride0, pSrc1, nSrcStride1, nSizeX, nSizeY, pSqSum, &nLocalSum, _GetDiffVar8x8_NEON, 8);
    else if(256 == nPatchSize)
        _GetDiffVar16x16_NEON(pSrc0, nSrcStride0, pSrc1, nSrcStride1, pSqSum, &nLocalSum);
    
    *pSum = nLocalSum;
    
    return ((F32)(*pSqSum) / (F32)nPatchSize) - ((F32)((*pSum) * (*pSum)) / (F32)(nPatchSize * nPatchSize));
}
#endif
#endif
