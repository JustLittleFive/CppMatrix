/*
 Copyright (c) 2022 SUSTech - JustLittleFive

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
// #pragma once

#include <malloc.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRAN_THR 64

#define ALLOC_ARR(n) (T*)calloc_align(sizeof(T) * 4, (n) * sizeof(T))

using namespace std;

void* calloc_align(size_t alignment_, size_t size_) {
  void* p = memalign(alignment_, size_);
  if (!p) {
    puts("Error: no more space for allocation.");
    abort();
  }
  memset(p, 0, size_);
  return p;
}

template <typename T>
void matadd(int m, int p, const T* a, int lda, const T* b, int ldb, T* c,
            int ldc) {
  int i, j;
  for (i = 0; i < m; i++) {
    for (j = 0; j < p; j++) {
      // hope Compiler will help to unroll the loop in O3 mode
      c[j + ldc * i] = (a[j + lda * i] + b[j + ldb * i]);
    }
  }
}

template <typename T>
void matsub(int m, int p, const T* a, int lda, const T* b, int ldb, T* c,
            int ldc) {
  int i, j;
  for (i = 0; i < m; i++) {
    for (j = 0; j < p; j++) {
      // hope Compiler will help to unroll the loop in O3 mode
      c[j + ldc * i] = (a[j + lda * i] - b[j + ldb * i]);
    }
  }
}

template <typename T>
void matcopy(int m, int p, const T* from, int ldf, T* to, int ldt) {
  for (int i = 0; i < m; i++) {
    if (p >= 512) {
      memcpy(to + i * ldt, from + i * ldf, p * sizeof(T));
    } else
      for (int j = 0; j < p; j++) {
        to[j + i * ldt] = from[j + i * ldf];
      }
  }
}

template <typename T>
void matmul_cache_opt(int m, int n, int p, const T* x, int ldx, const T* y,
                      int ldy, T* z, int ldz) {
  // i-k-j order matrix mul, optimize the cache hits.
  int i, j, k;
  // #pragma omp parallel for
  for (i = 0; i < m; i++) {
    for (k = 0; k < p; k++) {
      register T aik = x[i * ldx + k];
      for (j = 0; j <= n - 4; j += 4) {
        // loop unrolling
        register const T* yptr = y + (k * ldy + j);
        register T* zptr = z + (i * ldz + j);

        // move pointer instead of computing address,
        // which involves multiplication
        *zptr += aik * (*yptr);
        zptr++;
        yptr++;
        *zptr += aik * (*yptr);
        zptr++;
        yptr++;
        *zptr += aik * (*yptr);
        zptr++;
        yptr++;
        *zptr += aik * (*yptr);
        zptr++;
        yptr++;
      }
      switch (n - j) {
        case 3:
          z[i * ldz + j + 2] += aik * y[k * ldy + j + 2];
        case 2:
          z[i * ldz + j + 1] += aik * y[k * ldy + j + 1];
        case 1:
          z[i * ldz + j] += aik * y[k * ldy + j];
        case 0:
          break;
      }
    }
  }
}

template <typename T>
void matmul_strassen(int m, int n, int p, const T* a, int lda, const T* b,
                     int ldb, T* c, int ldc) {
  // when reach goal size just use matmul_cache_opt
  if (m <= STRAN_THR && n <= STRAN_THR && p <= STRAN_THR) {
    matmul_cache_opt(m, n, p, a, lda, b, ldb, c, ldc);
    return;
  }

  // get HALF of the input size.
  int hm = m / 2;
  int hn = n / 2;
  int hp = p / 2;

  // padding flag on each dimension
  int padding_m = (m % 2);
  int padding_n = (n % 2);
  int padding_p = (p % 2);

  // the size of matrix after padding,
  // (m % 2 == 1 means m is odd and padding is needed.)
  // which also is the matrix size involves matrix mul.
  int phm = hm + padding_m;
  int phn = hn + padding_n;
  int php = hp + padding_p;

  // intermidiate matrices
  T* lhs = ALLOC_ARR(phm * php);  // left-hand-side
  T* rhs = ALLOC_ARR(php * phn);  // right-hand-side
  T* l;
  T* r;

  T* m1 = ALLOC_ARR(phm * phn);
  T* m2 = ALLOC_ARR(phm * phn);
  T* m3 = ALLOC_ARR(phm * phn);
  T* m4 = ALLOC_ARR(phm * phn);
  T* m5 = ALLOC_ARR(phm * phn);
  T* m6 = ALLOC_ARR(phm * phn);
  T* m7 = ALLOC_ARR(phm * phn);

#define A11 (a)
#define A12 (a + php)
#define A21 (a + phm * lda)
#define A22 (a + php + phm * lda)
#define B11 (b)
#define B12 (b + phn)
#define B21 (b + php * ldb)
#define B22 (b + phn + php * ldb)
#define C11 (c)
#define C12 (c + phn)
#define C21 (c + phm * ldc)
#define C22 (c + phn + phm * ldc)

  // implicit padding, avoid extra memory overhead
  // M2 = (A11+A12) * B22
  matadd(phm, hp, A11, lda, A12, lda, lhs, php);
  matcopy(hp, hn, B22, ldb, rhs, phn);
  if (padding_p) matcopy(phm, 1, A11 + hp, lda, lhs + hp, php);
  matmul_strassen(phm, phn, php, lhs, php, rhs, phn, m2, phn);

  // M1 = A11*(B12-B22)
  matcopy(phm, php, A11, lda, lhs, php);
  matsub(hp, hn, B12, ldb, B22, ldb, rhs, phn);
  if (padding_p) matcopy(1, hn, B12 + hp * ldb, ldb, rhs + hp * phn, phn);
  matmul_strassen(phm, phn, php, lhs, php, rhs, phn, m1, phn);

  // M3 = (A21+A22) * B11
  matadd(hm, hp, A21, lda, A22, lda, lhs, php);
  matcopy(php, phn, B11, ldb, rhs, phn);
  if (padding_p) matcopy(hm, 1, A21 + hp, lda, lhs + hp, php);
  if (padding_m) memset(lhs + hm * php, 0, sizeof(T) * php);
  matmul_strassen(phm, phn, php, lhs, php, rhs, phn, m3, phn);

  // M4 = A22 * (B21-B11)
  if (padding_m || padding_p) memset(lhs, 0, sizeof(T) * (phn * php));
  matcopy(hm, hp, A22, lda, lhs, php);
  matsub(hp, phn, B11, ldb, B21, ldb, rhs, phn);
  if (padding_p) matcopy(1, phn, B11 + hp * ldb, ldb, rhs + hp * phn, phn);
  matmul_strassen(phm, phn, php, lhs, php, rhs, phn, m4, phn);

  // M5 = (A11+A22) * (B11+B22)
  if (padding_m || padding_p) {
    matcopy(phm, php, A11, lda, lhs, php);
    matcopy(php, phn, B11, ldb, rhs, phn);
  }
  matadd(hm, hp, A11, lda, A22, lda, lhs, php);
  matadd(hp, hn, B11, ldb, B22, ldb, rhs, phn);

  matmul_strassen(phm, phn, php, lhs, php, rhs, phn, m5, phn);

  // M6 = (A12-A22) * (B21+B22)
  matsub(hm, hp, A12, lda, A22, lda, lhs, php);
  matadd(hp, hn, B21, ldb, B22, ldb, rhs, phn);
  if (padding_m) matcopy(1, hp, A12 + hm * lda, lda, lhs + hm * php, php);
  if (padding_p) {
    for (int i = 0; i < phm; i++) lhs[i * php + hp] = 0;
    memset(rhs + hp * phn, 0, sizeof(T) * phn);
  }
  if (padding_n) matcopy(hp, 1, B21 + hn, ldb, rhs + hn, php);

  matmul_strassen(phm, phn, php, lhs, php, rhs, phn, m6, phn);

  // M7 = (A11-A21) * (B11+B12)
  matsub(hm, php, A11, lda, A21, lda, lhs, php);
  matadd(php, hn, B11, ldb, B12, ldb, rhs, phn);
  if (padding_m) matcopy(1, php, A11 + hm * lda, lda, lhs + hm * php, php);
  if (padding_n) matcopy(php, 1, B11 + hn, ldb, rhs + hn, phn);
  matmul_strassen(phm, phn, php, lhs, php, rhs, phn, m7, phn);

  // discard.
  free(rhs);
  free(lhs);

  // C12 = M1+M2
  matadd(phm, hn, m1, phn, m2, phn, C12, ldc);

  // C21 = M3+M4
  matsub(hm, phn, m3, phn, m4, phn, C21, ldc);

  l = ALLOC_ARR(phm * phn);
  r = ALLOC_ARR(phm * phn);

  // C11 = M5+M4-M2+M6
  matsub(phm, phn, m5, phn, m4, phn, l, phn);
  matsub(phm, phn, m6, phn, m2, phn, r, phn);
  matadd(phm, phn, l, phn, r, phn, C11, ldc);

  // C22 = M5+M1-M3-M7
  matadd(phm, phn, m5, phn, m1, phn, l, phn);
  matadd(phm, phn, m3, phn, m7, phn, r, phn);
  matsub(hm, hn, l, phn, r, phn, C22, ldc);

  // free matrix in operation
  free(m1);
  free(m2);
  free(m3);
  free(m4);
  free(m5);
  free(m6);
  free(m7);
  free(r);
  free(l);

#undef A11
#undef A12
#undef A21
#undef A22
#undef B11
#undef B12
#undef B21
#undef B22
}
