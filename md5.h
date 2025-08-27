#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#ifdef __ARM_NEON
#include <arm_neon.h>

// 优化的NEON基础运算宏 - 减少中间寄存器使用
#define NEON_F(x, y, z) \
    vbslq_u32(x, y, z)  // 使用位选择指令替代原来的与或操作

#define NEON_G(x, y, z) \
    vbslq_u32(z, x, y)  // 同样使用位选择指令

#define NEON_H(x, y, z) \
    veorq_u32(veorq_u32(x, y), z)  // 异或操作保持不变，因为已经是最优的

#define NEON_I(x, y, z) \
    veorq_u32(y, vornq_u32(x, z))  // 使用vorn指令优化

// 优化的循环左移 - 使用vext指令优化大位移
#define NEON_ROTATELEFT(x, n) \
    vorrq_u32(vshlq_n_u32(x, n), vshrq_n_u32(x, 32-n))

// 优化的四轮运算宏 - 减少中间步骤
// 优化的四轮运算宏
#define NEON_FF(a, b, c, d, x, s, ac) { \
    a = vaddq_u32(a, vaddq_u32(vaddq_u32(NEON_F(b, c, d), x), vdupq_n_u32(ac))); \
    a = vaddq_u32(b, NEON_ROTATELEFT(a, s)); \
}

#define NEON_GG(a, b, c, d, x, s, ac) { \
    a = vaddq_u32(a, vaddq_u32(vaddq_u32(NEON_G(b, c, d), x), vdupq_n_u32(ac))); \
    a = vaddq_u32(b, NEON_ROTATELEFT(a, s)); \
}

#define NEON_HH(a, b, c, d, x, s, ac) { \
    a = vaddq_u32(a, vaddq_u32(vaddq_u32(NEON_H(b, c, d), x), vdupq_n_u32(ac))); \
    a = vaddq_u32(b, NEON_ROTATELEFT(a, s)); \
}

#define NEON_II(a, b, c, d, x, s, ac) { \
    a = vaddq_u32(a, vaddq_u32(vaddq_u32(NEON_I(b, c, d), x), vdupq_n_u32(ac))); \
    a = vaddq_u32(b, NEON_ROTATELEFT(a, s)); \
}


// 添加优化的数据加载宏
#define NEON_LOAD_DATA(ptr) \
    vreinterpretq_u32_u8(vld1q_u8((const uint8_t*)(ptr)))

// 添加优化的数据存储宏
#define NEON_STORE_DATA(ptr, val) \
    vst1q_u8((uint8_t*)(ptr), vreinterpretq_u8_u32(val))

#endif


#define MAX_BLOCKS 10  
using namespace std;

// 定义了Byte，便于使用
typedef unsigned char Byte;
// 定义了32比特
typedef unsigned int bit32;

// MD5的一系列参数。参数是固定的，其实你不需要看懂这些
#define s11 7
#define s12 12
#define s13 17
#define s14 22
#define s21 5
#define s22 9
#define s23 14
#define s24 20
#define s31 4
#define s32 11
#define s33 16
#define s34 23
#define s41 6
#define s42 10
#define s43 15
#define s44 21

/**
 * @Basic MD5 functions.
 *
 * @param there bit32.
 *
 * @return one bit32.
 */
// 定义了一系列MD5中的具体函数
// 这四个计算函数是需要你进行SIMD并行化的
// 可以看到，FGHI四个函数都涉及一系列位运算，在数据上是对齐的，非常容易实现SIMD的并行化

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/**
 * @Rotate Left.
 *
 * @param {num} the raw number.
 *
 * @param {n} rotate left n.
 *
 * @return the number after rotated left.
 */
// 定义了一系列MD5中的具体函数
// 这五个计算函数（ROTATELEFT/FF/GG/HH/II）和之前的FGHI一样，都是需要你进行SIMD并行化的
// 但是你需要注意的是#define的功能及其效果，可以发现这里的FGHI是没有返回值的，为什么呢？你可以查询#define的含义和用法
#define ROTATELEFT(num, n) (((num) << (n)) | ((num) >> (32-(n))))

#define FF(a, b, c, d, x, s, ac) { \
  (a) += F ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}

#define GG(a, b, c, d, x, s, ac) { \
  (a) += G ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
  (a) += H ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define II(a, b, c, d, x, s, ac) { \
  (a) += I ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}


// 在md5.h中加入
// 函数声明

void SIMD_MD5Hash(const std::vector<std::string>& passwords, bit32* state);
Byte *StringProcess(std::string input, int *n_byte);
void MD5Hash(string input, bit32 *state);
void neon_md5_transform_4(uint8_t blocks[4][MAX_BLOCKS*64], bit32* state, int total_blocks);