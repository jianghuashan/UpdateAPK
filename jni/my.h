#include <stdio.h>

#include "bzlib_private.h"
#include "bzlib.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <android/log.h>
#include <jni.h>


/*---------------------------------------------*/
Int32 BZ2_rNums[512] = {
   619, 720, 127, 481, 931, 816, 813, 233, 566, 247,
   985, 724, 205, 454, 863, 491, 741, 242, 949, 214,
   733, 859, 335, 708, 621, 574, 73, 654, 730, 472,
   419, 436, 278, 496, 867, 210, 399, 680, 480, 51,
   878, 465, 811, 169, 869, 675, 611, 697, 867, 561,
   862, 687, 507, 283, 482, 129, 807, 591, 733, 623,
   150, 238, 59, 379, 684, 877, 625, 169, 643, 105,
   170, 607, 520, 932, 727, 476, 693, 425, 174, 647,
   73, 122, 335, 530, 442, 853, 695, 249, 445, 515,
   909, 545, 703, 919, 874, 474, 882, 500, 594, 612,
   641, 801, 220, 162, 819, 984, 589, 513, 495, 799,
   161, 604, 958, 533, 221, 400, 386, 867, 600, 782,
   382, 596, 414, 171, 516, 375, 682, 485, 911, 276,
   98, 553, 163, 354, 666, 933, 424, 341, 533, 870,
   227, 730, 475, 186, 263, 647, 537, 686, 600, 224,
   469, 68, 770, 919, 190, 373, 294, 822, 808, 206,
   184, 943, 795, 384, 383, 461, 404, 758, 839, 887,
   715, 67, 618, 276, 204, 918, 873, 777, 604, 560,
   951, 160, 578, 722, 79, 804, 96, 409, 713, 940,
   652, 934, 970, 447, 318, 353, 859, 672, 112, 785,
   645, 863, 803, 350, 139, 93, 354, 99, 820, 908,
   609, 772, 154, 274, 580, 184, 79, 626, 630, 742,
   653, 282, 762, 623, 680, 81, 927, 626, 789, 125,
   411, 521, 938, 300, 821, 78, 343, 175, 128, 250,
   170, 774, 972, 275, 999, 639, 495, 78, 352, 126,
   857, 956, 358, 619, 580, 124, 737, 594, 701, 612,
   669, 112, 134, 694, 363, 992, 809, 743, 168, 974,
   944, 375, 748, 52, 600, 747, 642, 182, 862, 81,
   344, 805, 988, 739, 511, 655, 814, 334, 249, 515,
   897, 955, 664, 981, 649, 113, 974, 459, 893, 228,
   433, 837, 553, 268, 926, 240, 102, 654, 459, 51,
   686, 754, 806, 760, 493, 403, 415, 394, 687, 700,
   946, 670, 656, 610, 738, 392, 760, 799, 887, 653,
   978, 321, 576, 617, 626, 502, 894, 679, 243, 440,
   680, 879, 194, 572, 640, 724, 926, 56, 204, 700,
   707, 151, 457, 449, 797, 195, 791, 558, 945, 679,
   297, 59, 87, 824, 713, 663, 412, 693, 342, 606,
   134, 108, 571, 364, 631, 212, 174, 643, 304, 329,
   343, 97, 430, 751, 497, 314, 983, 374, 822, 928,
   140, 206, 73, 263, 980, 736, 876, 478, 430, 305,
   170, 514, 364, 692, 829, 82, 855, 953, 676, 246,
   369, 970, 294, 750, 807, 827, 150, 790, 288, 923,
   804, 378, 215, 828, 592, 281, 565, 555, 710, 82,
   896, 831, 547, 261, 524, 462, 293, 465, 502, 56,
   661, 821, 976, 991, 658, 869, 905, 758, 745, 193,
   768, 550, 608, 933, 378, 286, 215, 979, 792, 961,
   61, 688, 793, 644, 986, 403, 106, 366, 905, 644,
   372, 567, 466, 434, 645, 210, 389, 550, 919, 135,
   780, 773, 635, 389, 707, 100, 626, 958, 165, 504,
   920, 176, 193, 713, 857, 265, 203, 50, 668, 108,
   645, 990, 626, 197, 510, 357, 358, 850, 858, 364,
   936, 638
};



/*---------------------------------------------------*/
#define WEIGHTOF(zz0)  ((zz0) & 0xffffff00)
#define DEPTHOF(zz1)   ((zz1) & 0x000000ff)
#define MYMAX(zz2,zz3) ((zz2) > (zz3) ? (zz2) : (zz3))

#define ADDWEIGHTS(zw1,zw2)                           \
   (WEIGHTOF(zw1)+WEIGHTOF(zw2)) |                    \
   (1 + MYMAX(DEPTHOF(zw1),DEPTHOF(zw2)))

#define UPHEAP(z)                                     \
{                                                     \
   Int32 zz, tmp;                                     \
   zz = z; tmp = heap[zz];                            \
   while (weight[tmp] < weight[heap[zz >> 1]]) {      \
      heap[zz] = heap[zz >> 1];                       \
      zz >>= 1;                                       \
   }                                                  \
   heap[zz] = tmp;                                    \
}

#define DOWNHEAP(z)                                   \
{                                                     \
   Int32 zz, yy, tmp;                                 \
   zz = z; tmp = heap[zz];                            \
   while (True) {                                     \
      yy = zz << 1;                                   \
      if (yy > nHeap) break;                          \
      if (yy < nHeap &&                               \
          weight[heap[yy+1]] < weight[heap[yy]])      \
         yy++;                                        \
      if (weight[tmp] < weight[heap[yy]]) break;      \
      heap[zz] = heap[yy];                            \
      zz = yy;                                        \
   }                                                  \
   heap[zz] = tmp;                                    \
}


/*---------------------------------------------------*/
void BZ2_hbMakeCodeLengths ( UChar *len,
                             Int32 *freq,
                             Int32 alphaSize,
                             Int32 maxLen )
{
   /*--
      Nodes and heap entries run from 1.  Entry 0
      for both the heap and nodes is a sentinel.
   --*/
   Int32 nNodes, nHeap, n1, n2, i, j, k;
   Bool  tooLong;

   Int32 heap   [ BZ_MAX_ALPHA_SIZE + 2 ];
   Int32 weight [ BZ_MAX_ALPHA_SIZE * 2 ];
   Int32 parent [ BZ_MAX_ALPHA_SIZE * 2 ];

   for (i = 0; i < alphaSize; i++)
      weight[i+1] = (freq[i] == 0 ? 1 : freq[i]) << 8;

   while (True) {

      nNodes = alphaSize;
      nHeap = 0;

      heap[0] = 0;
      weight[0] = 0;
      parent[0] = -2;

      for (i = 1; i <= alphaSize; i++) {
         parent[i] = -1;
         nHeap++;
         heap[nHeap] = i;
         UPHEAP(nHeap);
      }

      AssertH( nHeap < (BZ_MAX_ALPHA_SIZE+2), 2001 );

      while (nHeap > 1) {
         n1 = heap[1]; heap[1] = heap[nHeap]; nHeap--; DOWNHEAP(1);
         n2 = heap[1]; heap[1] = heap[nHeap]; nHeap--; DOWNHEAP(1);
         nNodes++;
         parent[n1] = parent[n2] = nNodes;
         weight[nNodes] = ADDWEIGHTS(weight[n1], weight[n2]);
         parent[nNodes] = -1;
         nHeap++;
         heap[nHeap] = nNodes;
         UPHEAP(nHeap);
      }

      AssertH( nNodes < (BZ_MAX_ALPHA_SIZE * 2), 2002 );

      tooLong = False;
      for (i = 1; i <= alphaSize; i++) {
         j = 0;
         k = i;
         while (parent[k] >= 0) { k = parent[k]; j++; }
         len[i-1] = j;
         if (j > maxLen) tooLong = True;
      }

      if (! tooLong) break;

      /* 17 Oct 04: keep-going condition for the following loop used
         to be 'i < alphaSize', which missed the last element,
         theoretically leading to the possibility of the compressor
         looping.  However, this count-scaling step is only needed if
         one of the generated Huffman code words is longer than
         maxLen, which up to and including version 1.0.2 was 20 bits,
         which is extremely unlikely.  In version 1.0.3 maxLen was
         changed to 17 bits, which has minimal effect on compression
         ratio, but does mean this scaling step is used from time to
         time, enough to verify that it works.

         This means that bzip2-1.0.3 and later will only produce
         Huffman codes with a maximum length of 17 bits.  However, in
         order to preserve backwards compatibility with bitstreams
         produced by versions pre-1.0.3, the decompressor must still
         handle lengths of up to 20. */

      for (i = 1; i <= alphaSize; i++) {
         j = weight[i] >> 8;
         j = 1 + (j / 2);
         weight[i] = j << 8;
      }
   }
}


/*---------------------------------------------------*/
void BZ2_hbAssignCodes ( Int32 *code,
                         UChar *length,
                         Int32 minLen,
                         Int32 maxLen,
                         Int32 alphaSize )
{
   Int32 n, vec, i;

   vec = 0;
   for (n = minLen; n <= maxLen; n++) {
      for (i = 0; i < alphaSize; i++)
         if (length[i] == n) { code[i] = vec; vec++; };
      vec <<= 1;
   }
}


/*---------------------------------------------------*/
void BZ2_hbCreateDecodeTables ( Int32 *limit,
                                Int32 *base,
                                Int32 *perm,
                                UChar *length,
                                Int32 minLen,
                                Int32 maxLen,
                                Int32 alphaSize )
{
   Int32 pp, i, j, vec;

   pp = 0;
   for (i = minLen; i <= maxLen; i++)
      for (j = 0; j < alphaSize; j++)
         if (length[j] == i) { perm[pp] = j; pp++; };

   for (i = 0; i < BZ_MAX_CODE_LEN; i++) base[i] = 0;
   for (i = 0; i < alphaSize; i++) base[length[i]+1]++;

   for (i = 1; i < BZ_MAX_CODE_LEN; i++) base[i] += base[i-1];

   for (i = 0; i < BZ_MAX_CODE_LEN; i++) limit[i] = 0;
   vec = 0;

   for (i = minLen; i <= maxLen; i++) {
      vec += (base[i+1] - base[i]);
      limit[i] = vec-1;
      vec <<= 1;
   }
   for (i = minLen + 1; i <= maxLen; i++)
      base[i] = ((limit[i-1] + 1) << 1) - base[i];
}

/*---------------------------------------------------*/
static
void makeMaps_d ( DState* s )
{
   Int32 i;
   s->nInUse = 0;
   for (i = 0; i < 256; i++)
      if (s->inUse[i]) {
         s->seqToUnseq[s->nInUse] = i;
         s->nInUse++;
      }
}


/*---------------------------------------------------*/
#define RETURN(rrr)                               \
   { retVal = rrr; goto save_state_and_return; };

#define GET_BITS(lll,vvv,nnn)                     \
   case lll: s->state = lll;                      \
   while (True) {                                 \
      if (s->bsLive >= nnn) {                     \
         UInt32 v;                                \
         v = (s->bsBuff >>                        \
             (s->bsLive-nnn)) & ((1 << nnn)-1);   \
         s->bsLive -= nnn;                        \
         vvv = v;                                 \
         break;                                   \
      }                                           \
      if (s->strm->avail_in == 0) RETURN(BZ_OK);  \
      s->bsBuff                                   \
         = (s->bsBuff << 8) |                     \
           ((UInt32)                              \
              (*((UChar*)(s->strm->next_in))));   \
      s->bsLive += 8;                             \
      s->strm->next_in++;                         \
      s->strm->avail_in--;                        \
      s->strm->total_in_lo32++;                   \
      if (s->strm->total_in_lo32 == 0)            \
         s->strm->total_in_hi32++;                \
   }

#define GET_UCHAR(lll,uuu)                        \
   GET_BITS(lll,uuu,8)

#define GET_BIT(lll,uuu)                          \
   GET_BITS(lll,uuu,1)

/*---------------------------------------------------*/
#define GET_MTF_VAL(label1,label2,lval)           \
{                                                 \
   if (groupPos == 0) {                           \
      groupNo++;                                  \
      if (groupNo >= nSelectors)                  \
         RETURN(BZ_DATA_ERROR);                   \
      groupPos = BZ_G_SIZE;                       \
      gSel = s->selector[groupNo];                \
      gMinlen = s->minLens[gSel];                 \
      gLimit = &(s->limit[gSel][0]);              \
      gPerm = &(s->perm[gSel][0]);                \
      gBase = &(s->base[gSel][0]);                \
   }                                              \
   groupPos--;                                    \
   zn = gMinlen;                                  \
   GET_BITS(label1, zvec, zn);                    \
   while (1) {                                    \
      if (zn > 20 /* the longest code */)         \
         RETURN(BZ_DATA_ERROR);                   \
      if (zvec <= gLimit[zn]) break;              \
      zn++;                                       \
      GET_BIT(label2, zj);                        \
      zvec = (zvec << 1) | zj;                    \
   };                                             \
   if (zvec - gBase[zn] < 0                       \
       || zvec - gBase[zn] >= BZ_MAX_ALPHA_SIZE)  \
      RETURN(BZ_DATA_ERROR);                      \
   lval = gPerm[zvec - gBase[zn]];                \
}


/*---------------------------------------------------*/
Int32 BZ2_decompress ( DState* s )
{
   UChar      uc;
   Int32      retVal;
   Int32      minLen, maxLen;
   bz_stream* strm = s->strm;

   /* stuff that needs to be saved/restored */
   Int32  i;
   Int32  j;
   Int32  t;
   Int32  alphaSize;
   Int32  nGroups;
   Int32  nSelectors;
   Int32  EOB;
   Int32  groupNo;
   Int32  groupPos;
   Int32  nextSym;
   Int32  nblockMAX;
   Int32  nblock;
   Int32  es;
   Int32  N;
   Int32  curr;
   Int32  zt;
   Int32  zn;
   Int32  zvec;
   Int32  zj;
   Int32  gSel;
   Int32  gMinlen;
   Int32* gLimit;
   Int32* gBase;
   Int32* gPerm;

   if (s->state == BZ_X_MAGIC_1) {
      /*initialise the save area*/
      s->save_i           = 0;
      s->save_j           = 0;
      s->save_t           = 0;
      s->save_alphaSize   = 0;
      s->save_nGroups     = 0;
      s->save_nSelectors  = 0;
      s->save_EOB         = 0;
      s->save_groupNo     = 0;
      s->save_groupPos    = 0;
      s->save_nextSym     = 0;
      s->save_nblockMAX   = 0;
      s->save_nblock      = 0;
      s->save_es          = 0;
      s->save_N           = 0;
      s->save_curr        = 0;
      s->save_zt          = 0;
      s->save_zn          = 0;
      s->save_zvec        = 0;
      s->save_zj          = 0;
      s->save_gSel        = 0;
      s->save_gMinlen     = 0;
      s->save_gLimit      = NULL;
      s->save_gBase       = NULL;
      s->save_gPerm       = NULL;
   }

   /*restore from the save area*/
   i           = s->save_i;
   j           = s->save_j;
   t           = s->save_t;
   alphaSize   = s->save_alphaSize;
   nGroups     = s->save_nGroups;
   nSelectors  = s->save_nSelectors;
   EOB         = s->save_EOB;
   groupNo     = s->save_groupNo;
   groupPos    = s->save_groupPos;
   nextSym     = s->save_nextSym;
   nblockMAX   = s->save_nblockMAX;
   nblock      = s->save_nblock;
   es          = s->save_es;
   N           = s->save_N;
   curr        = s->save_curr;
   zt          = s->save_zt;
   zn          = s->save_zn;
   zvec        = s->save_zvec;
   zj          = s->save_zj;
   gSel        = s->save_gSel;
   gMinlen     = s->save_gMinlen;
   gLimit      = s->save_gLimit;
   gBase       = s->save_gBase;
   gPerm       = s->save_gPerm;

   retVal = BZ_OK;

   switch (s->state) {

      GET_UCHAR(BZ_X_MAGIC_1, uc);
      if (uc != BZ_HDR_B) RETURN(BZ_DATA_ERROR_MAGIC);

      GET_UCHAR(BZ_X_MAGIC_2, uc);
      if (uc != BZ_HDR_Z) RETURN(BZ_DATA_ERROR_MAGIC);

      GET_UCHAR(BZ_X_MAGIC_3, uc)
      if (uc != BZ_HDR_h) RETURN(BZ_DATA_ERROR_MAGIC);

      GET_BITS(BZ_X_MAGIC_4, s->blockSize100k, 8)
      if (s->blockSize100k < (BZ_HDR_0 + 1) ||
          s->blockSize100k > (BZ_HDR_0 + 9)) RETURN(BZ_DATA_ERROR_MAGIC);
      s->blockSize100k -= BZ_HDR_0;

      if (s->smallDecompress) {
         s->ll16 = BZALLOC( s->blockSize100k * 100000 * sizeof(UInt16) );
         s->ll4  = BZALLOC(
                      ((1 + s->blockSize100k * 100000) >> 1) * sizeof(UChar)
                   );
         if (s->ll16 == NULL || s->ll4 == NULL) RETURN(BZ_MEM_ERROR);
      } else {
         s->tt  = BZALLOC( s->blockSize100k * 100000 * sizeof(Int32) );
         if (s->tt == NULL) RETURN(BZ_MEM_ERROR);
      }

      GET_UCHAR(BZ_X_BLKHDR_1, uc);

      if (uc == 0x17) goto endhdr_2;
      if (uc != 0x31) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_BLKHDR_2, uc);
      if (uc != 0x41) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_BLKHDR_3, uc);
      if (uc != 0x59) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_BLKHDR_4, uc);
      if (uc != 0x26) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_BLKHDR_5, uc);
      if (uc != 0x53) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_BLKHDR_6, uc);
      if (uc != 0x59) RETURN(BZ_DATA_ERROR);

      s->currBlockNo++;
      if (s->verbosity >= 2)
         VPrintf1 ( "\n    [%d: huff+mtf ", s->currBlockNo );

      s->storedBlockCRC = 0;
      GET_UCHAR(BZ_X_BCRC_1, uc);
      s->storedBlockCRC = (s->storedBlockCRC << 8) | ((UInt32)uc);
      GET_UCHAR(BZ_X_BCRC_2, uc);
      s->storedBlockCRC = (s->storedBlockCRC << 8) | ((UInt32)uc);
      GET_UCHAR(BZ_X_BCRC_3, uc);
      s->storedBlockCRC = (s->storedBlockCRC << 8) | ((UInt32)uc);
      GET_UCHAR(BZ_X_BCRC_4, uc);
      s->storedBlockCRC = (s->storedBlockCRC << 8) | ((UInt32)uc);

      GET_BITS(BZ_X_RANDBIT, s->blockRandomised, 1);

      s->origPtr = 0;
      GET_UCHAR(BZ_X_ORIGPTR_1, uc);
      s->origPtr = (s->origPtr << 8) | ((Int32)uc);
      GET_UCHAR(BZ_X_ORIGPTR_2, uc);
      s->origPtr = (s->origPtr << 8) | ((Int32)uc);
      GET_UCHAR(BZ_X_ORIGPTR_3, uc);
      s->origPtr = (s->origPtr << 8) | ((Int32)uc);

      if (s->origPtr < 0)
         RETURN(BZ_DATA_ERROR);
      if (s->origPtr > 10 + 100000*s->blockSize100k)
         RETURN(BZ_DATA_ERROR);

      /*--- Receive the mapping table ---*/
      for (i = 0; i < 16; i++) {
         GET_BIT(BZ_X_MAPPING_1, uc);
         if (uc == 1)
            s->inUse16[i] = True; else
            s->inUse16[i] = False;
      }

      for (i = 0; i < 256; i++) s->inUse[i] = False;

      for (i = 0; i < 16; i++)
         if (s->inUse16[i])
            for (j = 0; j < 16; j++) {
               GET_BIT(BZ_X_MAPPING_2, uc);
               if (uc == 1) s->inUse[i * 16 + j] = True;
            }
      makeMaps_d ( s );
      if (s->nInUse == 0) RETURN(BZ_DATA_ERROR);
      alphaSize = s->nInUse+2;

      /*--- Now the selectors ---*/
      GET_BITS(BZ_X_SELECTOR_1, nGroups, 3);
      if (nGroups < 2 || nGroups > 6) RETURN(BZ_DATA_ERROR);
      GET_BITS(BZ_X_SELECTOR_2, nSelectors, 15);
      if (nSelectors < 1) RETURN(BZ_DATA_ERROR);
      for (i = 0; i < nSelectors; i++) {
         j = 0;
         while (True) {
            GET_BIT(BZ_X_SELECTOR_3, uc);
            if (uc == 0) break;
            j++;
            if (j >= nGroups) RETURN(BZ_DATA_ERROR);
         }
         s->selectorMtf[i] = j;
      }

      /*--- Undo the MTF values for the selectors. ---*/
      {
         UChar pos[BZ_N_GROUPS], tmp, v;
         for (v = 0; v < nGroups; v++) pos[v] = v;

         for (i = 0; i < nSelectors; i++) {
            v = s->selectorMtf[i];
            tmp = pos[v];
            while (v > 0) { pos[v] = pos[v-1]; v--; }
            pos[0] = tmp;
            s->selector[i] = tmp;
         }
      }

      /*--- Now the coding tables ---*/
      for (t = 0; t < nGroups; t++) {
         GET_BITS(BZ_X_CODING_1, curr, 5);
         for (i = 0; i < alphaSize; i++) {
            while (True) {
               if (curr < 1 || curr > 20) RETURN(BZ_DATA_ERROR);
               GET_BIT(BZ_X_CODING_2, uc);
               if (uc == 0) break;
               GET_BIT(BZ_X_CODING_3, uc);
               if (uc == 0) curr++; else curr--;
            }
            s->len[t][i] = curr;
         }
      }

      /*--- Create the Huffman decoding tables ---*/
      for (t = 0; t < nGroups; t++) {
         minLen = 32;
         maxLen = 0;
         for (i = 0; i < alphaSize; i++) {
            if (s->len[t][i] > maxLen) maxLen = s->len[t][i];
            if (s->len[t][i] < minLen) minLen = s->len[t][i];
         }
         BZ2_hbCreateDecodeTables (
            &(s->limit[t][0]),
            &(s->base[t][0]),
            &(s->perm[t][0]),
            &(s->len[t][0]),
            minLen, maxLen, alphaSize
         );
         s->minLens[t] = minLen;
      }

      /*--- Now the MTF values ---*/

      EOB      = s->nInUse+1;
      nblockMAX = 100000 * s->blockSize100k;
      groupNo  = -1;
      groupPos = 0;

      for (i = 0; i <= 255; i++) s->unzftab[i] = 0;

      /*-- MTF init --*/
      {
         Int32 ii, jj, kk;
         kk = MTFA_SIZE-1;
         for (ii = 256 / MTFL_SIZE - 1; ii >= 0; ii--) {
            for (jj = MTFL_SIZE-1; jj >= 0; jj--) {
               s->mtfa[kk] = (UChar)(ii * MTFL_SIZE + jj);
               kk--;
            }
            s->mtfbase[ii] = kk + 1;
         }
      }
      /*-- end MTF init --*/

      nblock = 0;
      GET_MTF_VAL(BZ_X_MTF_1, BZ_X_MTF_2, nextSym);

      while (True) {

         if (nextSym == EOB) break;

         if (nextSym == BZ_RUNA || nextSym == BZ_RUNB) {

            es = -1;
            N = 1;
            do {
               /* Check that N doesn't get too big, so that es doesn't
                  go negative.  The maximum value that can be
                  RUNA/RUNB encoded is equal to the block size (post
                  the initial RLE), viz, 900k, so bounding N at 2
                  million should guard against overflow without
                  rejecting any legitimate inputs. */
               if (N >= 2*1024*1024) RETURN(BZ_DATA_ERROR);
               if (nextSym == BZ_RUNA) es = es + (0+1) * N; else
               if (nextSym == BZ_RUNB) es = es + (1+1) * N;
               N = N * 2;
               GET_MTF_VAL(BZ_X_MTF_3, BZ_X_MTF_4, nextSym);
            }
               while (nextSym == BZ_RUNA || nextSym == BZ_RUNB);

            es++;
            uc = s->seqToUnseq[ s->mtfa[s->mtfbase[0]] ];
            s->unzftab[uc] += es;

            if (s->smallDecompress)
               while (es > 0) {
                  if (nblock >= nblockMAX) RETURN(BZ_DATA_ERROR);
                  s->ll16[nblock] = (UInt16)uc;
                  nblock++;
                  es--;
               }
            else
               while (es > 0) {
                  if (nblock >= nblockMAX) RETURN(BZ_DATA_ERROR);
                  s->tt[nblock] = (UInt32)uc;
                  nblock++;
                  es--;
               };

            continue;

         } else {

            if (nblock >= nblockMAX) RETURN(BZ_DATA_ERROR);

            /*-- uc = MTF ( nextSym-1 ) --*/
            {
               Int32 ii, jj, kk, pp, lno, off;
               UInt32 nn;
               nn = (UInt32)(nextSym - 1);

               if (nn < MTFL_SIZE) {
                  /* avoid general-case expense */
                  pp = s->mtfbase[0];
                  uc = s->mtfa[pp+nn];
                  while (nn > 3) {
                     Int32 z = pp+nn;
                     s->mtfa[(z)  ] = s->mtfa[(z)-1];
                     s->mtfa[(z)-1] = s->mtfa[(z)-2];
                     s->mtfa[(z)-2] = s->mtfa[(z)-3];
                     s->mtfa[(z)-3] = s->mtfa[(z)-4];
                     nn -= 4;
                  }
                  while (nn > 0) {
                     s->mtfa[(pp+nn)] = s->mtfa[(pp+nn)-1]; nn--;
                  };
                  s->mtfa[pp] = uc;
               } else {
                  /* general case */
                  lno = nn / MTFL_SIZE;
                  off = nn % MTFL_SIZE;
                  pp = s->mtfbase[lno] + off;
                  uc = s->mtfa[pp];
                  while (pp > s->mtfbase[lno]) {
                     s->mtfa[pp] = s->mtfa[pp-1]; pp--;
                  };
                  s->mtfbase[lno]++;
                  while (lno > 0) {
                     s->mtfbase[lno]--;
                     s->mtfa[s->mtfbase[lno]]
                        = s->mtfa[s->mtfbase[lno-1] + MTFL_SIZE - 1];
                     lno--;
                  }
                  s->mtfbase[0]--;
                  s->mtfa[s->mtfbase[0]] = uc;
                  if (s->mtfbase[0] == 0) {
                     kk = MTFA_SIZE-1;
                     for (ii = 256 / MTFL_SIZE-1; ii >= 0; ii--) {
                        for (jj = MTFL_SIZE-1; jj >= 0; jj--) {
                           s->mtfa[kk] = s->mtfa[s->mtfbase[ii] + jj];
                           kk--;
                        }
                        s->mtfbase[ii] = kk + 1;
                     }
                  }
               }
            }
            /*-- end uc = MTF ( nextSym-1 ) --*/

            s->unzftab[s->seqToUnseq[uc]]++;
            if (s->smallDecompress)
               s->ll16[nblock] = (UInt16)(s->seqToUnseq[uc]); else
               s->tt[nblock]   = (UInt32)(s->seqToUnseq[uc]);
            nblock++;

            GET_MTF_VAL(BZ_X_MTF_5, BZ_X_MTF_6, nextSym);
            continue;
         }
      }

      /* Now we know what nblock is, we can do a better sanity
         check on s->origPtr.
      */
      if (s->origPtr < 0 || s->origPtr >= nblock)
         RETURN(BZ_DATA_ERROR);

      /*-- Set up cftab to facilitate generation of T^(-1) --*/
      /* Check: unzftab entries in range. */
      for (i = 0; i <= 255; i++) {
         if (s->unzftab[i] < 0 || s->unzftab[i] > nblock)
            RETURN(BZ_DATA_ERROR);
      }
      /* Actually generate cftab. */
      s->cftab[0] = 0;
      for (i = 1; i <= 256; i++) s->cftab[i] = s->unzftab[i-1];
      for (i = 1; i <= 256; i++) s->cftab[i] += s->cftab[i-1];
      /* Check: cftab entries in range. */
      for (i = 0; i <= 256; i++) {
         if (s->cftab[i] < 0 || s->cftab[i] > nblock) {
            /* s->cftab[i] can legitimately be == nblock */
            RETURN(BZ_DATA_ERROR);
         }
      }
      /* Check: cftab entries non-descending. */
      for (i = 1; i <= 256; i++) {
         if (s->cftab[i-1] > s->cftab[i]) {
            RETURN(BZ_DATA_ERROR);
         }
      }

      s->state_out_len = 0;
      s->state_out_ch  = 0;
      BZ_INITIALISE_CRC ( s->calculatedBlockCRC );
      s->state = BZ_X_OUTPUT;
      if (s->verbosity >= 2) VPrintf0 ( "rt+rld" );

      if (s->smallDecompress) {

         /*-- Make a copy of cftab, used in generation of T --*/
         for (i = 0; i <= 256; i++) s->cftabCopy[i] = s->cftab[i];

         /*-- compute the T vector --*/
         for (i = 0; i < nblock; i++) {
            uc = (UChar)(s->ll16[i]);
            SET_LL(i, s->cftabCopy[uc]);
            s->cftabCopy[uc]++;
         }

         /*-- Compute T^(-1) by pointer reversal on T --*/
         i = s->origPtr;
         j = GET_LL(i);
         do {
            Int32 tmp = GET_LL(j);
            SET_LL(j, i);
            i = j;
            j = tmp;
         }
            while (i != s->origPtr);

         s->tPos = s->origPtr;
         s->nblock_used = 0;
         if (s->blockRandomised) {
            BZ_RAND_INIT_MASK;
            BZ_GET_SMALL(s->k0); s->nblock_used++;
            BZ_RAND_UPD_MASK; s->k0 ^= BZ_RAND_MASK;
         } else {
            BZ_GET_SMALL(s->k0); s->nblock_used++;
         }

      } else {

         /*-- compute the T^(-1) vector --*/
         for (i = 0; i < nblock; i++) {
            uc = (UChar)(s->tt[i] & 0xff);
            s->tt[s->cftab[uc]] |= (i << 8);
            s->cftab[uc]++;
         }

         s->tPos = s->tt[s->origPtr] >> 8;
         s->nblock_used = 0;
         if (s->blockRandomised) {
            BZ_RAND_INIT_MASK;
            BZ_GET_FAST(s->k0); s->nblock_used++;
            BZ_RAND_UPD_MASK; s->k0 ^= BZ_RAND_MASK;
         } else {
            BZ_GET_FAST(s->k0); s->nblock_used++;
         }

      }

      RETURN(BZ_OK);



    endhdr_2:

      GET_UCHAR(BZ_X_ENDHDR_2, uc);
      if (uc != 0x72) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_ENDHDR_3, uc);
      if (uc != 0x45) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_ENDHDR_4, uc);
      if (uc != 0x38) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_ENDHDR_5, uc);
      if (uc != 0x50) RETURN(BZ_DATA_ERROR);
      GET_UCHAR(BZ_X_ENDHDR_6, uc);
      if (uc != 0x90) RETURN(BZ_DATA_ERROR);

      s->storedCombinedCRC = 0;
      GET_UCHAR(BZ_X_CCRC_1, uc);
      s->storedCombinedCRC = (s->storedCombinedCRC << 8) | ((UInt32)uc);
      GET_UCHAR(BZ_X_CCRC_2, uc);
      s->storedCombinedCRC = (s->storedCombinedCRC << 8) | ((UInt32)uc);
      GET_UCHAR(BZ_X_CCRC_3, uc);
      s->storedCombinedCRC = (s->storedCombinedCRC << 8) | ((UInt32)uc);
      GET_UCHAR(BZ_X_CCRC_4, uc);
      s->storedCombinedCRC = (s->storedCombinedCRC << 8) | ((UInt32)uc);

      s->state = BZ_X_IDLE;
      RETURN(BZ_STREAM_END);

      default: AssertH ( False, 4001 );
   }

   AssertH ( False, 4002 );

   save_state_and_return:

   s->save_i           = i;
   s->save_j           = j;
   s->save_t           = t;
   s->save_alphaSize   = alphaSize;
   s->save_nGroups     = nGroups;
   s->save_nSelectors  = nSelectors;
   s->save_EOB         = EOB;
   s->save_groupNo     = groupNo;
   s->save_groupPos    = groupPos;
   s->save_nextSym     = nextSym;
   s->save_nblockMAX   = nblockMAX;
   s->save_nblock      = nblock;
   s->save_es          = es;
   s->save_N           = N;
   s->save_curr        = curr;
   s->save_zt          = zt;
   s->save_zn          = zn;
   s->save_zvec        = zvec;
   s->save_zj          = zj;
   s->save_gSel        = gSel;
   s->save_gMinlen     = gMinlen;
   s->save_gLimit      = gLimit;
   s->save_gBase       = gBase;
   s->save_gPerm       = gPerm;

   return retVal;
}


/*--
  I think this is an implementation of the AUTODIN-II,
  Ethernet & FDDI 32-bit CRC standard.  Vaguely derived
  from code by Rob Warnock, in Section 51 of the
  comp.compression FAQ.
--*/

UInt32 BZ2_crc32Table[256] = {

   /*-- Ugly, innit? --*/

   0x00000000L, 0x04c11db7L, 0x09823b6eL, 0x0d4326d9L,
   0x130476dcL, 0x17c56b6bL, 0x1a864db2L, 0x1e475005L,
   0x2608edb8L, 0x22c9f00fL, 0x2f8ad6d6L, 0x2b4bcb61L,
   0x350c9b64L, 0x31cd86d3L, 0x3c8ea00aL, 0x384fbdbdL,
   0x4c11db70L, 0x48d0c6c7L, 0x4593e01eL, 0x4152fda9L,
   0x5f15adacL, 0x5bd4b01bL, 0x569796c2L, 0x52568b75L,
   0x6a1936c8L, 0x6ed82b7fL, 0x639b0da6L, 0x675a1011L,
   0x791d4014L, 0x7ddc5da3L, 0x709f7b7aL, 0x745e66cdL,
   0x9823b6e0L, 0x9ce2ab57L, 0x91a18d8eL, 0x95609039L,
   0x8b27c03cL, 0x8fe6dd8bL, 0x82a5fb52L, 0x8664e6e5L,
   0xbe2b5b58L, 0xbaea46efL, 0xb7a96036L, 0xb3687d81L,
   0xad2f2d84L, 0xa9ee3033L, 0xa4ad16eaL, 0xa06c0b5dL,
   0xd4326d90L, 0xd0f37027L, 0xddb056feL, 0xd9714b49L,
   0xc7361b4cL, 0xc3f706fbL, 0xceb42022L, 0xca753d95L,
   0xf23a8028L, 0xf6fb9d9fL, 0xfbb8bb46L, 0xff79a6f1L,
   0xe13ef6f4L, 0xe5ffeb43L, 0xe8bccd9aL, 0xec7dd02dL,
   0x34867077L, 0x30476dc0L, 0x3d044b19L, 0x39c556aeL,
   0x278206abL, 0x23431b1cL, 0x2e003dc5L, 0x2ac12072L,
   0x128e9dcfL, 0x164f8078L, 0x1b0ca6a1L, 0x1fcdbb16L,
   0x018aeb13L, 0x054bf6a4L, 0x0808d07dL, 0x0cc9cdcaL,
   0x7897ab07L, 0x7c56b6b0L, 0x71159069L, 0x75d48ddeL,
   0x6b93dddbL, 0x6f52c06cL, 0x6211e6b5L, 0x66d0fb02L,
   0x5e9f46bfL, 0x5a5e5b08L, 0x571d7dd1L, 0x53dc6066L,
   0x4d9b3063L, 0x495a2dd4L, 0x44190b0dL, 0x40d816baL,
   0xaca5c697L, 0xa864db20L, 0xa527fdf9L, 0xa1e6e04eL,
   0xbfa1b04bL, 0xbb60adfcL, 0xb6238b25L, 0xb2e29692L,
   0x8aad2b2fL, 0x8e6c3698L, 0x832f1041L, 0x87ee0df6L,
   0x99a95df3L, 0x9d684044L, 0x902b669dL, 0x94ea7b2aL,
   0xe0b41de7L, 0xe4750050L, 0xe9362689L, 0xedf73b3eL,
   0xf3b06b3bL, 0xf771768cL, 0xfa325055L, 0xfef34de2L,
   0xc6bcf05fL, 0xc27dede8L, 0xcf3ecb31L, 0xcbffd686L,
   0xd5b88683L, 0xd1799b34L, 0xdc3abdedL, 0xd8fba05aL,
   0x690ce0eeL, 0x6dcdfd59L, 0x608edb80L, 0x644fc637L,
   0x7a089632L, 0x7ec98b85L, 0x738aad5cL, 0x774bb0ebL,
   0x4f040d56L, 0x4bc510e1L, 0x46863638L, 0x42472b8fL,
   0x5c007b8aL, 0x58c1663dL, 0x558240e4L, 0x51435d53L,
   0x251d3b9eL, 0x21dc2629L, 0x2c9f00f0L, 0x285e1d47L,
   0x36194d42L, 0x32d850f5L, 0x3f9b762cL, 0x3b5a6b9bL,
   0x0315d626L, 0x07d4cb91L, 0x0a97ed48L, 0x0e56f0ffL,
   0x1011a0faL, 0x14d0bd4dL, 0x19939b94L, 0x1d528623L,
   0xf12f560eL, 0xf5ee4bb9L, 0xf8ad6d60L, 0xfc6c70d7L,
   0xe22b20d2L, 0xe6ea3d65L, 0xeba91bbcL, 0xef68060bL,
   0xd727bbb6L, 0xd3e6a601L, 0xdea580d8L, 0xda649d6fL,
   0xc423cd6aL, 0xc0e2d0ddL, 0xcda1f604L, 0xc960ebb3L,
   0xbd3e8d7eL, 0xb9ff90c9L, 0xb4bcb610L, 0xb07daba7L,
   0xae3afba2L, 0xaafbe615L, 0xa7b8c0ccL, 0xa379dd7bL,
   0x9b3660c6L, 0x9ff77d71L, 0x92b45ba8L, 0x9675461fL,
   0x8832161aL, 0x8cf30badL, 0x81b02d74L, 0x857130c3L,
   0x5d8a9099L, 0x594b8d2eL, 0x5408abf7L, 0x50c9b640L,
   0x4e8ee645L, 0x4a4ffbf2L, 0x470cdd2bL, 0x43cdc09cL,
   0x7b827d21L, 0x7f436096L, 0x7200464fL, 0x76c15bf8L,
   0x68860bfdL, 0x6c47164aL, 0x61043093L, 0x65c52d24L,
   0x119b4be9L, 0x155a565eL, 0x18197087L, 0x1cd86d30L,
   0x029f3d35L, 0x065e2082L, 0x0b1d065bL, 0x0fdc1becL,
   0x3793a651L, 0x3352bbe6L, 0x3e119d3fL, 0x3ad08088L,
   0x2497d08dL, 0x2056cd3aL, 0x2d15ebe3L, 0x29d4f654L,
   0xc5a92679L, 0xc1683bceL, 0xcc2b1d17L, 0xc8ea00a0L,
   0xd6ad50a5L, 0xd26c4d12L, 0xdf2f6bcbL, 0xdbee767cL,
   0xe3a1cbc1L, 0xe760d676L, 0xea23f0afL, 0xeee2ed18L,
   0xf0a5bd1dL, 0xf464a0aaL, 0xf9278673L, 0xfde69bc4L,
   0x89b8fd09L, 0x8d79e0beL, 0x803ac667L, 0x84fbdbd0L,
   0x9abc8bd5L, 0x9e7d9662L, 0x933eb0bbL, 0x97ffad0cL,
   0xafb010b1L, 0xab710d06L, 0xa6322bdfL, 0xa2f33668L,
   0xbcb4666dL, 0xb8757bdaL, 0xb5365d03L, 0xb1f740b4L
};

/*---------------------------------------------------*/
void BZ2_bsInitWrite ( EState* s )
{
   s->bsLive = 0;
   s->bsBuff = 0;
}


/*---------------------------------------------------*/
static
void bsFinishWrite ( EState* s )
{
   while (s->bsLive > 0) {
      s->zbits[s->numZ] = (UChar)(s->bsBuff >> 24);
      s->numZ++;
      s->bsBuff <<= 8;
      s->bsLive -= 8;
   }
}


/*---------------------------------------------------*/
#define bsNEEDW(nz)                           \
{                                             \
   while (s->bsLive >= 8) {                   \
      s->zbits[s->numZ]                       \
         = (UChar)(s->bsBuff >> 24);          \
      s->numZ++;                              \
      s->bsBuff <<= 8;                        \
      s->bsLive -= 8;                         \
   }                                          \
}


/*---------------------------------------------------*/
static
__inline__
void bsW ( EState* s, Int32 n, UInt32 v )
{
   bsNEEDW ( n );
   s->bsBuff |= (v << (32 - s->bsLive - n));
   s->bsLive += n;
}


/*---------------------------------------------------*/
static
void bsPutUInt32 ( EState* s, UInt32 u )
{
   bsW ( s, 8, (u >> 24) & 0xffL );
   bsW ( s, 8, (u >> 16) & 0xffL );
   bsW ( s, 8, (u >>  8) & 0xffL );
   bsW ( s, 8,  u        & 0xffL );
}


/*---------------------------------------------------*/
static
void bsPutUChar ( EState* s, UChar c )
{
   bsW( s, 8, (UInt32)c );
}


/*---------------------------------------------------*/
/*--- The back end proper                         ---*/
/*---------------------------------------------------*/

/*---------------------------------------------------*/
static
void makeMaps_e ( EState* s )
{
   Int32 i;
   s->nInUse = 0;
   for (i = 0; i < 256; i++)
      if (s->inUse[i]) {
         s->unseqToSeq[i] = s->nInUse;
         s->nInUse++;
      }
}


/*---------------------------------------------------*/
static
void generateMTFValues ( EState* s )
{
   UChar   yy[256];
   Int32   i, j;
   Int32   zPend;
   Int32   wr;
   Int32   EOB;

   /*
      After sorting (eg, here),
         s->arr1 [ 0 .. s->nblock-1 ] holds sorted order,
         and
         ((UChar*)s->arr2) [ 0 .. s->nblock-1 ]
         holds the original block data.

      The first thing to do is generate the MTF values,
      and put them in
         ((UInt16*)s->arr1) [ 0 .. s->nblock-1 ].
      Because there are strictly fewer or equal MTF values
      than block values, ptr values in this area are overwritten
      with MTF values only when they are no longer needed.

      The final compressed bitstream is generated into the
      area starting at
         (UChar*) (&((UChar*)s->arr2)[s->nblock])

      These storage aliases are set up in bzCompressInit(),
      except for the last one, which is arranged in
      compressBlock().
   */
   UInt32* ptr   = s->ptr;
   UChar* block  = s->block;
   UInt16* mtfv  = s->mtfv;

   makeMaps_e ( s );
   EOB = s->nInUse+1;

   for (i = 0; i <= EOB; i++) s->mtfFreq[i] = 0;

   wr = 0;
   zPend = 0;
   for (i = 0; i < s->nInUse; i++) yy[i] = (UChar) i;

   for (i = 0; i < s->nblock; i++) {
      UChar ll_i;
      AssertD ( wr <= i, "generateMTFValues(1)" );
      j = ptr[i]-1; if (j < 0) j += s->nblock;
      ll_i = s->unseqToSeq[block[j]];
      AssertD ( ll_i < s->nInUse, "generateMTFValues(2a)" );

      if (yy[0] == ll_i) {
         zPend++;
      } else {

         if (zPend > 0) {
            zPend--;
            while (True) {
               if (zPend & 1) {
                  mtfv[wr] = BZ_RUNB; wr++;
                  s->mtfFreq[BZ_RUNB]++;
               } else {
                  mtfv[wr] = BZ_RUNA; wr++;
                  s->mtfFreq[BZ_RUNA]++;
               }
               if (zPend < 2) break;
               zPend = (zPend - 2) / 2;
            };
            zPend = 0;
         }
         {
            register UChar  rtmp;
            register UChar* ryy_j;
            register UChar  rll_i;
            rtmp  = yy[1];
            yy[1] = yy[0];
            ryy_j = &(yy[1]);
            rll_i = ll_i;
            while ( rll_i != rtmp ) {
               register UChar rtmp2;
               ryy_j++;
               rtmp2  = rtmp;
               rtmp   = *ryy_j;
               *ryy_j = rtmp2;
            };
            yy[0] = rtmp;
            j = ryy_j - &(yy[0]);
            mtfv[wr] = j+1; wr++; s->mtfFreq[j+1]++;
         }

      }
   }

   if (zPend > 0) {
      zPend--;
      while (True) {
         if (zPend & 1) {
            mtfv[wr] = BZ_RUNB; wr++;
            s->mtfFreq[BZ_RUNB]++;
         } else {
            mtfv[wr] = BZ_RUNA; wr++;
            s->mtfFreq[BZ_RUNA]++;
         }
         if (zPend < 2) break;
         zPend = (zPend - 2) / 2;
      };
      zPend = 0;
   }

   mtfv[wr] = EOB; wr++; s->mtfFreq[EOB]++;

   s->nMTF = wr;
}


/*---------------------------------------------------*/
#define BZ_LESSER_ICOST  0
#define BZ_GREATER_ICOST 15

static
void sendMTFValues ( EState* s )
{
   Int32 v, t, i, j, gs, ge, totc, bt, bc, iter;
   Int32 nSelectors, alphaSize, minLen, maxLen, selCtr;
   Int32 nGroups, nBytes;

   /*--
   UChar  len [BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
   is a global since the decoder also needs it.

   Int32  code[BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
   Int32  rfreq[BZ_N_GROUPS][BZ_MAX_ALPHA_SIZE];
   are also globals only used in this proc.
   Made global to keep stack frame size small.
   --*/


   UInt16 cost[BZ_N_GROUPS];
   Int32  fave[BZ_N_GROUPS];

   UInt16* mtfv = s->mtfv;

   if (s->verbosity >= 3)
      VPrintf3( "      %d in block, %d after MTF & 1-2 coding, "
                "%d+2 syms in use\n",
                s->nblock, s->nMTF, s->nInUse );

   alphaSize = s->nInUse+2;
   for (t = 0; t < BZ_N_GROUPS; t++)
      for (v = 0; v < alphaSize; v++)
         s->len[t][v] = BZ_GREATER_ICOST;

   /*--- Decide how many coding tables to use ---*/
   AssertH ( s->nMTF > 0, 3001 );
   if (s->nMTF < 200)  nGroups = 2; else
   if (s->nMTF < 600)  nGroups = 3; else
   if (s->nMTF < 1200) nGroups = 4; else
   if (s->nMTF < 2400) nGroups = 5; else
                       nGroups = 6;

   /*--- Generate an initial set of coding tables ---*/
   {
      Int32 nPart, remF, tFreq, aFreq;

      nPart = nGroups;
      remF  = s->nMTF;
      gs = 0;
      while (nPart > 0) {
         tFreq = remF / nPart;
         ge = gs-1;
         aFreq = 0;
         while (aFreq < tFreq && ge < alphaSize-1) {
            ge++;
            aFreq += s->mtfFreq[ge];
         }

         if (ge > gs
             && nPart != nGroups && nPart != 1
             && ((nGroups-nPart) % 2 == 1)) {
            aFreq -= s->mtfFreq[ge];
            ge--;
         }

         if (s->verbosity >= 3)
            VPrintf5( "      initial group %d, [%d .. %d], "
                      "has %d syms (%4.1f%%)\n",
                      nPart, gs, ge, aFreq,
                      (100.0 * (float)aFreq) / (float)(s->nMTF) );

         for (v = 0; v < alphaSize; v++)
            if (v >= gs && v <= ge)
               s->len[nPart-1][v] = BZ_LESSER_ICOST; else
               s->len[nPart-1][v] = BZ_GREATER_ICOST;

         nPart--;
         gs = ge+1;
         remF -= aFreq;
      }
   }

   /*---
      Iterate up to BZ_N_ITERS times to improve the tables.
   ---*/
   for (iter = 0; iter < BZ_N_ITERS; iter++) {

      for (t = 0; t < nGroups; t++) fave[t] = 0;

      for (t = 0; t < nGroups; t++)
         for (v = 0; v < alphaSize; v++)
            s->rfreq[t][v] = 0;

      /*---
        Set up an auxiliary length table which is used to fast-track
	the common case (nGroups == 6).
      ---*/
      if (nGroups == 6) {
         for (v = 0; v < alphaSize; v++) {
            s->len_pack[v][0] = (s->len[1][v] << 16) | s->len[0][v];
            s->len_pack[v][1] = (s->len[3][v] << 16) | s->len[2][v];
            s->len_pack[v][2] = (s->len[5][v] << 16) | s->len[4][v];
	 }
      }

      nSelectors = 0;
      totc = 0;
      gs = 0;
      while (True) {

         /*--- Set group start & end marks. --*/
         if (gs >= s->nMTF) break;
         ge = gs + BZ_G_SIZE - 1;
         if (ge >= s->nMTF) ge = s->nMTF-1;

         /*--
            Calculate the cost of this group as coded
            by each of the coding tables.
         --*/
         for (t = 0; t < nGroups; t++) cost[t] = 0;

         if (nGroups == 6 && 50 == ge-gs+1) {
            /*--- fast track the common case ---*/
            register UInt32 cost01, cost23, cost45;
            register UInt16 icv;
            cost01 = cost23 = cost45 = 0;

#           define BZ_ITER(nn)                \
               icv = mtfv[gs+(nn)];           \
               cost01 += s->len_pack[icv][0]; \
               cost23 += s->len_pack[icv][1]; \
               cost45 += s->len_pack[icv][2]; \

            BZ_ITER(0);  BZ_ITER(1);  BZ_ITER(2);  BZ_ITER(3);  BZ_ITER(4);
            BZ_ITER(5);  BZ_ITER(6);  BZ_ITER(7);  BZ_ITER(8);  BZ_ITER(9);
            BZ_ITER(10); BZ_ITER(11); BZ_ITER(12); BZ_ITER(13); BZ_ITER(14);
            BZ_ITER(15); BZ_ITER(16); BZ_ITER(17); BZ_ITER(18); BZ_ITER(19);
            BZ_ITER(20); BZ_ITER(21); BZ_ITER(22); BZ_ITER(23); BZ_ITER(24);
            BZ_ITER(25); BZ_ITER(26); BZ_ITER(27); BZ_ITER(28); BZ_ITER(29);
            BZ_ITER(30); BZ_ITER(31); BZ_ITER(32); BZ_ITER(33); BZ_ITER(34);
            BZ_ITER(35); BZ_ITER(36); BZ_ITER(37); BZ_ITER(38); BZ_ITER(39);
            BZ_ITER(40); BZ_ITER(41); BZ_ITER(42); BZ_ITER(43); BZ_ITER(44);
            BZ_ITER(45); BZ_ITER(46); BZ_ITER(47); BZ_ITER(48); BZ_ITER(49);

#           undef BZ_ITER

            cost[0] = cost01 & 0xffff; cost[1] = cost01 >> 16;
            cost[2] = cost23 & 0xffff; cost[3] = cost23 >> 16;
            cost[4] = cost45 & 0xffff; cost[5] = cost45 >> 16;

         } else {
	    /*--- slow version which correctly handles all situations ---*/
            for (i = gs; i <= ge; i++) {
               UInt16 icv = mtfv[i];
               for (t = 0; t < nGroups; t++) cost[t] += s->len[t][icv];
            }
         }

         /*--
            Find the coding table which is best for this group,
            and record its identity in the selector table.
         --*/
         bc = 999999999; bt = -1;
         for (t = 0; t < nGroups; t++)
            if (cost[t] < bc) { bc = cost[t]; bt = t; };
         totc += bc;
         fave[bt]++;
         s->selector[nSelectors] = bt;
         nSelectors++;

         /*--
            Increment the symbol frequencies for the selected table.
          --*/
         if (nGroups == 6 && 50 == ge-gs+1) {
            /*--- fast track the common case ---*/

#           define BZ_ITUR(nn) s->rfreq[bt][ mtfv[gs+(nn)] ]++

            BZ_ITUR(0);  BZ_ITUR(1);  BZ_ITUR(2);  BZ_ITUR(3);  BZ_ITUR(4);
            BZ_ITUR(5);  BZ_ITUR(6);  BZ_ITUR(7);  BZ_ITUR(8);  BZ_ITUR(9);
            BZ_ITUR(10); BZ_ITUR(11); BZ_ITUR(12); BZ_ITUR(13); BZ_ITUR(14);
            BZ_ITUR(15); BZ_ITUR(16); BZ_ITUR(17); BZ_ITUR(18); BZ_ITUR(19);
            BZ_ITUR(20); BZ_ITUR(21); BZ_ITUR(22); BZ_ITUR(23); BZ_ITUR(24);
            BZ_ITUR(25); BZ_ITUR(26); BZ_ITUR(27); BZ_ITUR(28); BZ_ITUR(29);
            BZ_ITUR(30); BZ_ITUR(31); BZ_ITUR(32); BZ_ITUR(33); BZ_ITUR(34);
            BZ_ITUR(35); BZ_ITUR(36); BZ_ITUR(37); BZ_ITUR(38); BZ_ITUR(39);
            BZ_ITUR(40); BZ_ITUR(41); BZ_ITUR(42); BZ_ITUR(43); BZ_ITUR(44);
            BZ_ITUR(45); BZ_ITUR(46); BZ_ITUR(47); BZ_ITUR(48); BZ_ITUR(49);

#           undef BZ_ITUR

         } else {
	    /*--- slow version which correctly handles all situations ---*/
            for (i = gs; i <= ge; i++)
               s->rfreq[bt][ mtfv[i] ]++;
         }

         gs = ge+1;
      }
      if (s->verbosity >= 3) {
         VPrintf2 ( "      pass %d: size is %d, grp uses are ",
                   iter+1, totc/8 );
         for (t = 0; t < nGroups; t++)
            VPrintf1 ( "%d ", fave[t] );
         VPrintf0 ( "\n" );
      }

      /*--
        Recompute the tables based on the accumulated frequencies.
      --*/
      /* maxLen was changed from 20 to 17 in bzip2-1.0.3.  See
         comment in huffman.c for details. */
      for (t = 0; t < nGroups; t++)
         BZ2_hbMakeCodeLengths ( &(s->len[t][0]), &(s->rfreq[t][0]),
                                 alphaSize, 17 /*20*/ );
   }


   AssertH( nGroups < 8, 3002 );
   AssertH( nSelectors < 32768 &&
            nSelectors <= (2 + (900000 / BZ_G_SIZE)),
            3003 );


   /*--- Compute MTF values for the selectors. ---*/
   {
      UChar pos[BZ_N_GROUPS], ll_i, tmp2, tmp;
      for (i = 0; i < nGroups; i++) pos[i] = i;
      for (i = 0; i < nSelectors; i++) {
         ll_i = s->selector[i];
         j = 0;
         tmp = pos[j];
         while ( ll_i != tmp ) {
            j++;
            tmp2 = tmp;
            tmp = pos[j];
            pos[j] = tmp2;
         };
         pos[0] = tmp;
         s->selectorMtf[i] = j;
      }
   };

   /*--- Assign actual codes for the tables. --*/
   for (t = 0; t < nGroups; t++) {
      minLen = 32;
      maxLen = 0;
      for (i = 0; i < alphaSize; i++) {
         if (s->len[t][i] > maxLen) maxLen = s->len[t][i];
         if (s->len[t][i] < minLen) minLen = s->len[t][i];
      }
      AssertH ( !(maxLen > 17 /*20*/ ), 3004 );
      AssertH ( !(minLen < 1),  3005 );
      BZ2_hbAssignCodes ( &(s->code[t][0]), &(s->len[t][0]),
                          minLen, maxLen, alphaSize );
   }

   /*--- Transmit the mapping table. ---*/
   {
      Bool inUse16[16];
      for (i = 0; i < 16; i++) {
          inUse16[i] = False;
          for (j = 0; j < 16; j++)
             if (s->inUse[i * 16 + j]) inUse16[i] = True;
      }

      nBytes = s->numZ;
      for (i = 0; i < 16; i++)
         if (inUse16[i]) bsW(s,1,1); else bsW(s,1,0);

      for (i = 0; i < 16; i++)
         if (inUse16[i])
            for (j = 0; j < 16; j++) {
               if (s->inUse[i * 16 + j]) bsW(s,1,1); else bsW(s,1,0);
            }

      if (s->verbosity >= 3)
         VPrintf1( "      bytes: mapping %d, ", s->numZ-nBytes );
   }

   /*--- Now the selectors. ---*/
   nBytes = s->numZ;
   bsW ( s, 3, nGroups );
   bsW ( s, 15, nSelectors );
   for (i = 0; i < nSelectors; i++) {
      for (j = 0; j < s->selectorMtf[i]; j++) bsW(s,1,1);
      bsW(s,1,0);
   }
   if (s->verbosity >= 3)
      VPrintf1( "selectors %d, ", s->numZ-nBytes );

   /*--- Now the coding tables. ---*/
   nBytes = s->numZ;

   for (t = 0; t < nGroups; t++) {
      Int32 curr = s->len[t][0];
      bsW ( s, 5, curr );
      for (i = 0; i < alphaSize; i++) {
         while (curr < s->len[t][i]) { bsW(s,2,2); curr++; /* 10 */ };
         while (curr > s->len[t][i]) { bsW(s,2,3); curr--; /* 11 */ };
         bsW ( s, 1, 0 );
      }
   }

   if (s->verbosity >= 3)
      VPrintf1 ( "code lengths %d, ", s->numZ-nBytes );

   /*--- And finally, the block data proper ---*/
   nBytes = s->numZ;
   selCtr = 0;
   gs = 0;
   while (True) {
      if (gs >= s->nMTF) break;
      ge = gs + BZ_G_SIZE - 1;
      if (ge >= s->nMTF) ge = s->nMTF-1;
      AssertH ( s->selector[selCtr] < nGroups, 3006 );

      if (nGroups == 6 && 50 == ge-gs+1) {
            /*--- fast track the common case ---*/
            UInt16 mtfv_i;
            UChar* s_len_sel_selCtr
               = &(s->len[s->selector[selCtr]][0]);
            Int32* s_code_sel_selCtr
               = &(s->code[s->selector[selCtr]][0]);

#           define BZ_ITAH(nn)                      \
               mtfv_i = mtfv[gs+(nn)];              \
               bsW ( s,                             \
                     s_len_sel_selCtr[mtfv_i],      \
                     s_code_sel_selCtr[mtfv_i] )

            BZ_ITAH(0);  BZ_ITAH(1);  BZ_ITAH(2);  BZ_ITAH(3);  BZ_ITAH(4);
            BZ_ITAH(5);  BZ_ITAH(6);  BZ_ITAH(7);  BZ_ITAH(8);  BZ_ITAH(9);
            BZ_ITAH(10); BZ_ITAH(11); BZ_ITAH(12); BZ_ITAH(13); BZ_ITAH(14);
            BZ_ITAH(15); BZ_ITAH(16); BZ_ITAH(17); BZ_ITAH(18); BZ_ITAH(19);
            BZ_ITAH(20); BZ_ITAH(21); BZ_ITAH(22); BZ_ITAH(23); BZ_ITAH(24);
            BZ_ITAH(25); BZ_ITAH(26); BZ_ITAH(27); BZ_ITAH(28); BZ_ITAH(29);
            BZ_ITAH(30); BZ_ITAH(31); BZ_ITAH(32); BZ_ITAH(33); BZ_ITAH(34);
            BZ_ITAH(35); BZ_ITAH(36); BZ_ITAH(37); BZ_ITAH(38); BZ_ITAH(39);
            BZ_ITAH(40); BZ_ITAH(41); BZ_ITAH(42); BZ_ITAH(43); BZ_ITAH(44);
            BZ_ITAH(45); BZ_ITAH(46); BZ_ITAH(47); BZ_ITAH(48); BZ_ITAH(49);

#           undef BZ_ITAH

      } else {
	 /*--- slow version which correctly handles all situations ---*/
         for (i = gs; i <= ge; i++) {
            bsW ( s,
                  s->len  [s->selector[selCtr]] [mtfv[i]],
                  s->code [s->selector[selCtr]] [mtfv[i]] );
         }
      }


      gs = ge+1;
      selCtr++;
   }
   AssertH( selCtr == nSelectors, 3007 );

   if (s->verbosity >= 3)
      VPrintf1( "codes %d\n", s->numZ-nBytes );
}


/*---------------------------------------------------*/
void BZ2_compressBlock ( EState* s, Bool is_last_block )
{
   if (s->nblock > 0) {

      BZ_FINALISE_CRC ( s->blockCRC );
      s->combinedCRC = (s->combinedCRC << 1) | (s->combinedCRC >> 31);
      s->combinedCRC ^= s->blockCRC;
      if (s->blockNo > 1) s->numZ = 0;

      if (s->verbosity >= 2)
         VPrintf4( "    block %d: crc = 0x%08x, "
                   "combined CRC = 0x%08x, size = %d\n",
                   s->blockNo, s->blockCRC, s->combinedCRC, s->nblock );

      BZ2_blockSort ( s );
   }

   s->zbits = (UChar*) (&((UChar*)s->arr2)[s->nblock]);

   /*-- If this is the first block, create the stream header. --*/
   if (s->blockNo == 1) {
      BZ2_bsInitWrite ( s );
      bsPutUChar ( s, BZ_HDR_B );
      bsPutUChar ( s, BZ_HDR_Z );
      bsPutUChar ( s, BZ_HDR_h );
      bsPutUChar ( s, (UChar)(BZ_HDR_0 + s->blockSize100k) );
   }

   if (s->nblock > 0) {

      bsPutUChar ( s, 0x31 ); bsPutUChar ( s, 0x41 );
      bsPutUChar ( s, 0x59 ); bsPutUChar ( s, 0x26 );
      bsPutUChar ( s, 0x53 ); bsPutUChar ( s, 0x59 );

      /*-- Now the block's CRC, so it is in a known place. --*/
      bsPutUInt32 ( s, s->blockCRC );

      /*--
         Now a single bit indicating (non-)randomisation.
         As of version 0.9.5, we use a better sorting algorithm
         which makes randomisation unnecessary.  So always set
         the randomised bit to 'no'.  Of course, the decoder
         still needs to be able to handle randomised blocks
         so as to maintain backwards compatibility with
         older versions of bzip2.
      --*/
      bsW(s,1,0);

      bsW ( s, 24, s->origPtr );
      generateMTFValues ( s );
      sendMTFValues ( s );
   }


   /*-- If this is the last block, add the stream trailer. --*/
   if (is_last_block) {

      bsPutUChar ( s, 0x17 ); bsPutUChar ( s, 0x72 );
      bsPutUChar ( s, 0x45 ); bsPutUChar ( s, 0x38 );
      bsPutUChar ( s, 0x50 ); bsPutUChar ( s, 0x90 );
      bsPutUInt32 ( s, s->combinedCRC );
      if (s->verbosity >= 2)
         VPrintf1( "    final combined CRC = 0x%08x\n   ", s->combinedCRC );
      bsFinishWrite ( s );
   }
}

/*---------------------------------------------------*/
#ifndef BZ_NO_STDIO
void BZ2_bz__AssertH__fail ( int errcode )
{
   fprintf(stderr,
      "\n\nbzip2/libbzip2: internal error number %d.\n"
      "This is a bug in bzip2/libbzip2, %s.\n"
      "Please report it to me at: jseward@bzip.org.  If this happened\n"
      "when you were using some program which uses libbzip2 as a\n"
      "component, you should also report this bug to the author(s)\n"
      "of that program.  Please make an effort to report this bug;\n"
      "timely and accurate bug reports eventually lead to higher\n"
      "quality software.  Thanks.  Julian Seward, 10 December 2007.\n\n",
      errcode,
      BZ2_bzlibVersion()
   );

   if (errcode == 1007) {
   fprintf(stderr,
      "\n*** A special note about internal error number 1007 ***\n"
      "\n"
      "Experience suggests that a common cause of i.e. 1007\n"
      "is unreliable memory or other hardware.  The 1007 assertion\n"
      "just happens to cross-check the results of huge numbers of\n"
      "memory reads/writes, and so acts (unintendedly) as a stress\n"
      "test of your memory system.\n"
      "\n"
      "I suggest the following: try compressing the file again,\n"
      "possibly monitoring progress in detail with the -vv flag.\n"
      "\n"
      "* If the error cannot be reproduced, and/or happens at different\n"
      "  points in compression, you may have a flaky memory system.\n"
      "  Try a memory-test program.  I have used Memtest86\n"
      "  (www.memtest86.com).  At the time of writing it is free (GPLd).\n"
      "  Memtest86 tests memory much more thorougly than your BIOSs\n"
      "  power-on test, and may find failures that the BIOS doesn't.\n"
      "\n"
      "* If the error can be repeatably reproduced, this is a bug in\n"
      "  bzip2, and I would very much like to hear about it.  Please\n"
      "  let me know, and, ideally, save a copy of the file causing the\n"
      "  problem -- without which I will be unable to investigate it.\n"
      "\n"
   );
   }

   exit(3);
}
#endif


/*---------------------------------------------------*/
static
int bz_config_ok ( void )
{
   if (sizeof(int)   != 4) return 0;
   if (sizeof(short) != 2) return 0;
   if (sizeof(char)  != 1) return 0;
   return 1;
}


/*---------------------------------------------------*/
static
void* default_bzalloc ( void* opaque, Int32 items, Int32 size )
{
   void* v = malloc ( items * size );
   return v;
}

static
void default_bzfree ( void* opaque, void* addr )
{
   if (addr != NULL) free ( addr );
}


/*---------------------------------------------------*/
static
void prepare_new_block ( EState* s )
{
   Int32 i;
   s->nblock = 0;
   s->numZ = 0;
   s->state_out_pos = 0;
   BZ_INITIALISE_CRC ( s->blockCRC );
   for (i = 0; i < 256; i++) s->inUse[i] = False;
   s->blockNo++;
}


/*---------------------------------------------------*/
static
void init_RL ( EState* s )
{
   s->state_in_ch  = 256;
   s->state_in_len = 0;
}


static
Bool isempty_RL ( EState* s )
{
   if (s->state_in_ch < 256 && s->state_in_len > 0)
      return False; else
      return True;
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzCompressInit)
                    ( bz_stream* strm,
                     int        blockSize100k,
                     int        verbosity,
                     int        workFactor )
{
   Int32   n;
   EState* s;

   if (!bz_config_ok()) return BZ_CONFIG_ERROR;

   if (strm == NULL ||
       blockSize100k < 1 || blockSize100k > 9 ||
       workFactor < 0 || workFactor > 250)
     return BZ_PARAM_ERROR;

   if (workFactor == 0) workFactor = 30;
   if (strm->bzalloc == NULL) strm->bzalloc = default_bzalloc;
   if (strm->bzfree == NULL) strm->bzfree = default_bzfree;

   s = BZALLOC( sizeof(EState) );
   if (s == NULL) return BZ_MEM_ERROR;
   s->strm = strm;

   s->arr1 = NULL;
   s->arr2 = NULL;
   s->ftab = NULL;

   n       = 100000 * blockSize100k;
   s->arr1 = BZALLOC( n                  * sizeof(UInt32) );
   s->arr2 = BZALLOC( (n+BZ_N_OVERSHOOT) * sizeof(UInt32) );
   s->ftab = BZALLOC( 65537              * sizeof(UInt32) );

   if (s->arr1 == NULL || s->arr2 == NULL || s->ftab == NULL) {
      if (s->arr1 != NULL) BZFREE(s->arr1);
      if (s->arr2 != NULL) BZFREE(s->arr2);
      if (s->ftab != NULL) BZFREE(s->ftab);
      if (s       != NULL) BZFREE(s);
      return BZ_MEM_ERROR;
   }

   s->blockNo           = 0;
   s->state             = BZ_S_INPUT;
   s->mode              = BZ_M_RUNNING;
   s->combinedCRC       = 0;
   s->blockSize100k     = blockSize100k;
   s->nblockMAX         = 100000 * blockSize100k - 19;
   s->verbosity         = verbosity;
   s->workFactor        = workFactor;

   s->block             = (UChar*)s->arr2;
   s->mtfv              = (UInt16*)s->arr1;
   s->zbits             = NULL;
   s->ptr               = (UInt32*)s->arr1;

   strm->state          = s;
   strm->total_in_lo32  = 0;
   strm->total_in_hi32  = 0;
   strm->total_out_lo32 = 0;
   strm->total_out_hi32 = 0;
   init_RL ( s );
   prepare_new_block ( s );
   return BZ_OK;
}


/*---------------------------------------------------*/
static
void add_pair_to_block ( EState* s )
{
   Int32 i;
   UChar ch = (UChar)(s->state_in_ch);
   for (i = 0; i < s->state_in_len; i++) {
      BZ_UPDATE_CRC( s->blockCRC, ch );
   }
   s->inUse[s->state_in_ch] = True;
   switch (s->state_in_len) {
      case 1:
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         break;
      case 2:
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         break;
      case 3:
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         break;
      default:
         s->inUse[s->state_in_len-4] = True;
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         s->block[s->nblock] = (UChar)ch; s->nblock++;
         s->block[s->nblock] = ((UChar)(s->state_in_len-4));
         s->nblock++;
         break;
   }
}


/*---------------------------------------------------*/
static
void flush_RL ( EState* s )
{
   if (s->state_in_ch < 256) add_pair_to_block ( s );
   init_RL ( s );
}


/*---------------------------------------------------*/
#define ADD_CHAR_TO_BLOCK(zs,zchh0)               \
{                                                 \
   UInt32 zchh = (UInt32)(zchh0);                 \
   /*-- fast track the common case --*/           \
   if (zchh != zs->state_in_ch &&                 \
       zs->state_in_len == 1) {                   \
      UChar ch = (UChar)(zs->state_in_ch);        \
      BZ_UPDATE_CRC( zs->blockCRC, ch );          \
      zs->inUse[zs->state_in_ch] = True;          \
      zs->block[zs->nblock] = (UChar)ch;          \
      zs->nblock++;                               \
      zs->state_in_ch = zchh;                     \
   }                                              \
   else                                           \
   /*-- general, uncommon cases --*/              \
   if (zchh != zs->state_in_ch ||                 \
      zs->state_in_len == 255) {                  \
      if (zs->state_in_ch < 256)                  \
         add_pair_to_block ( zs );                \
      zs->state_in_ch = zchh;                     \
      zs->state_in_len = 1;                       \
   } else {                                       \
      zs->state_in_len++;                         \
   }                                              \
}


/*---------------------------------------------------*/
static
Bool copy_input_until_stop ( EState* s )
{
   Bool progress_in = False;

   if (s->mode == BZ_M_RUNNING) {

      /*-- fast track the common case --*/
      while (True) {
         /*-- block full? --*/
         if (s->nblock >= s->nblockMAX) break;
         /*-- no input? --*/
         if (s->strm->avail_in == 0) break;
         progress_in = True;
         ADD_CHAR_TO_BLOCK ( s, (UInt32)(*((UChar*)(s->strm->next_in))) );
         s->strm->next_in++;
         s->strm->avail_in--;
         s->strm->total_in_lo32++;
         if (s->strm->total_in_lo32 == 0) s->strm->total_in_hi32++;
      }

   } else {

      /*-- general, uncommon case --*/
      while (True) {
         /*-- block full? --*/
         if (s->nblock >= s->nblockMAX) break;
         /*-- no input? --*/
         if (s->strm->avail_in == 0) break;
         /*-- flush/finish end? --*/
         if (s->avail_in_expect == 0) break;
         progress_in = True;
         ADD_CHAR_TO_BLOCK ( s, (UInt32)(*((UChar*)(s->strm->next_in))) );
         s->strm->next_in++;
         s->strm->avail_in--;
         s->strm->total_in_lo32++;
         if (s->strm->total_in_lo32 == 0) s->strm->total_in_hi32++;
         s->avail_in_expect--;
      }
   }
   return progress_in;
}


/*---------------------------------------------------*/
static
Bool copy_output_until_stop ( EState* s )
{
   Bool progress_out = False;

   while (True) {

      /*-- no output space? --*/
      if (s->strm->avail_out == 0) break;

      /*-- block done? --*/
      if (s->state_out_pos >= s->numZ) break;

      progress_out = True;
      *(s->strm->next_out) = s->zbits[s->state_out_pos];
      s->state_out_pos++;
      s->strm->avail_out--;
      s->strm->next_out++;
      s->strm->total_out_lo32++;
      if (s->strm->total_out_lo32 == 0) s->strm->total_out_hi32++;
   }

   return progress_out;
}


/*---------------------------------------------------*/
static
Bool handle_compress ( bz_stream* strm )
{
   Bool progress_in  = False;
   Bool progress_out = False;
   EState* s = strm->state;

   while (True) {

      if (s->state == BZ_S_OUTPUT) {
         progress_out |= copy_output_until_stop ( s );
         if (s->state_out_pos < s->numZ) break;
         if (s->mode == BZ_M_FINISHING &&
             s->avail_in_expect == 0 &&
             isempty_RL(s)) break;
         prepare_new_block ( s );
         s->state = BZ_S_INPUT;
         if (s->mode == BZ_M_FLUSHING &&
             s->avail_in_expect == 0 &&
             isempty_RL(s)) break;
      }

      if (s->state == BZ_S_INPUT) {
         progress_in |= copy_input_until_stop ( s );
         if (s->mode != BZ_M_RUNNING && s->avail_in_expect == 0) {
            flush_RL ( s );
            BZ2_compressBlock ( s, (Bool)(s->mode == BZ_M_FINISHING) );
            s->state = BZ_S_OUTPUT;
         }
         else
         if (s->nblock >= s->nblockMAX) {
            BZ2_compressBlock ( s, False );
            s->state = BZ_S_OUTPUT;
         }
         else
         if (s->strm->avail_in == 0) {
            break;
         }
      }

   }

   return progress_in || progress_out;
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzCompress) ( bz_stream *strm, int action )
{
   Bool progress;
   EState* s;
   if (strm == NULL) return BZ_PARAM_ERROR;
   s = strm->state;
   if (s == NULL) return BZ_PARAM_ERROR;
   if (s->strm != strm) return BZ_PARAM_ERROR;

   preswitch:
   switch (s->mode) {

      case BZ_M_IDLE:
         return BZ_SEQUENCE_ERROR;

      case BZ_M_RUNNING:
         if (action == BZ_RUN) {
            progress = handle_compress ( strm );
            return progress ? BZ_RUN_OK : BZ_PARAM_ERROR;
         }
         else
	 if (action == BZ_FLUSH) {
            s->avail_in_expect = strm->avail_in;
            s->mode = BZ_M_FLUSHING;
            goto preswitch;
         }
         else
         if (action == BZ_FINISH) {
            s->avail_in_expect = strm->avail_in;
            s->mode = BZ_M_FINISHING;
            goto preswitch;
         }
         else
            return BZ_PARAM_ERROR;

      case BZ_M_FLUSHING:
         if (action != BZ_FLUSH) return BZ_SEQUENCE_ERROR;
         if (s->avail_in_expect != s->strm->avail_in)
            return BZ_SEQUENCE_ERROR;
         progress = handle_compress ( strm );
         if (s->avail_in_expect > 0 || !isempty_RL(s) ||
             s->state_out_pos < s->numZ) return BZ_FLUSH_OK;
         s->mode = BZ_M_RUNNING;
         return BZ_RUN_OK;

      case BZ_M_FINISHING:
         if (action != BZ_FINISH) return BZ_SEQUENCE_ERROR;
         if (s->avail_in_expect != s->strm->avail_in)
            return BZ_SEQUENCE_ERROR;
         progress = handle_compress ( strm );
         if (!progress) return BZ_SEQUENCE_ERROR;
         if (s->avail_in_expect > 0 || !isempty_RL(s) ||
             s->state_out_pos < s->numZ) return BZ_FINISH_OK;
         s->mode = BZ_M_IDLE;
         return BZ_STREAM_END;
   }
   return BZ_OK; /*--not reached--*/
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzCompressEnd)  ( bz_stream *strm )
{
   EState* s;
   if (strm == NULL) return BZ_PARAM_ERROR;
   s = strm->state;
   if (s == NULL) return BZ_PARAM_ERROR;
   if (s->strm != strm) return BZ_PARAM_ERROR;

   if (s->arr1 != NULL) BZFREE(s->arr1);
   if (s->arr2 != NULL) BZFREE(s->arr2);
   if (s->ftab != NULL) BZFREE(s->ftab);
   BZFREE(strm->state);

   strm->state = NULL;

   return BZ_OK;
}


/*---------------------------------------------------*/
/*--- Decompression stuff                         ---*/
/*---------------------------------------------------*/

/*---------------------------------------------------*/
int BZ_API(BZ2_bzDecompressInit)
                     ( bz_stream* strm,
                       int        verbosity,
                       int        small )
{
   DState* s;

   if (!bz_config_ok()) return BZ_CONFIG_ERROR;

   if (strm == NULL) return BZ_PARAM_ERROR;
   if (small != 0 && small != 1) return BZ_PARAM_ERROR;
   if (verbosity < 0 || verbosity > 4) return BZ_PARAM_ERROR;

   if (strm->bzalloc == NULL) strm->bzalloc = default_bzalloc;
   if (strm->bzfree == NULL) strm->bzfree = default_bzfree;

   s = BZALLOC( sizeof(DState) );
   if (s == NULL) return BZ_MEM_ERROR;
   s->strm                  = strm;
   strm->state              = s;
   s->state                 = BZ_X_MAGIC_1;
   s->bsLive                = 0;
   s->bsBuff                = 0;
   s->calculatedCombinedCRC = 0;
   strm->total_in_lo32      = 0;
   strm->total_in_hi32      = 0;
   strm->total_out_lo32     = 0;
   strm->total_out_hi32     = 0;
   s->smallDecompress       = (Bool)small;
   s->ll4                   = NULL;
   s->ll16                  = NULL;
   s->tt                    = NULL;
   s->currBlockNo           = 0;
   s->verbosity             = verbosity;

   return BZ_OK;
}


/*---------------------------------------------------*/
/* Return  True iff data corruption is discovered.
   Returns False if there is no problem.
*/
static
Bool unRLE_obuf_to_output_FAST ( DState* s )
{
   UChar k1;

   if (s->blockRandomised) {

      while (True) {
         /* try to finish existing run */
         while (True) {
            if (s->strm->avail_out == 0) return False;
            if (s->state_out_len == 0) break;
            *( (UChar*)(s->strm->next_out) ) = s->state_out_ch;
            BZ_UPDATE_CRC ( s->calculatedBlockCRC, s->state_out_ch );
            s->state_out_len--;
            s->strm->next_out++;
            s->strm->avail_out--;
            s->strm->total_out_lo32++;
            if (s->strm->total_out_lo32 == 0) s->strm->total_out_hi32++;
         }

         /* can a new run be started? */
         if (s->nblock_used == s->save_nblock+1) return False;

         /* Only caused by corrupt data stream? */
         if (s->nblock_used > s->save_nblock+1)
            return True;

         s->state_out_len = 1;
         s->state_out_ch = s->k0;
         BZ_GET_FAST(k1); BZ_RAND_UPD_MASK;
         k1 ^= BZ_RAND_MASK; s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };

         s->state_out_len = 2;
         BZ_GET_FAST(k1); BZ_RAND_UPD_MASK;
         k1 ^= BZ_RAND_MASK; s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };

         s->state_out_len = 3;
         BZ_GET_FAST(k1); BZ_RAND_UPD_MASK;
         k1 ^= BZ_RAND_MASK; s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };

         BZ_GET_FAST(k1); BZ_RAND_UPD_MASK;
         k1 ^= BZ_RAND_MASK; s->nblock_used++;
         s->state_out_len = ((Int32)k1) + 4;
         BZ_GET_FAST(s->k0); BZ_RAND_UPD_MASK;
         s->k0 ^= BZ_RAND_MASK; s->nblock_used++;
      }

   } else {

      /* restore */
      UInt32        c_calculatedBlockCRC = s->calculatedBlockCRC;
      UChar         c_state_out_ch       = s->state_out_ch;
      Int32         c_state_out_len      = s->state_out_len;
      Int32         c_nblock_used        = s->nblock_used;
      Int32         c_k0                 = s->k0;
      UInt32*       c_tt                 = s->tt;
      UInt32        c_tPos               = s->tPos;
      char*         cs_next_out          = s->strm->next_out;
      unsigned int  cs_avail_out         = s->strm->avail_out;
      Int32         ro_blockSize100k     = s->blockSize100k;
      /* end restore */

      UInt32       avail_out_INIT = cs_avail_out;
      Int32        s_save_nblockPP = s->save_nblock+1;
      unsigned int total_out_lo32_old;

      while (True) {

         /* try to finish existing run */
         if (c_state_out_len > 0) {
            while (True) {
               if (cs_avail_out == 0) goto return_notr;
               if (c_state_out_len == 1) break;
               *( (UChar*)(cs_next_out) ) = c_state_out_ch;
               BZ_UPDATE_CRC ( c_calculatedBlockCRC, c_state_out_ch );
               c_state_out_len--;
               cs_next_out++;
               cs_avail_out--;
            }
            s_state_out_len_eq_one:
            {
               if (cs_avail_out == 0) {
                  c_state_out_len = 1; goto return_notr;
               };
               *( (UChar*)(cs_next_out) ) = c_state_out_ch;
               BZ_UPDATE_CRC ( c_calculatedBlockCRC, c_state_out_ch );
               cs_next_out++;
               cs_avail_out--;
            }
         }
         /* Only caused by corrupt data stream? */
         if (c_nblock_used > s_save_nblockPP)
            return True;

         /* can a new run be started? */
         if (c_nblock_used == s_save_nblockPP) {
            c_state_out_len = 0; goto return_notr;
         };
         c_state_out_ch = c_k0;
         BZ_GET_FAST_C(k1); c_nblock_used++;
         if (k1 != c_k0) {
            c_k0 = k1; goto s_state_out_len_eq_one;
         };
         if (c_nblock_used == s_save_nblockPP)
            goto s_state_out_len_eq_one;

         c_state_out_len = 2;
         BZ_GET_FAST_C(k1); c_nblock_used++;
         if (c_nblock_used == s_save_nblockPP) continue;
         if (k1 != c_k0) { c_k0 = k1; continue; };

         c_state_out_len = 3;
         BZ_GET_FAST_C(k1); c_nblock_used++;
         if (c_nblock_used == s_save_nblockPP) continue;
         if (k1 != c_k0) { c_k0 = k1; continue; };

         BZ_GET_FAST_C(k1); c_nblock_used++;
         c_state_out_len = ((Int32)k1) + 4;
         BZ_GET_FAST_C(c_k0); c_nblock_used++;
      }

      return_notr:
      total_out_lo32_old = s->strm->total_out_lo32;
      s->strm->total_out_lo32 += (avail_out_INIT - cs_avail_out);
      if (s->strm->total_out_lo32 < total_out_lo32_old)
         s->strm->total_out_hi32++;

      /* save */
      s->calculatedBlockCRC = c_calculatedBlockCRC;
      s->state_out_ch       = c_state_out_ch;
      s->state_out_len      = c_state_out_len;
      s->nblock_used        = c_nblock_used;
      s->k0                 = c_k0;
      s->tt                 = c_tt;
      s->tPos               = c_tPos;
      s->strm->next_out     = cs_next_out;
      s->strm->avail_out    = cs_avail_out;
      /* end save */
   }
   return False;
}



/*---------------------------------------------------*/
__inline__ Int32 BZ2_indexIntoF ( Int32 indx, Int32 *cftab )
{
   Int32 nb, na, mid;
   nb = 0;
   na = 256;
   do {
      mid = (nb + na) >> 1;
      if (indx >= cftab[mid]) nb = mid; else na = mid;
   }
   while (na - nb != 1);
   return nb;
}


/*---------------------------------------------------*/
/* Return  True iff data corruption is discovered.
   Returns False if there is no problem.
*/
static
Bool unRLE_obuf_to_output_SMALL ( DState* s )
{
   UChar k1;

   if (s->blockRandomised) {

      while (True) {
         /* try to finish existing run */
         while (True) {
            if (s->strm->avail_out == 0) return False;
            if (s->state_out_len == 0) break;
            *( (UChar*)(s->strm->next_out) ) = s->state_out_ch;
            BZ_UPDATE_CRC ( s->calculatedBlockCRC, s->state_out_ch );
            s->state_out_len--;
            s->strm->next_out++;
            s->strm->avail_out--;
            s->strm->total_out_lo32++;
            if (s->strm->total_out_lo32 == 0) s->strm->total_out_hi32++;
         }

         /* can a new run be started? */
         if (s->nblock_used == s->save_nblock+1) return False;

         /* Only caused by corrupt data stream? */
         if (s->nblock_used > s->save_nblock+1)
            return True;

         s->state_out_len = 1;
         s->state_out_ch = s->k0;
         BZ_GET_SMALL(k1); BZ_RAND_UPD_MASK;
         k1 ^= BZ_RAND_MASK; s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };

         s->state_out_len = 2;
         BZ_GET_SMALL(k1); BZ_RAND_UPD_MASK;
         k1 ^= BZ_RAND_MASK; s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };

         s->state_out_len = 3;
         BZ_GET_SMALL(k1); BZ_RAND_UPD_MASK;
         k1 ^= BZ_RAND_MASK; s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };

         BZ_GET_SMALL(k1); BZ_RAND_UPD_MASK;
         k1 ^= BZ_RAND_MASK; s->nblock_used++;
         s->state_out_len = ((Int32)k1) + 4;
         BZ_GET_SMALL(s->k0); BZ_RAND_UPD_MASK;
         s->k0 ^= BZ_RAND_MASK; s->nblock_used++;
      }

   } else {

      while (True) {
         /* try to finish existing run */
         while (True) {
            if (s->strm->avail_out == 0) return False;
            if (s->state_out_len == 0) break;
            *( (UChar*)(s->strm->next_out) ) = s->state_out_ch;
            BZ_UPDATE_CRC ( s->calculatedBlockCRC, s->state_out_ch );
            s->state_out_len--;
            s->strm->next_out++;
            s->strm->avail_out--;
            s->strm->total_out_lo32++;
            if (s->strm->total_out_lo32 == 0) s->strm->total_out_hi32++;
         }

         /* can a new run be started? */
         if (s->nblock_used == s->save_nblock+1) return False;

         /* Only caused by corrupt data stream? */
         if (s->nblock_used > s->save_nblock+1)
            return True;

         s->state_out_len = 1;
         s->state_out_ch = s->k0;
         BZ_GET_SMALL(k1); s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };

         s->state_out_len = 2;
         BZ_GET_SMALL(k1); s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };

         s->state_out_len = 3;
         BZ_GET_SMALL(k1); s->nblock_used++;
         if (s->nblock_used == s->save_nblock+1) continue;
         if (k1 != s->k0) { s->k0 = k1; continue; };

         BZ_GET_SMALL(k1); s->nblock_used++;
         s->state_out_len = ((Int32)k1) + 4;
         BZ_GET_SMALL(s->k0); s->nblock_used++;
      }

   }
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzDecompress) ( bz_stream *strm )
{
   Bool    corrupt;
   DState* s;
   if (strm == NULL) return BZ_PARAM_ERROR;
   s = strm->state;
   if (s == NULL) return BZ_PARAM_ERROR;
   if (s->strm != strm) return BZ_PARAM_ERROR;

   while (True) {
      if (s->state == BZ_X_IDLE) return BZ_SEQUENCE_ERROR;
      if (s->state == BZ_X_OUTPUT) {
         if (s->smallDecompress)
            corrupt = unRLE_obuf_to_output_SMALL ( s ); else
            corrupt = unRLE_obuf_to_output_FAST  ( s );
         if (corrupt) return BZ_DATA_ERROR;
         if (s->nblock_used == s->save_nblock+1 && s->state_out_len == 0) {
            BZ_FINALISE_CRC ( s->calculatedBlockCRC );
            if (s->verbosity >= 3)
               VPrintf2 ( " {0x%08x, 0x%08x}", s->storedBlockCRC,
                          s->calculatedBlockCRC );
            if (s->verbosity >= 2) VPrintf0 ( "]" );
            if (s->calculatedBlockCRC != s->storedBlockCRC)
               return BZ_DATA_ERROR;
            s->calculatedCombinedCRC
               = (s->calculatedCombinedCRC << 1) |
                    (s->calculatedCombinedCRC >> 31);
            s->calculatedCombinedCRC ^= s->calculatedBlockCRC;
            s->state = BZ_X_BLKHDR_1;
         } else {
            return BZ_OK;
         }
      }
      if (s->state >= BZ_X_MAGIC_1) {
         Int32 r = BZ2_decompress ( s );
         if (r == BZ_STREAM_END) {
            if (s->verbosity >= 3)
               VPrintf2 ( "\n    combined CRCs: stored = 0x%08x, computed = 0x%08x",
                          s->storedCombinedCRC, s->calculatedCombinedCRC );
            if (s->calculatedCombinedCRC != s->storedCombinedCRC)
               return BZ_DATA_ERROR;
            return r;
         }
         if (s->state != BZ_X_OUTPUT) return r;
      }
   }

   AssertH ( 0, 6001 );

   return 0;  /*NOTREACHED*/
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzDecompressEnd)  ( bz_stream *strm )
{
   DState* s;
   if (strm == NULL) return BZ_PARAM_ERROR;
   s = strm->state;
   if (s == NULL) return BZ_PARAM_ERROR;
   if (s->strm != strm) return BZ_PARAM_ERROR;

   if (s->tt   != NULL) BZFREE(s->tt);
   if (s->ll16 != NULL) BZFREE(s->ll16);
   if (s->ll4  != NULL) BZFREE(s->ll4);

   BZFREE(strm->state);
   strm->state = NULL;

   return BZ_OK;
}


#ifndef BZ_NO_STDIO
/*---------------------------------------------------*/
/*--- File I/O stuff                              ---*/
/*---------------------------------------------------*/

#define BZ_SETERR(eee)                    \
{                                         \
   if (bzerror != NULL) *bzerror = eee;   \
   if (bzf != NULL) bzf->lastErr = eee;   \
}

typedef
   struct {
      FILE*     handle;
      Char      buf[BZ_MAX_UNUSED];
      Int32     bufN;
      Bool      writing;
      bz_stream strm;
      Int32     lastErr;
      Bool      initialisedOk;
   }
   bzFile;


/*---------------------------------------------*/
static Bool myfeof ( FILE* f )
{
   Int32 c = fgetc ( f );
   if (c == EOF) return True;
   ungetc ( c, f );
   return False;
}


/*---------------------------------------------------*/
BZFILE* BZ_API(BZ2_bzWriteOpen)
                    ( int*  bzerror,
                      FILE* f,
                      int   blockSize100k,
                      int   verbosity,
                      int   workFactor )
{
   Int32   ret;
   bzFile* bzf = NULL;

   BZ_SETERR(BZ_OK);

   if (f == NULL ||
       (blockSize100k < 1 || blockSize100k > 9) ||
       (workFactor < 0 || workFactor > 250) ||
       (verbosity < 0 || verbosity > 4))
      { BZ_SETERR(BZ_PARAM_ERROR); return NULL; };

   if (ferror(f))
      { BZ_SETERR(BZ_IO_ERROR); return NULL; };

   bzf = malloc ( sizeof(bzFile) );
   if (bzf == NULL)
      { BZ_SETERR(BZ_MEM_ERROR); return NULL; };

   BZ_SETERR(BZ_OK);
   bzf->initialisedOk = False;
   bzf->bufN          = 0;
   bzf->handle        = f;
   bzf->writing       = True;
   bzf->strm.bzalloc  = NULL;
   bzf->strm.bzfree   = NULL;
   bzf->strm.opaque   = NULL;

   if (workFactor == 0) workFactor = 30;
   ret = BZ2_bzCompressInit ( &(bzf->strm), blockSize100k,
                              verbosity, workFactor );
   if (ret != BZ_OK)
      { BZ_SETERR(ret); free(bzf); return NULL; };

   bzf->strm.avail_in = 0;
   bzf->initialisedOk = True;
   return bzf;
}



/*---------------------------------------------------*/
void BZ_API(BZ2_bzWrite)
             ( int*    bzerror,
               BZFILE* b,
               void*   buf,
               int     len )
{
   Int32 n, n2, ret;
   bzFile* bzf = (bzFile*)b;

   BZ_SETERR(BZ_OK);
   if (bzf == NULL || buf == NULL || len < 0)
      { BZ_SETERR(BZ_PARAM_ERROR); return; };
   if (!(bzf->writing))
      { BZ_SETERR(BZ_SEQUENCE_ERROR); return; };
   if (ferror(bzf->handle))
      { BZ_SETERR(BZ_IO_ERROR); return; };

   if (len == 0)
      { BZ_SETERR(BZ_OK); return; };

   bzf->strm.avail_in = len;
   bzf->strm.next_in  = buf;

   while (True) {
      bzf->strm.avail_out = BZ_MAX_UNUSED;
      bzf->strm.next_out = bzf->buf;
      ret = BZ2_bzCompress ( &(bzf->strm), BZ_RUN );
      if (ret != BZ_RUN_OK)
         { BZ_SETERR(ret); return; };

      if (bzf->strm.avail_out < BZ_MAX_UNUSED) {
         n = BZ_MAX_UNUSED - bzf->strm.avail_out;
         n2 = fwrite ( (void*)(bzf->buf), sizeof(UChar),
                       n, bzf->handle );
         if (n != n2 || ferror(bzf->handle))
            { BZ_SETERR(BZ_IO_ERROR); return; };
      }

      if (bzf->strm.avail_in == 0)
         { BZ_SETERR(BZ_OK); return; };
   }
}


/*---------------------------------------------------*/
void BZ_API(BZ2_bzWriteClose)
                  ( int*          bzerror,
                    BZFILE*       b,
                    int           abandon,
                    unsigned int* nbytes_in,
                    unsigned int* nbytes_out )
{
   BZ2_bzWriteClose64 ( bzerror, b, abandon,
                        nbytes_in, NULL, nbytes_out, NULL );
}


void BZ_API(BZ2_bzWriteClose64)
                  ( int*          bzerror,
                    BZFILE*       b,
                    int           abandon,
                    unsigned int* nbytes_in_lo32,
                    unsigned int* nbytes_in_hi32,
                    unsigned int* nbytes_out_lo32,
                    unsigned int* nbytes_out_hi32 )
{
   Int32   n, n2, ret;
   bzFile* bzf = (bzFile*)b;

   if (bzf == NULL)
      { BZ_SETERR(BZ_OK); return; };
   if (!(bzf->writing))
      { BZ_SETERR(BZ_SEQUENCE_ERROR); return; };
   if (ferror(bzf->handle))
      { BZ_SETERR(BZ_IO_ERROR); return; };

   if (nbytes_in_lo32 != NULL) *nbytes_in_lo32 = 0;
   if (nbytes_in_hi32 != NULL) *nbytes_in_hi32 = 0;
   if (nbytes_out_lo32 != NULL) *nbytes_out_lo32 = 0;
   if (nbytes_out_hi32 != NULL) *nbytes_out_hi32 = 0;

   if ((!abandon) && bzf->lastErr == BZ_OK) {
      while (True) {
         bzf->strm.avail_out = BZ_MAX_UNUSED;
         bzf->strm.next_out = bzf->buf;
         ret = BZ2_bzCompress ( &(bzf->strm), BZ_FINISH );
         if (ret != BZ_FINISH_OK && ret != BZ_STREAM_END)
            { BZ_SETERR(ret); return; };

         if (bzf->strm.avail_out < BZ_MAX_UNUSED) {
            n = BZ_MAX_UNUSED - bzf->strm.avail_out;
            n2 = fwrite ( (void*)(bzf->buf), sizeof(UChar),
                          n, bzf->handle );
            if (n != n2 || ferror(bzf->handle))
               { BZ_SETERR(BZ_IO_ERROR); return; };
         }

         if (ret == BZ_STREAM_END) break;
      }
   }

   if ( !abandon && !ferror ( bzf->handle ) ) {
      fflush ( bzf->handle );
      if (ferror(bzf->handle))
         { BZ_SETERR(BZ_IO_ERROR); return; };
   }

   if (nbytes_in_lo32 != NULL)
      *nbytes_in_lo32 = bzf->strm.total_in_lo32;
   if (nbytes_in_hi32 != NULL)
      *nbytes_in_hi32 = bzf->strm.total_in_hi32;
   if (nbytes_out_lo32 != NULL)
      *nbytes_out_lo32 = bzf->strm.total_out_lo32;
   if (nbytes_out_hi32 != NULL)
      *nbytes_out_hi32 = bzf->strm.total_out_hi32;

   BZ_SETERR(BZ_OK);
   BZ2_bzCompressEnd ( &(bzf->strm) );
   free ( bzf );
}


/*---------------------------------------------------*/
BZFILE* BZ_API(BZ2_bzReadOpen)
                   ( int*  bzerror,
                     FILE* f,
                     int   verbosity,
                     int   small,
                     void* unused,
                     int   nUnused )
{
   bzFile* bzf = NULL;
   int     ret;

   BZ_SETERR(BZ_OK);

   if (f == NULL ||
       (small != 0 && small != 1) ||
       (verbosity < 0 || verbosity > 4) ||
       (unused == NULL && nUnused != 0) ||
       (unused != NULL && (nUnused < 0 || nUnused > BZ_MAX_UNUSED)))
      { BZ_SETERR(BZ_PARAM_ERROR); return NULL; };

   if (ferror(f))
      { BZ_SETERR(BZ_IO_ERROR); return NULL; };

   bzf = malloc ( sizeof(bzFile) );
   if (bzf == NULL)
      { BZ_SETERR(BZ_MEM_ERROR); return NULL; };

   BZ_SETERR(BZ_OK);

   bzf->initialisedOk = False;
   bzf->handle        = f;
   bzf->bufN          = 0;
   bzf->writing       = False;
   bzf->strm.bzalloc  = NULL;
   bzf->strm.bzfree   = NULL;
   bzf->strm.opaque   = NULL;

   while (nUnused > 0) {
      bzf->buf[bzf->bufN] = *((UChar*)(unused)); bzf->bufN++;
      unused = ((void*)( 1 + ((UChar*)(unused))  ));
      nUnused--;
   }

   ret = BZ2_bzDecompressInit ( &(bzf->strm), verbosity, small );
   if (ret != BZ_OK)
      { BZ_SETERR(ret); free(bzf); return NULL; };

   bzf->strm.avail_in = bzf->bufN;
   bzf->strm.next_in  = bzf->buf;

   bzf->initialisedOk = True;
   return bzf;
}


/*---------------------------------------------------*/
void BZ_API(BZ2_bzReadClose) ( int *bzerror, BZFILE *b )
{
   bzFile* bzf = (bzFile*)b;

   BZ_SETERR(BZ_OK);
   if (bzf == NULL)
      { BZ_SETERR(BZ_OK); return; };

   if (bzf->writing)
      { BZ_SETERR(BZ_SEQUENCE_ERROR); return; };

   if (bzf->initialisedOk)
      (void)BZ2_bzDecompressEnd ( &(bzf->strm) );
   free ( bzf );
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzRead)
           ( int*    bzerror,
             BZFILE* b,
             void*   buf,
             int     len )
{
   Int32   n, ret;
   bzFile* bzf = (bzFile*)b;

   BZ_SETERR(BZ_OK);

   if (bzf == NULL || buf == NULL || len < 0)
      { BZ_SETERR(BZ_PARAM_ERROR); return 0; };

   if (bzf->writing)
      { BZ_SETERR(BZ_SEQUENCE_ERROR); return 0; };

   if (len == 0)
      { BZ_SETERR(BZ_OK); return 0; };

   bzf->strm.avail_out = len;
   bzf->strm.next_out = buf;

   while (True) {

      if (ferror(bzf->handle))
         { BZ_SETERR(BZ_IO_ERROR); return 0; };

      if (bzf->strm.avail_in == 0 && !myfeof(bzf->handle)) {
         n = fread ( bzf->buf, sizeof(UChar),
                     BZ_MAX_UNUSED, bzf->handle );
         if (ferror(bzf->handle))
            { BZ_SETERR(BZ_IO_ERROR); return 0; };
         bzf->bufN = n;
         bzf->strm.avail_in = bzf->bufN;
         bzf->strm.next_in = bzf->buf;
      }

      ret = BZ2_bzDecompress ( &(bzf->strm) );

      if (ret != BZ_OK && ret != BZ_STREAM_END)
         { BZ_SETERR(ret); return 0; };

      if (ret == BZ_OK && myfeof(bzf->handle) &&
          bzf->strm.avail_in == 0 && bzf->strm.avail_out > 0)
         { BZ_SETERR(BZ_UNEXPECTED_EOF); return 0; };

      if (ret == BZ_STREAM_END)
         { BZ_SETERR(BZ_STREAM_END);
           return len - bzf->strm.avail_out; };
      if (bzf->strm.avail_out == 0)
         { BZ_SETERR(BZ_OK); return len; };

   }

   return 0; /*not reached*/
}


/*---------------------------------------------------*/
void BZ_API(BZ2_bzReadGetUnused)
                     ( int*    bzerror,
                       BZFILE* b,
                       void**  unused,
                       int*    nUnused )
{
   bzFile* bzf = (bzFile*)b;
   if (bzf == NULL)
      { BZ_SETERR(BZ_PARAM_ERROR); return; };
   if (bzf->lastErr != BZ_STREAM_END)
      { BZ_SETERR(BZ_SEQUENCE_ERROR); return; };
   if (unused == NULL || nUnused == NULL)
      { BZ_SETERR(BZ_PARAM_ERROR); return; };

   BZ_SETERR(BZ_OK);
   *nUnused = bzf->strm.avail_in;
   *unused = bzf->strm.next_in;
}
#endif


/*---------------------------------------------------*/
/*--- Misc convenience stuff                      ---*/
/*---------------------------------------------------*/

/*---------------------------------------------------*/
int BZ_API(BZ2_bzBuffToBuffCompress)
                         ( char*         dest,
                           unsigned int* destLen,
                           char*         source,
                           unsigned int  sourceLen,
                           int           blockSize100k,
                           int           verbosity,
                           int           workFactor )
{
   bz_stream strm;
   int ret;

   if (dest == NULL || destLen == NULL ||
       source == NULL ||
       blockSize100k < 1 || blockSize100k > 9 ||
       verbosity < 0 || verbosity > 4 ||
       workFactor < 0 || workFactor > 250)
      return BZ_PARAM_ERROR;

   if (workFactor == 0) workFactor = 30;
   strm.bzalloc = NULL;
   strm.bzfree = NULL;
   strm.opaque = NULL;
   ret = BZ2_bzCompressInit ( &strm, blockSize100k,
                              verbosity, workFactor );
   if (ret != BZ_OK) return ret;

   strm.next_in = source;
   strm.next_out = dest;
   strm.avail_in = sourceLen;
   strm.avail_out = *destLen;

   ret = BZ2_bzCompress ( &strm, BZ_FINISH );
   if (ret == BZ_FINISH_OK) goto output_overflow;
   if (ret != BZ_STREAM_END) goto errhandler;

   /* normal termination */
   *destLen -= strm.avail_out;
   BZ2_bzCompressEnd ( &strm );
   return BZ_OK;

   output_overflow:
   BZ2_bzCompressEnd ( &strm );
   return BZ_OUTBUFF_FULL;

   errhandler:
   BZ2_bzCompressEnd ( &strm );
   return ret;
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzBuffToBuffDecompress)
                           ( char*         dest,
                             unsigned int* destLen,
                             char*         source,
                             unsigned int  sourceLen,
                             int           small,
                             int           verbosity )
{
   bz_stream strm;
   int ret;

   if (dest == NULL || destLen == NULL ||
       source == NULL ||
       (small != 0 && small != 1) ||
       verbosity < 0 || verbosity > 4)
          return BZ_PARAM_ERROR;

   strm.bzalloc = NULL;
   strm.bzfree = NULL;
   strm.opaque = NULL;
   ret = BZ2_bzDecompressInit ( &strm, verbosity, small );
   if (ret != BZ_OK) return ret;

   strm.next_in = source;
   strm.next_out = dest;
   strm.avail_in = sourceLen;
   strm.avail_out = *destLen;

   ret = BZ2_bzDecompress ( &strm );
   if (ret == BZ_OK) goto output_overflow_or_eof;
   if (ret != BZ_STREAM_END) goto errhandler;

   /* normal termination */
   *destLen -= strm.avail_out;
   BZ2_bzDecompressEnd ( &strm );
   return BZ_OK;

   output_overflow_or_eof:
   if (strm.avail_out > 0) {
      BZ2_bzDecompressEnd ( &strm );
      return BZ_UNEXPECTED_EOF;
   } else {
      BZ2_bzDecompressEnd ( &strm );
      return BZ_OUTBUFF_FULL;
   };

   errhandler:
   BZ2_bzDecompressEnd ( &strm );
   return ret;
}


/*---------------------------------------------------*/
/*--
   Code contributed by Yoshioka Tsuneo (tsuneo@rr.iij4u.or.jp)
   to support better zlib compatibility.
   This code is not _officially_ part of libbzip2 (yet);
   I haven't tested it, documented it, or considered the
   threading-safeness of it.
   If this code breaks, please contact both Yoshioka and me.
--*/
/*---------------------------------------------------*/

/*---------------------------------------------------*/
/*--
   return version like "0.9.5d, 4-Sept-1999".
--*/
const char * BZ_API(BZ2_bzlibVersion)(void)
{
   return BZ_VERSION;
}


#ifndef BZ_NO_STDIO
/*---------------------------------------------------*/

#if defined(_WIN32) || defined(OS2) || defined(MSDOS)
#   include <fcntl.h>
#   include <io.h>
#   define SET_BINARY_MODE(file) setmode(fileno(file),O_BINARY)
#else
#   define SET_BINARY_MODE(file)
#endif
static
BZFILE * bzopen_or_bzdopen
               ( const char *path,   /* no use when bzdopen */
                 int fd,             /* no use when bzdopen */
                 const char *mode,
                 int open_mode)      /* bzopen: 0, bzdopen:1 */
{
   int    bzerr;
   char   unused[BZ_MAX_UNUSED];
   int    blockSize100k = 9;
   int    writing       = 0;
   char   mode2[10]     = "";
   FILE   *fp           = NULL;
   BZFILE *bzfp         = NULL;
   int    verbosity     = 0;
   int    workFactor    = 30;
   int    smallMode     = 0;
   int    nUnused       = 0;

   if (mode == NULL) return NULL;
   while (*mode) {
      switch (*mode) {
      case 'r':
         writing = 0; break;
      case 'w':
         writing = 1; break;
      case 's':
         smallMode = 1; break;
      default:
         if (isdigit((int)(*mode))) {
            blockSize100k = *mode-BZ_HDR_0;
         }
      }
      mode++;
   }
   strcat(mode2, writing ? "w" : "r" );
   strcat(mode2,"b");   /* binary mode */

   if (open_mode==0) {
      if (path==NULL || strcmp(path,"")==0) {
        fp = (writing ? stdout : stdin);
        SET_BINARY_MODE(fp);
      } else {
        fp = fopen(path,mode2);
      }
   } else {
#ifdef BZ_STRICT_ANSI
      fp = NULL;
#else
      fp = fdopen(fd,mode2);
#endif
   }
   if (fp == NULL) return NULL;

   if (writing) {
      /* Guard against total chaos and anarchy -- JRS */
      if (blockSize100k < 1) blockSize100k = 1;
      if (blockSize100k > 9) blockSize100k = 9;
      bzfp = BZ2_bzWriteOpen(&bzerr,fp,blockSize100k,
                             verbosity,workFactor);
   } else {
      bzfp = BZ2_bzReadOpen(&bzerr,fp,verbosity,smallMode,
                            unused,nUnused);
   }
   if (bzfp == NULL) {
      if (fp != stdin && fp != stdout) fclose(fp);
      return NULL;
   }
   return bzfp;
}


/*---------------------------------------------------*/
/*--
   open file for read or write.
      ex) bzopen("file","w9")
      case path="" or NULL => use stdin or stdout.
--*/
BZFILE * BZ_API(BZ2_bzopen)
               ( const char *path,
                 const char *mode )
{
   return bzopen_or_bzdopen(path,-1,mode,/*bzopen*/0);
}


/*---------------------------------------------------*/
BZFILE * BZ_API(BZ2_bzdopen)
               ( int fd,
                 const char *mode )
{
   return bzopen_or_bzdopen(NULL,fd,mode,/*bzdopen*/1);
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzread) (BZFILE* b, void* buf, int len )
{
   int bzerr, nread;
   if (((bzFile*)b)->lastErr == BZ_STREAM_END) return 0;
   nread = BZ2_bzRead(&bzerr,b,buf,len);
   if (bzerr == BZ_OK || bzerr == BZ_STREAM_END) {
      return nread;
   } else {
      return -1;
   }
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzwrite) (BZFILE* b, void* buf, int len )
{
   int bzerr;

   BZ2_bzWrite(&bzerr,b,buf,len);
   if(bzerr == BZ_OK){
      return len;
   }else{
      return -1;
   }
}


/*---------------------------------------------------*/
int BZ_API(BZ2_bzflush) (BZFILE *b)
{
   /* do nothing now... */
   return 0;
}


/*---------------------------------------------------*/
void BZ_API(BZ2_bzclose) (BZFILE* b)
{
   int bzerr;
   FILE *fp;

   if (b==NULL) {return;}
   fp = ((bzFile *)b)->handle;
   if(((bzFile*)b)->writing){
      BZ2_bzWriteClose(&bzerr,b,0,NULL,NULL);
      if(bzerr != BZ_OK){
         BZ2_bzWriteClose(NULL,b,1,NULL,NULL);
      }
   }else{
      BZ2_bzReadClose(&bzerr,b);
   }
   if(fp!=stdin && fp!=stdout){
      fclose(fp);
   }
}


/*---------------------------------------------------*/
/*--
   return last error code
--*/
static const char *bzerrorstrings[] = {
       "OK"
      ,"SEQUENCE_ERROR"
      ,"PARAM_ERROR"
      ,"MEM_ERROR"
      ,"DATA_ERROR"
      ,"DATA_ERROR_MAGIC"
      ,"IO_ERROR"
      ,"UNEXPECTED_EOF"
      ,"OUTBUFF_FULL"
      ,"CONFIG_ERROR"
      ,"???"   /* for future */
      ,"???"   /* for future */
      ,"???"   /* for future */
      ,"???"   /* for future */
      ,"???"   /* for future */
      ,"???"   /* for future */
};


const char * BZ_API(BZ2_bzerror) (BZFILE *b, int *errnum)
{
   int err = ((bzFile *)b)->lastErr;

   if(err>0) err = 0;
   *errnum = err;
   return bzerrorstrings[err*-1];
}
#endif

#define BZ_UNIX      1

/*--
  Win32, as seen by Jacob Navia's excellent
  port of (Chris Fraser & David Hanson)'s excellent
  lcc compiler.  Or with MS Visual C.
  This is selected automatically if compiled by a compiler which
  defines _WIN32, not including the Cygwin GCC.
--*/
#define BZ_LCCWIN32  0

#if defined(_WIN32) && !defined(__CYGWIN__)
#undef  BZ_LCCWIN32
#define BZ_LCCWIN32 1
#undef  BZ_UNIX
#define BZ_UNIX 0
#endif


/*---------------------------------------------*/
/*--
  Some stuff for all platforms.
--*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include "bzlib.h"

#define ERROR_IF_EOF(i)       { if ((i) == EOF)  ioError(); }
#define ERROR_IF_NOT_ZERO(i)  { if ((i) != 0)    ioError(); }
#define ERROR_IF_MINUS_ONE(i) { if ((i) == (-1)) ioError(); }


/*---------------------------------------------*/
/*--
   Platform-specific stuff.
--*/

#if BZ_UNIX
#   include <fcntl.h>
#   include <sys/types.h>
#   include <utime.h>
#   include <unistd.h>
#   include <sys/stat.h>
#   include <sys/times.h>

#   define PATH_SEP    '/'
#   define MY_LSTAT    lstat
#   define MY_STAT     stat
#   define MY_S_ISREG  S_ISREG
#   define MY_S_ISDIR  S_ISDIR

#   define APPEND_FILESPEC(root, name) \
      root=snocString((root), (name))

#   define APPEND_FLAG(root, name) \
      root=snocString((root), (name))

#   define SET_BINARY_MODE(fd) /**/

#   ifdef __GNUC__
#      define NORETURN __attribute__ ((noreturn))
#   else
#      define NORETURN /**/
#   endif

#   ifdef __DJGPP__
#     include <io.h>
#     include <fcntl.h>
#     undef MY_LSTAT
#     undef MY_STAT
#     define MY_LSTAT stat
#     define MY_STAT stat
#     undef SET_BINARY_MODE
#     define SET_BINARY_MODE(fd)                        \
        do {                                            \
           int retVal = setmode ( fileno ( fd ),        \
                                  O_BINARY );           \
           ERROR_IF_MINUS_ONE ( retVal );               \
        } while ( 0 )
#   endif

#   ifdef __CYGWIN__
#     include <io.h>
#     include <fcntl.h>
#     undef SET_BINARY_MODE
#     define SET_BINARY_MODE(fd)                        \
        do {                                            \
           int retVal = setmode ( fileno ( fd ),        \
                                  O_BINARY );           \
           ERROR_IF_MINUS_ONE ( retVal );               \
        } while ( 0 )
#   endif
#endif /* BZ_UNIX */



#if BZ_LCCWIN32
#   include <io.h>
#   include <fcntl.h>
#   include <sys\stat.h>

#   define NORETURN       /**/
#   define PATH_SEP       '\\'
#   define MY_LSTAT       _stat
#   define MY_STAT        _stat
#   define MY_S_ISREG(x)  ((x) & _S_IFREG)
#   define MY_S_ISDIR(x)  ((x) & _S_IFDIR)

#   define APPEND_FLAG(root, name) \
      root=snocString((root), (name))

#   define APPEND_FILESPEC(root, name)                \
      root = snocString ((root), (name))

#   define SET_BINARY_MODE(fd)                        \
      do {                                            \
         int retVal = setmode ( fileno ( fd ),        \
                                O_BINARY );           \
         ERROR_IF_MINUS_ONE ( retVal );               \
      } while ( 0 )

#endif /* BZ_LCCWIN32 */


/*---------------------------------------------*/
/*--
  Some more stuff for all platforms :-)
--*/

typedef char            Char;
typedef unsigned char   Bool;
typedef unsigned char   UChar;
typedef int             Int32;
typedef unsigned int    UInt32;
typedef short           Int16;
typedef unsigned short  UInt16;

#define True  ((Bool)1)
#define False ((Bool)0)

/*--
  IntNative is your platform's `native' int size.
  Only here to avoid probs with 64-bit platforms.
--*/
typedef int IntNative;


/*---------------------------------------------------*/
/*--- Misc (file handling) data decls             ---*/
/*---------------------------------------------------*/

Int32   verbosity;
Bool    keepInputFiles, smallMode, deleteOutputOnInterrupt;
Bool    forceOverwrite, testFailsExist, unzFailsExist, noisy;
Int32   numFileNames, numFilesProcessed, blockSize100k;
Int32   exitValue;

/*-- source modes; F==file, I==stdin, O==stdout --*/
#define SM_I2O           1
#define SM_F2O           2
#define SM_F2F           3

/*-- operation modes --*/
#define OM_Z             1
#define OM_UNZ           2
#define OM_TEST          3

Int32   opMode;
Int32   srcMode;

#define FILE_NAME_LEN 1034

Int32   longestFileName;
Char    inName [FILE_NAME_LEN];
Char    outName[FILE_NAME_LEN];
Char    tmpName[FILE_NAME_LEN];
Char    *progName;
Char    progNameReally[FILE_NAME_LEN];
FILE    *outputHandleJustInCase;
Int32   workFactor;

static void    panic                 ( const Char* ) NORETURN;
static void    ioError               ( void )        NORETURN;
static void    outOfMemory           ( void )        NORETURN;
static void    configError           ( void )        NORETURN;
static void    crcError              ( void )        NORETURN;
static void    cleanUpAndFail        ( Int32 )       NORETURN;
static void    compressedStreamEOF   ( void )        NORETURN;

static void    copyFileName ( Char*, Char* );
static void*   myMalloc     ( Int32 );
static void    applySavedFileAttrToOutputFile ( IntNative fd );



/*---------------------------------------------------*/
/*--- An implementation of 64-bit ints.  Sigh.    ---*/
/*--- Roll on widespread deployment of ANSI C9X ! ---*/
/*---------------------------------------------------*/

typedef
   struct { UChar b[8]; }
   UInt64;


static
void uInt64_from_UInt32s ( UInt64* n, UInt32 lo32, UInt32 hi32 )
{
   n->b[7] = (UChar)((hi32 >> 24) & 0xFF);
   n->b[6] = (UChar)((hi32 >> 16) & 0xFF);
   n->b[5] = (UChar)((hi32 >> 8)  & 0xFF);
   n->b[4] = (UChar) (hi32        & 0xFF);
   n->b[3] = (UChar)((lo32 >> 24) & 0xFF);
   n->b[2] = (UChar)((lo32 >> 16) & 0xFF);
   n->b[1] = (UChar)((lo32 >> 8)  & 0xFF);
   n->b[0] = (UChar) (lo32        & 0xFF);
}


static
double uInt64_to_double ( UInt64* n )
{
   Int32  i;
   double base = 1.0;
   double sum  = 0.0;
   for (i = 0; i < 8; i++) {
      sum  += base * (double)(n->b[i]);
      base *= 256.0;
   }
   return sum;
}


static
Bool uInt64_isZero ( UInt64* n )
{
   Int32 i;
   for (i = 0; i < 8; i++)
      if (n->b[i] != 0) return 0;
   return 1;
}


/* Divide *n by 10, and return the remainder.  */
static
Int32 uInt64_qrm10 ( UInt64* n )
{
   UInt32 rem, tmp;
   Int32  i;
   rem = 0;
   for (i = 7; i >= 0; i--) {
      tmp = rem * 256 + n->b[i];
      n->b[i] = tmp / 10;
      rem = tmp % 10;
   }
   return rem;
}


/* ... and the Whole Entire Point of all this UInt64 stuff is
   so that we can supply the following function.
*/
static
void uInt64_toAscii ( char* outbuf, UInt64* n )
{
   Int32  i, q;
   UChar  buf[32];
   Int32  nBuf   = 0;
   UInt64 n_copy = *n;
   do {
      q = uInt64_qrm10 ( &n_copy );
      buf[nBuf] = q + '0';
      nBuf++;
   } while (!uInt64_isZero(&n_copy));
   outbuf[nBuf] = 0;
   for (i = 0; i < nBuf; i++)
      outbuf[i] = buf[nBuf-i-1];
}


/*---------------------------------------------------*/
/*--- Processing of complete files and streams    ---*/
/*---------------------------------------------------*/

/*---------------------------------------------*/
static
Bool myfeof ( FILE* f )
{
   Int32 c = fgetc ( f );
   if (c == EOF) return True;
   ungetc ( c, f );
   return False;
}


/*---------------------------------------------*/
static
void compressStream ( FILE *stream, FILE *zStream )
{
   BZFILE* bzf = NULL;
   UChar   ibuf[5000];
   Int32   nIbuf;
   UInt32  nbytes_in_lo32, nbytes_in_hi32;
   UInt32  nbytes_out_lo32, nbytes_out_hi32;
   Int32   bzerr, bzerr_dummy, ret;

   SET_BINARY_MODE(stream);
   SET_BINARY_MODE(zStream);

   if (ferror(stream)) goto errhandler_io;
   if (ferror(zStream)) goto errhandler_io;

   bzf = BZ2_bzWriteOpen ( &bzerr, zStream,
                           blockSize100k, verbosity, workFactor );
   if (bzerr != BZ_OK) goto errhandler;

   if (verbosity >= 2) fprintf ( stderr, "\n" );

   while (True) {

      if (myfeof(stream)) break;
      nIbuf = fread ( ibuf, sizeof(UChar), 5000, stream );
      if (ferror(stream)) goto errhandler_io;
      if (nIbuf > 0) BZ2_bzWrite ( &bzerr, bzf, (void*)ibuf, nIbuf );
      if (bzerr != BZ_OK) goto errhandler;

   }

   BZ2_bzWriteClose64 ( &bzerr, bzf, 0,
                        &nbytes_in_lo32, &nbytes_in_hi32,
                        &nbytes_out_lo32, &nbytes_out_hi32 );
   if (bzerr != BZ_OK) goto errhandler;

   if (ferror(zStream)) goto errhandler_io;
   ret = fflush ( zStream );
   if (ret == EOF) goto errhandler_io;
   if (zStream != stdout) {
      Int32 fd = fileno ( zStream );
      if (fd < 0) goto errhandler_io;
      applySavedFileAttrToOutputFile ( fd );
      ret = fclose ( zStream );
      outputHandleJustInCase = NULL;
      if (ret == EOF) goto errhandler_io;
   }
   outputHandleJustInCase = NULL;
   if (ferror(stream)) goto errhandler_io;
   ret = fclose ( stream );
   if (ret == EOF) goto errhandler_io;

   if (verbosity >= 1) {
      if (nbytes_in_lo32 == 0 && nbytes_in_hi32 == 0) {
	 fprintf ( stderr, " no data compressed.\n");
      } else {
	 Char   buf_nin[32], buf_nout[32];
	 UInt64 nbytes_in,   nbytes_out;
	 double nbytes_in_d, nbytes_out_d;
	 uInt64_from_UInt32s ( &nbytes_in,
			       nbytes_in_lo32, nbytes_in_hi32 );
	 uInt64_from_UInt32s ( &nbytes_out,
			       nbytes_out_lo32, nbytes_out_hi32 );
	 nbytes_in_d  = uInt64_to_double ( &nbytes_in );
	 nbytes_out_d = uInt64_to_double ( &nbytes_out );
	 uInt64_toAscii ( buf_nin, &nbytes_in );
	 uInt64_toAscii ( buf_nout, &nbytes_out );
	 fprintf ( stderr, "%6.3f:1, %6.3f bits/byte, "
		   "%5.2f%% saved, %s in, %s out.\n",
		   nbytes_in_d / nbytes_out_d,
		   (8.0 * nbytes_out_d) / nbytes_in_d,
		   100.0 * (1.0 - nbytes_out_d / nbytes_in_d),
		   buf_nin,
		   buf_nout
		 );
      }
   }

   return;

   errhandler:
   BZ2_bzWriteClose64 ( &bzerr_dummy, bzf, 1,
                        &nbytes_in_lo32, &nbytes_in_hi32,
                        &nbytes_out_lo32, &nbytes_out_hi32 );
   switch (bzerr) {
      case BZ_CONFIG_ERROR:
         configError(); break;
      case BZ_MEM_ERROR:
         outOfMemory (); break;
      case BZ_IO_ERROR:
         errhandler_io:
         ioError(); break;
      default:
         panic ( "compress:unexpected error" );
   }

   panic ( "compress:end" );
   /*notreached*/
}



/*---------------------------------------------*/
static
Bool uncompressStream ( FILE *zStream, FILE *stream )
{
   BZFILE* bzf = NULL;
   Int32   bzerr, bzerr_dummy, ret, nread, streamNo, i;
   UChar   obuf[5000];
   UChar   unused[BZ_MAX_UNUSED];
   Int32   nUnused;
   void*   unusedTmpV;
   UChar*  unusedTmp;

   nUnused = 0;
   streamNo = 0;

   SET_BINARY_MODE(stream);
   SET_BINARY_MODE(zStream);

   if (ferror(stream)) goto errhandler_io;
   if (ferror(zStream)) goto errhandler_io;

   while (True) {

      bzf = BZ2_bzReadOpen (
               &bzerr, zStream, verbosity,
               (int)smallMode, unused, nUnused
            );
      if (bzf == NULL || bzerr != BZ_OK) goto errhandler;
      streamNo++;

      while (bzerr == BZ_OK) {
         nread = BZ2_bzRead ( &bzerr, bzf, obuf, 5000 );
         if (bzerr == BZ_DATA_ERROR_MAGIC) goto trycat;
         if ((bzerr == BZ_OK || bzerr == BZ_STREAM_END) && nread > 0)
            fwrite ( obuf, sizeof(UChar), nread, stream );
         if (ferror(stream)) goto errhandler_io;
      }
      if (bzerr != BZ_STREAM_END) goto errhandler;

      BZ2_bzReadGetUnused ( &bzerr, bzf, &unusedTmpV, &nUnused );
      if (bzerr != BZ_OK) panic ( "decompress:bzReadGetUnused" );

      unusedTmp = (UChar*)unusedTmpV;
      for (i = 0; i < nUnused; i++) unused[i] = unusedTmp[i];

      BZ2_bzReadClose ( &bzerr, bzf );
      if (bzerr != BZ_OK) panic ( "decompress:bzReadGetUnused" );

      if (nUnused == 0 && myfeof(zStream)) break;
   }

   closeok:
   if (ferror(zStream)) goto errhandler_io;
   if (stream != stdout) {
      Int32 fd = fileno ( stream );
      if (fd < 0) goto errhandler_io;
      applySavedFileAttrToOutputFile ( fd );
   }
   ret = fclose ( zStream );
   if (ret == EOF) goto errhandler_io;

   if (ferror(stream)) goto errhandler_io;
   ret = fflush ( stream );
   if (ret != 0) goto errhandler_io;
   if (stream != stdout) {
      ret = fclose ( stream );
      outputHandleJustInCase = NULL;
      if (ret == EOF) goto errhandler_io;
   }
   outputHandleJustInCase = NULL;
   if (verbosity >= 2) fprintf ( stderr, "\n    " );
   return True;

   trycat:
   if (forceOverwrite) {
      rewind(zStream);
      while (True) {
      	 if (myfeof(zStream)) break;
      	 nread = fread ( obuf, sizeof(UChar), 5000, zStream );
      	 if (ferror(zStream)) goto errhandler_io;
      	 if (nread > 0) fwrite ( obuf, sizeof(UChar), nread, stream );
      	 if (ferror(stream)) goto errhandler_io;
      }
      goto closeok;
   }

   errhandler:
   BZ2_bzReadClose ( &bzerr_dummy, bzf );
   switch (bzerr) {
      case BZ_CONFIG_ERROR:
         configError(); break;
      case BZ_IO_ERROR:
         errhandler_io:
         ioError(); break;
      case BZ_DATA_ERROR:
         crcError();
      case BZ_MEM_ERROR:
         outOfMemory();
      case BZ_UNEXPECTED_EOF:
         compressedStreamEOF();
      case BZ_DATA_ERROR_MAGIC:
         if (zStream != stdin) fclose(zStream);
         if (stream != stdout) fclose(stream);
         if (streamNo == 1) {
            return False;
         } else {
            if (noisy)
            fprintf ( stderr,
                      "\n%s: %s: trailing garbage after EOF ignored\n",
                      progName, inName );
            return True;
         }
      default:
         panic ( "decompress:unexpected error" );
   }

   panic ( "decompress:end" );
   return True; /*notreached*/
}


/*---------------------------------------------*/
static
Bool testStream ( FILE *zStream )
{
   BZFILE* bzf = NULL;
   Int32   bzerr, bzerr_dummy, ret, nread, streamNo, i;
   UChar   obuf[5000];
   UChar   unused[BZ_MAX_UNUSED];
   Int32   nUnused;
   void*   unusedTmpV;
   UChar*  unusedTmp;

   nUnused = 0;
   streamNo = 0;

   SET_BINARY_MODE(zStream);
   if (ferror(zStream)) goto errhandler_io;

   while (True) {

      bzf = BZ2_bzReadOpen (
               &bzerr, zStream, verbosity,
               (int)smallMode, unused, nUnused
            );
      if (bzf == NULL || bzerr != BZ_OK) goto errhandler;
      streamNo++;

      while (bzerr == BZ_OK) {
         nread = BZ2_bzRead ( &bzerr, bzf, obuf, 5000 );
         if (bzerr == BZ_DATA_ERROR_MAGIC) goto errhandler;
      }
      if (bzerr != BZ_STREAM_END) goto errhandler;

      BZ2_bzReadGetUnused ( &bzerr, bzf, &unusedTmpV, &nUnused );
      if (bzerr != BZ_OK) panic ( "test:bzReadGetUnused" );

      unusedTmp = (UChar*)unusedTmpV;
      for (i = 0; i < nUnused; i++) unused[i] = unusedTmp[i];

      BZ2_bzReadClose ( &bzerr, bzf );
      if (bzerr != BZ_OK) panic ( "test:bzReadGetUnused" );
      if (nUnused == 0 && myfeof(zStream)) break;

   }

   if (ferror(zStream)) goto errhandler_io;
   ret = fclose ( zStream );
   if (ret == EOF) goto errhandler_io;

   if (verbosity >= 2) fprintf ( stderr, "\n    " );
   return True;

   errhandler:
   BZ2_bzReadClose ( &bzerr_dummy, bzf );
   if (verbosity == 0)
      fprintf ( stderr, "%s: %s: ", progName, inName );
   switch (bzerr) {
      case BZ_CONFIG_ERROR:
         configError(); break;
      case BZ_IO_ERROR:
         errhandler_io:
         ioError(); break;
      case BZ_DATA_ERROR:
         fprintf ( stderr,
                   "data integrity (CRC) error in data\n" );
         return False;
      case BZ_MEM_ERROR:
         outOfMemory();
      case BZ_UNEXPECTED_EOF:
         fprintf ( stderr,
                   "file ends unexpectedly\n" );
         return False;
      case BZ_DATA_ERROR_MAGIC:
         if (zStream != stdin) fclose(zStream);
         if (streamNo == 1) {
          fprintf ( stderr,
                    "bad magic number (file not created by bzip2)\n" );
            return False;
         } else {
            if (noisy)
            fprintf ( stderr,
                      "trailing garbage after EOF ignored\n" );
            return True;
         }
      default:
         panic ( "test:unexpected error" );
   }

   panic ( "test:end" );
   return True; /*notreached*/
}


/*---------------------------------------------------*/
/*--- Error [non-] handling grunge                ---*/
/*---------------------------------------------------*/

/*---------------------------------------------*/
static
void setExit ( Int32 v )
{
   if (v > exitValue) exitValue = v;
}


/*---------------------------------------------*/
static
void cadvise ( void )
{
   if (noisy)
   fprintf (
      stderr,
      "\nIt is possible that the compressed file(s) have become corrupted.\n"
        "You can use the -tvv option to test integrity of such files.\n\n"
        "You can use the `bzip2recover' program to attempt to recover\n"
        "data from undamaged sections of corrupted files.\n\n"
    );
}


/*---------------------------------------------*/
static
void showFileNames ( void )
{
   if (noisy)
   fprintf (
      stderr,
      "\tInput file = %s, output file = %s\n",
      inName, outName
   );
}


/*---------------------------------------------*/
static
void cleanUpAndFail ( Int32 ec )
{
   IntNative      retVal;
   struct MY_STAT statBuf;

   if ( srcMode == SM_F2F
        && opMode != OM_TEST
        && deleteOutputOnInterrupt ) {

      /* Check whether input file still exists.  Delete output file
         only if input exists to avoid loss of data.  Joerg Prante, 5
         January 2002.  (JRS 06-Jan-2002: other changes in 1.0.2 mean
         this is less likely to happen.  But to be ultra-paranoid, we
         do the check anyway.)  */
      retVal = MY_STAT ( inName, &statBuf );
      if (retVal == 0) {
         if (noisy)
            fprintf ( stderr,
                      "%s: Deleting output file %s, if it exists.\n",
                      progName, outName );
         if (outputHandleJustInCase != NULL)
            fclose ( outputHandleJustInCase );
         retVal = remove ( outName );
         if (retVal != 0)
            fprintf ( stderr,
                      "%s: WARNING: deletion of output file "
                      "(apparently) failed.\n",
                      progName );
      } else {
         fprintf ( stderr,
                   "%s: WARNING: deletion of output file suppressed\n",
                    progName );
         fprintf ( stderr,
                   "%s:    since input file no longer exists.  Output file\n",
                   progName );
         fprintf ( stderr,
                   "%s:    `%s' may be incomplete.\n",
                   progName, outName );
         fprintf ( stderr,
                   "%s:    I suggest doing an integrity test (bzip2 -tv)"
                   " of it.\n",
                   progName );
      }
   }

   if (noisy && numFileNames > 0 && numFilesProcessed < numFileNames) {
      fprintf ( stderr,
                "%s: WARNING: some files have not been processed:\n"
                "%s:    %d specified on command line, %d not processed yet.\n\n",
                progName, progName,
                numFileNames, numFileNames - numFilesProcessed );
   }
   setExit(ec);
   exit(exitValue);
}


/*---------------------------------------------*/
static
void panic ( const Char* s )
{
   fprintf ( stderr,
             "\n%s: PANIC -- internal consistency error:\n"
             "\t%s\n"
             "\tThis is a BUG.  Please report it to me at:\n"
             "\tjseward@bzip.org\n",
             progName, s );
   showFileNames();
   cleanUpAndFail( 3 );
}


/*---------------------------------------------*/
static
void crcError ( void )
{
   fprintf ( stderr,
             "\n%s: Data integrity error when decompressing.\n",
             progName );
   showFileNames();
   cadvise();
   cleanUpAndFail( 2 );
}


/*---------------------------------------------*/
static
void compressedStreamEOF ( void )
{
  if (noisy) {
    fprintf ( stderr,
	      "\n%s: Compressed file ends unexpectedly;\n\t"
	      "perhaps it is corrupted?  *Possible* reason follows.\n",
	      progName );
    perror ( progName );
    showFileNames();
    cadvise();
  }
  cleanUpAndFail( 2 );
}


/*---------------------------------------------*/
static
void ioError ( void )
{
   fprintf ( stderr,
             "\n%s: I/O or other error, bailing out.  "
             "Possible reason follows.\n",
             progName );
   perror ( progName );
   showFileNames();
   cleanUpAndFail( 1 );
}


/*---------------------------------------------*/
static
void mySignalCatcher ( IntNative n )
{
   fprintf ( stderr,
             "\n%s: Control-C or similar caught, quitting.\n",
             progName );
   cleanUpAndFail(1);
}


/*---------------------------------------------*/
static
void mySIGSEGVorSIGBUScatcher ( IntNative n )
{
   if (opMode == OM_Z)
      fprintf (
      stderr,
      "\n%s: Caught a SIGSEGV or SIGBUS whilst compressing.\n"
      "\n"
      "   Possible causes are (most likely first):\n"
      "   (1) This computer has unreliable memory or cache hardware\n"
      "       (a surprisingly common problem; try a different machine.)\n"
      "   (2) A bug in the compiler used to create this executable\n"
      "       (unlikely, if you didn't compile bzip2 yourself.)\n"
      "   (3) A real bug in bzip2 -- I hope this should never be the case.\n"
      "   The user's manual, Section 4.3, has more info on (1) and (2).\n"
      "   \n"
      "   If you suspect this is a bug in bzip2, or are unsure about (1)\n"
      "   or (2), feel free to report it to me at: jseward@bzip.org.\n"
      "   Section 4.3 of the user's manual describes the info a useful\n"
      "   bug report should have.  If the manual is available on your\n"
      "   system, please try and read it before mailing me.  If you don't\n"
      "   have the manual or can't be bothered to read it, mail me anyway.\n"
      "\n",
      progName );
      else
      fprintf (
      stderr,
      "\n%s: Caught a SIGSEGV or SIGBUS whilst decompressing.\n"
      "\n"
      "   Possible causes are (most likely first):\n"
      "   (1) The compressed data is corrupted, and bzip2's usual checks\n"
      "       failed to detect this.  Try bzip2 -tvv my_file.bz2.\n"
      "   (2) This computer has unreliable memory or cache hardware\n"
      "       (a surprisingly common problem; try a different machine.)\n"
      "   (3) A bug in the compiler used to create this executable\n"
      "       (unlikely, if you didn't compile bzip2 yourself.)\n"
      "   (4) A real bug in bzip2 -- I hope this should never be the case.\n"
      "   The user's manual, Section 4.3, has more info on (2) and (3).\n"
      "   \n"
      "   If you suspect this is a bug in bzip2, or are unsure about (2)\n"
      "   or (3), feel free to report it to me at: jseward@bzip.org.\n"
      "   Section 4.3 of the user's manual describes the info a useful\n"
      "   bug report should have.  If the manual is available on your\n"
      "   system, please try and read it before mailing me.  If you don't\n"
      "   have the manual or can't be bothered to read it, mail me anyway.\n"
      "\n",
      progName );

   showFileNames();
   if (opMode == OM_Z)
      cleanUpAndFail( 3 ); else
      { cadvise(); cleanUpAndFail( 2 ); }
}


/*---------------------------------------------*/
static
void outOfMemory ( void )
{
   fprintf ( stderr,
             "\n%s: couldn't allocate enough memory\n",
             progName );
   showFileNames();
   cleanUpAndFail(1);
}


/*---------------------------------------------*/
static
void configError ( void )
{
   fprintf ( stderr,
             "bzip2: I'm not configured correctly for this platform!\n"
             "\tI require Int32, Int16 and Char to have sizes\n"
             "\tof 4, 2 and 1 bytes to run properly, and they don't.\n"
             "\tProbably you can fix this by defining them correctly,\n"
             "\tand recompiling.  Bye!\n" );
   setExit(3);
   exit(exitValue);
}


/*---------------------------------------------------*/
/*--- The main driver machinery                   ---*/
/*---------------------------------------------------*/

/* All rather crufty.  The main problem is that input files
   are stat()d multiple times before use.  This should be
   cleaned up.
*/

/*---------------------------------------------*/
static
void pad ( Char *s )
{
   Int32 i;
   if ( (Int32)strlen(s) >= longestFileName ) return;
   for (i = 1; i <= longestFileName - (Int32)strlen(s); i++)
      fprintf ( stderr, " " );
}


/*---------------------------------------------*/
static
void copyFileName ( Char* to, Char* from )
{
   if ( strlen(from) > FILE_NAME_LEN-10 )  {
      fprintf (
         stderr,
         "bzip2: file name\n`%s'\n"
         "is suspiciously (more than %d chars) long.\n"
         "Try using a reasonable file name instead.  Sorry! :-)\n",
         from, FILE_NAME_LEN-10
      );
      setExit(1);
      exit(exitValue);
   }

  strncpy(to,from,FILE_NAME_LEN-10);
  to[FILE_NAME_LEN-10]='\0';
}


/*---------------------------------------------*/
static
Bool fileExists ( Char* name )
{
   FILE *tmp   = fopen ( name, "rb" );
   Bool exists = (tmp != NULL);
   if (tmp != NULL) fclose ( tmp );
   return exists;
}


/*---------------------------------------------*/
/* Open an output file safely with O_EXCL and good permissions.
   This avoids a race condition in versions < 1.0.2, in which
   the file was first opened and then had its interim permissions
   set safely.  We instead use open() to create the file with
   the interim permissions required. (--- --- rw-).

   For non-Unix platforms, if we are not worrying about
   security issues, simple this simply behaves like fopen.
*/
static
FILE* fopen_output_safely ( Char* name, const char* mode )
{
#  if BZ_UNIX
   FILE*     fp;
   IntNative fh;
   fh = open(name, O_WRONLY|O_CREAT|O_EXCL, S_IWUSR|S_IRUSR);
   if (fh == -1) return NULL;
   fp = fdopen(fh, mode);
   if (fp == NULL) close(fh);
   return fp;
#  else
   return fopen(name, mode);
#  endif
}


/*---------------------------------------------*/
/*--
  if in doubt, return True
--*/
static
Bool notAStandardFile ( Char* name )
{
   IntNative      i;
   struct MY_STAT statBuf;

   i = MY_LSTAT ( name, &statBuf );
   if (i != 0) return True;
   if (MY_S_ISREG(statBuf.st_mode)) return False;
   return True;
}


/*---------------------------------------------*/
/*--
  rac 11/21/98 see if file has hard links to it
--*/
static
Int32 countHardLinks ( Char* name )
{
   IntNative      i;
   struct MY_STAT statBuf;

   i = MY_LSTAT ( name, &statBuf );
   if (i != 0) return 0;
   return (statBuf.st_nlink - 1);
}


/*---------------------------------------------*/
/* Copy modification date, access date, permissions and owner from the
   source to destination file.  We have to copy this meta-info off
   into fileMetaInfo before starting to compress / decompress it,
   because doing it afterwards means we get the wrong access time.

   To complicate matters, in compress() and decompress() below, the
   sequence of tests preceding the call to saveInputFileMetaInfo()
   involves calling fileExists(), which in turn establishes its result
   by attempting to fopen() the file, and if successful, immediately
   fclose()ing it again.  So we have to assume that the fopen() call
   does not cause the access time field to be updated.

   Reading of the man page for stat() (man 2 stat) on RedHat 7.2 seems
   to imply that merely doing open() will not affect the access time.
   Therefore we merely need to hope that the C library only does
   open() as a result of fopen(), and not any kind of read()-ahead
   cleverness.

   It sounds pretty fragile to me.  Whether this carries across
   robustly to arbitrary Unix-like platforms (or even works robustly
   on this one, RedHat 7.2) is unknown to me.  Nevertheless ...
*/
#if BZ_UNIX
static
struct MY_STAT fileMetaInfo;
#endif

static
void saveInputFileMetaInfo ( Char *srcName )
{
#  if BZ_UNIX
   IntNative retVal;
   /* Note use of stat here, not lstat. */
   retVal = MY_STAT( srcName, &fileMetaInfo );
   ERROR_IF_NOT_ZERO ( retVal );
#  endif
}


static
void applySavedTimeInfoToOutputFile ( Char *dstName )
{
#  if BZ_UNIX
   IntNative      retVal;
   struct utimbuf uTimBuf;

   uTimBuf.actime = fileMetaInfo.st_atime;
   uTimBuf.modtime = fileMetaInfo.st_mtime;

   retVal = utime ( dstName, &uTimBuf );
   ERROR_IF_NOT_ZERO ( retVal );
#  endif
}

static
void applySavedFileAttrToOutputFile ( IntNative fd )
{
#  if BZ_UNIX
   IntNative retVal;

   retVal = fchmod ( fd, fileMetaInfo.st_mode );
   ERROR_IF_NOT_ZERO ( retVal );

   (void) fchown ( fd, fileMetaInfo.st_uid, fileMetaInfo.st_gid );
   /* chown() will in many cases return with EPERM, which can
      be safely ignored.
   */
#  endif
}


/*---------------------------------------------*/
static
Bool containsDubiousChars ( Char* name )
{
#  if BZ_UNIX
   /* On unix, files can contain any characters and the file expansion
    * is performed by the shell.
    */
   return False;
#  else /* ! BZ_UNIX */
   /* On non-unix (Win* platforms), wildcard characters are not allowed in
    * filenames.
    */
   for (; *name != '\0'; name++)
      if (*name == '?' || *name == '*') return True;
   return False;
#  endif /* BZ_UNIX */
}


/*---------------------------------------------*/
#define BZ_N_SUFFIX_PAIRS 4

const Char* zSuffix[BZ_N_SUFFIX_PAIRS]
   = { ".bz2", ".bz", ".tbz2", ".tbz" };
const Char* unzSuffix[BZ_N_SUFFIX_PAIRS]
   = { "", "", ".tar", ".tar" };

static
Bool hasSuffix ( Char* s, const Char* suffix )
{
   Int32 ns = strlen(s);
   Int32 nx = strlen(suffix);
   if (ns < nx) return False;
   if (strcmp(s + ns - nx, suffix) == 0) return True;
   return False;
}

static
Bool mapSuffix ( Char* name,
                 const Char* oldSuffix,
                 const Char* newSuffix )
{
   if (!hasSuffix(name,oldSuffix)) return False;
   name[strlen(name)-strlen(oldSuffix)] = 0;
   strcat ( name, newSuffix );
   return True;
}


/*---------------------------------------------*/
static
void compress ( Char *name )
{
   FILE  *inStr;
   FILE  *outStr;
   Int32 n, i;
   struct MY_STAT statBuf;

   deleteOutputOnInterrupt = False;

   if (name == NULL && srcMode != SM_I2O)
      panic ( "compress: bad modes\n" );

   switch (srcMode) {
      case SM_I2O:
         copyFileName ( inName, (Char*)"(stdin)" );
         copyFileName ( outName, (Char*)"(stdout)" );
         break;
      case SM_F2F:
         copyFileName ( inName, name );
         copyFileName ( outName, name );
         strcat ( outName, ".bz2" );
         break;
      case SM_F2O:
         copyFileName ( inName, name );
         copyFileName ( outName, (Char*)"(stdout)" );
         break;
   }

   if ( srcMode != SM_I2O && containsDubiousChars ( inName ) ) {
      if (noisy)
      fprintf ( stderr, "%s: There are no files matching `%s'.\n",
                progName, inName );
      setExit(1);
      return;
   }
   if ( srcMode != SM_I2O && !fileExists ( inName ) ) {
      fprintf ( stderr, "%s: Can't open input file %s: %s.\n",
                progName, inName, strerror(errno) );
      setExit(1);
      return;
   }
   for (i = 0; i < BZ_N_SUFFIX_PAIRS; i++) {
      if (hasSuffix(inName, zSuffix[i])) {
         if (noisy)
         fprintf ( stderr,
                   "%s: Input file %s already has %s suffix.\n",
                   progName, inName, zSuffix[i] );
         setExit(1);
         return;
      }
   }
   if ( srcMode == SM_F2F || srcMode == SM_F2O ) {
      MY_STAT(inName, &statBuf);
      if ( MY_S_ISDIR(statBuf.st_mode) ) {
         fprintf( stderr,
                  "%s: Input file %s is a directory.\n",
                  progName,inName);
         setExit(1);
         return;
      }
   }
   if ( srcMode == SM_F2F && !forceOverwrite && notAStandardFile ( inName )) {
      if (noisy)
      fprintf ( stderr, "%s: Input file %s is not a normal file.\n",
                progName, inName );
      setExit(1);
      return;
   }
   if ( srcMode == SM_F2F && fileExists ( outName ) ) {
      if (forceOverwrite) {
	 remove(outName);
      } else {
	 fprintf ( stderr, "%s: Output file %s already exists.\n",
		   progName, outName );
	 setExit(1);
	 return;
      }
   }
   if ( srcMode == SM_F2F && !forceOverwrite &&
        (n=countHardLinks ( inName )) > 0) {
      fprintf ( stderr, "%s: Input file %s has %d other link%s.\n",
                progName, inName, n, n > 1 ? "s" : "" );
      setExit(1);
      return;
   }

   if ( srcMode == SM_F2F ) {
      /* Save the file's meta-info before we open it.  Doing it later
         means we mess up the access times. */
      saveInputFileMetaInfo ( inName );
   }

   switch ( srcMode ) {

      case SM_I2O:
         inStr = stdin;
         outStr = stdout;
         if ( isatty ( fileno ( stdout ) ) ) {
            fprintf ( stderr,
                      "%s: I won't write compressed data to a terminal.\n",
                      progName );
            fprintf ( stderr, "%s: For help, type: `%s --help'.\n",
                              progName, progName );
            setExit(1);
            return;
         };
         break;

      case SM_F2O:
         inStr = fopen ( inName, "rb" );
         outStr = stdout;
         if ( isatty ( fileno ( stdout ) ) ) {
            fprintf ( stderr,
                      "%s: I won't write compressed data to a terminal.\n",
                      progName );
            fprintf ( stderr, "%s: For help, type: `%s --help'.\n",
                              progName, progName );
            if ( inStr != NULL ) fclose ( inStr );
            setExit(1);
            return;
         };
         if ( inStr == NULL ) {
            fprintf ( stderr, "%s: Can't open input file %s: %s.\n",
                      progName, inName, strerror(errno) );
            setExit(1);
            return;
         };
         break;

      case SM_F2F:
         inStr = fopen ( inName, "rb" );
         outStr = fopen_output_safely ( outName, "wb" );
         if ( outStr == NULL) {
            fprintf ( stderr, "%s: Can't create output file %s: %s.\n",
                      progName, outName, strerror(errno) );
            if ( inStr != NULL ) fclose ( inStr );
            setExit(1);
            return;
         }
         if ( inStr == NULL ) {
            fprintf ( stderr, "%s: Can't open input file %s: %s.\n",
                      progName, inName, strerror(errno) );
            if ( outStr != NULL ) fclose ( outStr );
            setExit(1);
            return;
         };
         break;

      default:
         panic ( "compress: bad srcMode" );
         break;
   }

   if (verbosity >= 1) {
      fprintf ( stderr,  "  %s: ", inName );
      pad ( inName );
      fflush ( stderr );
   }

   /*--- Now the input and output handles are sane.  Do the Biz. ---*/
   outputHandleJustInCase = outStr;
   deleteOutputOnInterrupt = True;
   compressStream ( inStr, outStr );
   outputHandleJustInCase = NULL;

   /*--- If there was an I/O error, we won't get here. ---*/
   if ( srcMode == SM_F2F ) {
      applySavedTimeInfoToOutputFile ( outName );
      deleteOutputOnInterrupt = False;
      if ( !keepInputFiles ) {
         IntNative retVal = remove ( inName );
         ERROR_IF_NOT_ZERO ( retVal );
      }
   }

   deleteOutputOnInterrupt = False;
}


/*---------------------------------------------*/
static
void uncompress ( Char *name )
{
   FILE  *inStr;
   FILE  *outStr;
   Int32 n, i;
   Bool  magicNumberOK;
   Bool  cantGuess;
   struct MY_STAT statBuf;

   deleteOutputOnInterrupt = False;

   if (name == NULL && srcMode != SM_I2O)
      panic ( "uncompress: bad modes\n" );

   cantGuess = False;
   switch (srcMode) {
      case SM_I2O:
         copyFileName ( inName, (Char*)"(stdin)" );
         copyFileName ( outName, (Char*)"(stdout)" );
         break;
      case SM_F2F:
         copyFileName ( inName, name );
         copyFileName ( outName, name );
         for (i = 0; i < BZ_N_SUFFIX_PAIRS; i++)
            if (mapSuffix(outName,zSuffix[i],unzSuffix[i]))
               goto zzz;
         cantGuess = True;
         strcat ( outName, ".out" );
         break;
      case SM_F2O:
         copyFileName ( inName, name );
         copyFileName ( outName, (Char*)"(stdout)" );
         break;
   }

   zzz:
   if ( srcMode != SM_I2O && containsDubiousChars ( inName ) ) {
      if (noisy)
      fprintf ( stderr, "%s: There are no files matching `%s'.\n",
                progName, inName );
      setExit(1);
      return;
   }
   if ( srcMode != SM_I2O && !fileExists ( inName ) ) {
      fprintf ( stderr, "%s: Can't open input file %s: %s.\n",
                progName, inName, strerror(errno) );
      setExit(1);
      return;
   }
   if ( srcMode == SM_F2F || srcMode == SM_F2O ) {
      MY_STAT(inName, &statBuf);
      if ( MY_S_ISDIR(statBuf.st_mode) ) {
         fprintf( stderr,
                  "%s: Input file %s is a directory.\n",
                  progName,inName);
         setExit(1);
         return;
      }
   }
   if ( srcMode == SM_F2F && !forceOverwrite && notAStandardFile ( inName )) {
      if (noisy)
      fprintf ( stderr, "%s: Input file %s is not a normal file.\n",
                progName, inName );
      setExit(1);
      return;
   }
   if ( /* srcMode == SM_F2F implied && */ cantGuess ) {
      if (noisy)
      fprintf ( stderr,
                "%s: Can't guess original name for %s -- using %s\n",
                progName, inName, outName );
      /* just a warning, no return */
   }
   if ( srcMode == SM_F2F && fileExists ( outName ) ) {
      if (forceOverwrite) {
	remove(outName);
      } else {
        fprintf ( stderr, "%s: Output file %s already exists.\n",
                  progName, outName );
        setExit(1);
        return;
      }
   }
   if ( srcMode == SM_F2F && !forceOverwrite &&
        (n=countHardLinks ( inName ) ) > 0) {
      fprintf ( stderr, "%s: Input file %s has %d other link%s.\n",
                progName, inName, n, n > 1 ? "s" : "" );
      setExit(1);
      return;
   }

   if ( srcMode == SM_F2F ) {
      /* Save the file's meta-info before we open it.  Doing it later
         means we mess up the access times. */
      saveInputFileMetaInfo ( inName );
   }

   switch ( srcMode ) {

      case SM_I2O:
         inStr = stdin;
         outStr = stdout;
         if ( isatty ( fileno ( stdin ) ) ) {
            fprintf ( stderr,
                      "%s: I won't read compressed data from a terminal.\n",
                      progName );
            fprintf ( stderr, "%s: For help, type: `%s --help'.\n",
                              progName, progName );
            setExit(1);
            return;
         };
         break;

      case SM_F2O:
         inStr = fopen ( inName, "rb" );
         outStr = stdout;
         if ( inStr == NULL ) {
            fprintf ( stderr, "%s: Can't open input file %s:%s.\n",
                      progName, inName, strerror(errno) );
            if ( inStr != NULL ) fclose ( inStr );
            setExit(1);
            return;
         };
         break;

      case SM_F2F:
         inStr = fopen ( inName, "rb" );
         outStr = fopen_output_safely ( outName, "wb" );
         if ( outStr == NULL) {
            fprintf ( stderr, "%s: Can't create output file %s: %s.\n",
                      progName, outName, strerror(errno) );
            if ( inStr != NULL ) fclose ( inStr );
            setExit(1);
            return;
         }
         if ( inStr == NULL ) {
            fprintf ( stderr, "%s: Can't open input file %s: %s.\n",
                      progName, inName, strerror(errno) );
            if ( outStr != NULL ) fclose ( outStr );
            setExit(1);
            return;
         };
         break;

      default:
         panic ( "uncompress: bad srcMode" );
         break;
   }

   if (verbosity >= 1) {
      fprintf ( stderr, "  %s: ", inName );
      pad ( inName );
      fflush ( stderr );
   }

   /*--- Now the input and output handles are sane.  Do the Biz. ---*/
   outputHandleJustInCase = outStr;
   deleteOutputOnInterrupt = True;
   magicNumberOK = uncompressStream ( inStr, outStr );
   outputHandleJustInCase = NULL;

   /*--- If there was an I/O error, we won't get here. ---*/
   if ( magicNumberOK ) {
      if ( srcMode == SM_F2F ) {
         applySavedTimeInfoToOutputFile ( outName );
         deleteOutputOnInterrupt = False;
         if ( !keepInputFiles ) {
            IntNative retVal = remove ( inName );
            ERROR_IF_NOT_ZERO ( retVal );
         }
      }
   } else {
      unzFailsExist = True;
      deleteOutputOnInterrupt = False;
      if ( srcMode == SM_F2F ) {
         IntNative retVal = remove ( outName );
         ERROR_IF_NOT_ZERO ( retVal );
      }
   }
   deleteOutputOnInterrupt = False;

   if ( magicNumberOK ) {
      if (verbosity >= 1)
         fprintf ( stderr, "done\n" );
   } else {
      setExit(2);
      if (verbosity >= 1)
         fprintf ( stderr, "not a bzip2 file.\n" ); else
         fprintf ( stderr,
                   "%s: %s is not a bzip2 file.\n",
                   progName, inName );
   }

}


/*---------------------------------------------*/
static
void testf ( Char *name )
{
   FILE *inStr;
   Bool allOK;
   struct MY_STAT statBuf;

   deleteOutputOnInterrupt = False;

   if (name == NULL && srcMode != SM_I2O)
      panic ( "testf: bad modes\n" );

   copyFileName ( outName, (Char*)"(none)" );
   switch (srcMode) {
      case SM_I2O: copyFileName ( inName, (Char*)"(stdin)" ); break;
      case SM_F2F: copyFileName ( inName, name ); break;
      case SM_F2O: copyFileName ( inName, name ); break;
   }

   if ( srcMode != SM_I2O && containsDubiousChars ( inName ) ) {
      if (noisy)
      fprintf ( stderr, "%s: There are no files matching `%s'.\n",
                progName, inName );
      setExit(1);
      return;
   }
   if ( srcMode != SM_I2O && !fileExists ( inName ) ) {
      fprintf ( stderr, "%s: Can't open input %s: %s.\n",
                progName, inName, strerror(errno) );
      setExit(1);
      return;
   }
   if ( srcMode != SM_I2O ) {
      MY_STAT(inName, &statBuf);
      if ( MY_S_ISDIR(statBuf.st_mode) ) {
         fprintf( stderr,
                  "%s: Input file %s is a directory.\n",
                  progName,inName);
         setExit(1);
         return;
      }
   }

   switch ( srcMode ) {

      case SM_I2O:
         if ( isatty ( fileno ( stdin ) ) ) {
            fprintf ( stderr,
                      "%s: I won't read compressed data from a terminal.\n",
                      progName );
            fprintf ( stderr, "%s: For help, type: `%s --help'.\n",
                              progName, progName );
            setExit(1);
            return;
         };
         inStr = stdin;
         break;

      case SM_F2O: case SM_F2F:
         inStr = fopen ( inName, "rb" );
         if ( inStr == NULL ) {
            fprintf ( stderr, "%s: Can't open input file %s:%s.\n",
                      progName, inName, strerror(errno) );
            setExit(1);
            return;
         };
         break;

      default:
         panic ( "testf: bad srcMode" );
         break;
   }

   if (verbosity >= 1) {
      fprintf ( stderr, "  %s: ", inName );
      pad ( inName );
      fflush ( stderr );
   }

   /*--- Now the input handle is sane.  Do the Biz. ---*/
   outputHandleJustInCase = NULL;
   allOK = testStream ( inStr );

   if (allOK && verbosity >= 1) fprintf ( stderr, "ok\n" );
   if (!allOK) testFailsExist = True;
}


/*---------------------------------------------*/
static
void license ( void )
{
   fprintf ( stderr,

    "bzip2, a block-sorting file compressor.  "
    "Version %s.\n"
    "   \n"
    "   Copyright (C) 1996-2010 by Julian Seward.\n"
    "   \n"
    "   This program is free software; you can redistribute it and/or modify\n"
    "   it under the terms set out in the LICENSE file, which is included\n"
    "   in the bzip2-1.0.6 source distribution.\n"
    "   \n"
    "   This program is distributed in the hope that it will be useful,\n"
    "   but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "   LICENSE file for more details.\n"
    "   \n",
    BZ2_bzlibVersion()
   );
}


/*---------------------------------------------*/
static
void usage ( Char *fullProgName )
{
   fprintf (
      stderr,
      "bzip2, a block-sorting file compressor.  "
      "Version %s.\n"
      "\n   usage: %s [flags and input files in any order]\n"
      "\n"
      "   -h --help           print this message\n"
      "   -d --decompress     force decompression\n"
      "   -z --compress       force compression\n"
      "   -k --keep           keep (don't delete) input files\n"
      "   -f --force          overwrite existing output files\n"
      "   -t --test           test compressed file integrity\n"
      "   -c --stdout         output to standard out\n"
      "   -q --quiet          suppress noncritical error messages\n"
      "   -v --verbose        be verbose (a 2nd -v gives more)\n"
      "   -L --license        display software version & license\n"
      "   -V --version        display software version & license\n"
      "   -s --small          use less memory (at most 2500k)\n"
      "   -1 .. -9            set block size to 100k .. 900k\n"
      "   --fast              alias for -1\n"
      "   --best              alias for -9\n"
      "\n"
      "   If invoked as `bzip2', default action is to compress.\n"
      "              as `bunzip2',  default action is to decompress.\n"
      "              as `bzcat', default action is to decompress to stdout.\n"
      "\n"
      "   If no file names are given, bzip2 compresses or decompresses\n"
      "   from standard input to standard output.  You can combine\n"
      "   short flags, so `-v -4' means the same as -v4 or -4v, &c.\n"
#     if BZ_UNIX
      "\n"
#     endif
      ,

      BZ2_bzlibVersion(),
      fullProgName
   );
}


/*---------------------------------------------*/
static
void redundant ( Char* flag )
{
   fprintf (
      stderr,
      "%s: %s is redundant in versions 0.9.5 and above\n",
      progName, flag );
}


/*---------------------------------------------*/
/*--
  All the garbage from here to main() is purely to
  implement a linked list of command-line arguments,
  into which main() copies argv[1 .. argc-1].

  The purpose of this exercise is to facilitate
  the expansion of wildcard characters * and ? in
  filenames for OSs which don't know how to do it
  themselves, like MSDOS, Windows 95 and NT.

  The actual Dirty Work is done by the platform-
  specific macro APPEND_FILESPEC.
--*/

typedef
   struct zzzz {
      Char        *name;
      struct zzzz *link;
   }
   Cell;


/*---------------------------------------------*/
static
void *myMalloc ( Int32 n )
{
   void* p;

   p = malloc ( (size_t)n );
   if (p == NULL) outOfMemory ();
   return p;
}


/*---------------------------------------------*/
static
Cell *mkCell ( void )
{
   Cell *c;

   c = (Cell*) myMalloc ( sizeof ( Cell ) );
   c->name = NULL;
   c->link = NULL;
   return c;
}


/*---------------------------------------------*/
static
Cell *snocString ( Cell *root, Char *name )
{
   if (root == NULL) {
      Cell *tmp = mkCell();
      tmp->name = (Char*) myMalloc ( 5 + strlen(name) );
      strcpy ( tmp->name, name );
      return tmp;
   } else {
      Cell *tmp = root;
      while (tmp->link != NULL) tmp = tmp->link;
      tmp->link = snocString ( tmp->link, name );
      return root;
   }
}


/*---------------------------------------------*/
static
void addFlagsFromEnvVar ( Cell** argList, Char* varName )
{
   Int32 i, j, k;
   Char *envbase, *p;

   envbase = getenv(varName);
   if (envbase != NULL) {
      p = envbase;
      i = 0;
      while (True) {
         if (p[i] == 0) break;
         p += i;
         i = 0;
         while (isspace((Int32)(p[0]))) p++;
         while (p[i] != 0 && !isspace((Int32)(p[i]))) i++;
         if (i > 0) {
            k = i; if (k > FILE_NAME_LEN-10) k = FILE_NAME_LEN-10;
            for (j = 0; j < k; j++) tmpName[j] = p[j];
            tmpName[k] = 0;
            APPEND_FLAG(*argList, tmpName);
         }
      }
   }
}


/*---------------------------------------------*/
#define ISFLAG(s) (strcmp(aa->name, (s))==0)

IntNative main ( IntNative argc, Char *argv[] )
{
   Int32  i, j;
   Char   *tmp;
   Cell   *argList;
   Cell   *aa;
   Bool   decode;

   /*-- Be really really really paranoid :-) --*/
   if (sizeof(Int32) != 4 || sizeof(UInt32) != 4  ||
       sizeof(Int16) != 2 || sizeof(UInt16) != 2  ||
       sizeof(Char)  != 1 || sizeof(UChar)  != 1)
      configError();

   /*-- Initialise --*/
   outputHandleJustInCase  = NULL;
   smallMode               = False;
   keepInputFiles          = False;
   forceOverwrite          = False;
   noisy                   = True;
   verbosity               = 0;
   blockSize100k           = 9;
   testFailsExist          = False;
   unzFailsExist           = False;
   numFileNames            = 0;
   numFilesProcessed       = 0;
   workFactor              = 30;
   deleteOutputOnInterrupt = False;
   exitValue               = 0;
   i = j = 0; /* avoid bogus warning from egcs-1.1.X */

   /*-- Set up signal handlers for mem access errors --*/
   signal (SIGSEGV, mySIGSEGVorSIGBUScatcher);
#  if BZ_UNIX
#  ifndef __DJGPP__
   signal (SIGBUS,  mySIGSEGVorSIGBUScatcher);
#  endif
#  endif

   copyFileName ( inName,  (Char*)"(none)" );
   copyFileName ( outName, (Char*)"(none)" );

   copyFileName ( progNameReally, argv[0] );
   progName = &progNameReally[0];
   for (tmp = &progNameReally[0]; *tmp != '\0'; tmp++)
      if (*tmp == PATH_SEP) progName = tmp + 1;


   /*-- Copy flags from env var BZIP2, and
        expand filename wildcards in arg list.
   --*/
   argList = NULL;
   addFlagsFromEnvVar ( &argList,  (Char*)"BZIP2" );
   addFlagsFromEnvVar ( &argList,  (Char*)"BZIP" );
   for (i = 1; i <= argc-1; i++)
      APPEND_FILESPEC(argList, argv[i]);


   /*-- Find the length of the longest filename --*/
   longestFileName = 7;
   numFileNames    = 0;
   decode          = True;
   for (aa = argList; aa != NULL; aa = aa->link) {
      if (ISFLAG("--")) { decode = False; continue; }
      if (aa->name[0] == '-' && decode) continue;
      numFileNames++;
      if (longestFileName < (Int32)strlen(aa->name) )
         longestFileName = (Int32)strlen(aa->name);
   }


   /*-- Determine source modes; flag handling may change this too. --*/
   if (numFileNames == 0)
      srcMode = SM_I2O; else srcMode = SM_F2F;


   /*-- Determine what to do (compress/uncompress/test/cat). --*/
   /*-- Note that subsequent flag handling may change this. --*/
   opMode = OM_Z;

   if ( (strstr ( progName, "unzip" ) != 0) ||
        (strstr ( progName, "UNZIP" ) != 0) )
      opMode = OM_UNZ;

   if ( (strstr ( progName, "z2cat" ) != 0) ||
        (strstr ( progName, "Z2CAT" ) != 0) ||
        (strstr ( progName, "zcat" ) != 0)  ||
        (strstr ( progName, "ZCAT" ) != 0) )  {
      opMode = OM_UNZ;
      srcMode = (numFileNames == 0) ? SM_I2O : SM_F2O;
   }


   /*-- Look at the flags. --*/
   for (aa = argList; aa != NULL; aa = aa->link) {
      if (ISFLAG("--")) break;
      if (aa->name[0] == '-' && aa->name[1] != '-') {
         for (j = 1; aa->name[j] != '\0'; j++) {
            switch (aa->name[j]) {
               case 'c': srcMode          = SM_F2O; break;
               case 'd': opMode           = OM_UNZ; break;
               case 'z': opMode           = OM_Z; break;
               case 'f': forceOverwrite   = True; break;
               case 't': opMode           = OM_TEST; break;
               case 'k': keepInputFiles   = True; break;
               case 's': smallMode        = True; break;
               case 'q': noisy            = False; break;
               case '1': blockSize100k    = 1; break;
               case '2': blockSize100k    = 2; break;
               case '3': blockSize100k    = 3; break;
               case '4': blockSize100k    = 4; break;
               case '5': blockSize100k    = 5; break;
               case '6': blockSize100k    = 6; break;
               case '7': blockSize100k    = 7; break;
               case '8': blockSize100k    = 8; break;
               case '9': blockSize100k    = 9; break;
               case 'V':
               case 'L': license();            break;
               case 'v': verbosity++; break;
               case 'h': usage ( progName );
                         exit ( 0 );
                         break;
               default:  fprintf ( stderr, "%s: Bad flag `%s'\n",
                                   progName, aa->name );
                         usage ( progName );
                         exit ( 1 );
                         break;
            }
         }
      }
   }

   /*-- And again ... --*/
   for (aa = argList; aa != NULL; aa = aa->link) {
      if (ISFLAG("--")) break;
      if (ISFLAG("--stdout"))            srcMode          = SM_F2O;  else
      if (ISFLAG("--decompress"))        opMode           = OM_UNZ;  else
      if (ISFLAG("--compress"))          opMode           = OM_Z;    else
      if (ISFLAG("--force"))             forceOverwrite   = True;    else
      if (ISFLAG("--test"))              opMode           = OM_TEST; else
      if (ISFLAG("--keep"))              keepInputFiles   = True;    else
      if (ISFLAG("--small"))             smallMode        = True;    else
      if (ISFLAG("--quiet"))             noisy            = False;   else
      if (ISFLAG("--version"))           license();                  else
      if (ISFLAG("--license"))           license();                  else
      if (ISFLAG("--exponential"))       workFactor = 1;             else
      if (ISFLAG("--repetitive-best"))   redundant(aa->name);        else
      if (ISFLAG("--repetitive-fast"))   redundant(aa->name);        else
      if (ISFLAG("--fast"))              blockSize100k = 1;          else
      if (ISFLAG("--best"))              blockSize100k = 9;          else
      if (ISFLAG("--verbose"))           verbosity++;                else
      if (ISFLAG("--help"))              { usage ( progName ); exit ( 0 ); }
         else
         if (strncmp ( aa->name, "--", 2) == 0) {
            fprintf ( stderr, "%s: Bad flag `%s'\n", progName, aa->name );
            usage ( progName );
            exit ( 1 );
         }
   }

   if (verbosity > 4) verbosity = 4;
   if (opMode == OM_Z && smallMode && blockSize100k > 2)
      blockSize100k = 2;

   if (opMode == OM_TEST && srcMode == SM_F2O) {
      fprintf ( stderr, "%s: -c and -t cannot be used together.\n",
                progName );
      exit ( 1 );
   }

   if (srcMode == SM_F2O && numFileNames == 0)
      srcMode = SM_I2O;

   if (opMode != OM_Z) blockSize100k = 0;

   if (srcMode == SM_F2F) {
      signal (SIGINT,  mySignalCatcher);
      signal (SIGTERM, mySignalCatcher);
#     if BZ_UNIX
      signal (SIGHUP,  mySignalCatcher);
#     endif
   }

   if (opMode == OM_Z) {
     if (srcMode == SM_I2O) {
        compress ( NULL );
     } else {
        decode = True;
        for (aa = argList; aa != NULL; aa = aa->link) {
           if (ISFLAG("--")) { decode = False; continue; }
           if (aa->name[0] == '-' && decode) continue;
           numFilesProcessed++;
           compress ( aa->name );
        }
     }
   }
   else

   if (opMode == OM_UNZ) {
      unzFailsExist = False;
      if (srcMode == SM_I2O) {
         uncompress ( NULL );
      } else {
         decode = True;
         for (aa = argList; aa != NULL; aa = aa->link) {
            if (ISFLAG("--")) { decode = False; continue; }
            if (aa->name[0] == '-' && decode) continue;
            numFilesProcessed++;
            uncompress ( aa->name );
         }
      }
      if (unzFailsExist) {
         setExit(2);
         exit(exitValue);
      }
   }

   else {
      testFailsExist = False;
      if (srcMode == SM_I2O) {
         testf ( NULL );
      } else {
         decode = True;
         for (aa = argList; aa != NULL; aa = aa->link) {
	    if (ISFLAG("--")) { decode = False; continue; }
            if (aa->name[0] == '-' && decode) continue;
            numFilesProcessed++;
            testf ( aa->name );
	 }
      }
      if (testFailsExist && noisy) {
         fprintf ( stderr,
           "\n"
           "You can use the `bzip2recover' program to attempt to recover\n"
           "data from undamaged sections of corrupted files.\n\n"
         );
         setExit(2);
         exit(exitValue);
      }
   }

   /* Free the argument list memory to mollify leak detectors
      (eg) Purify, Checker.  Serves no other useful purpose.
   */
   aa = argList;
   while (aa != NULL) {
      Cell* aa2 = aa->link;
      if (aa->name != NULL) free(aa->name);
      free(aa);
      aa = aa2;
   }

   return exitValue;
}
