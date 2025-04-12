#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define NORTH 8
#define WEST 1
#define SOUTH -8
#define EAST -1

enum {
    fileHBB = 0x0101010101010101,
    fileGBB = 0x0202020202020202,
    fileFBB = 0x0404040404040404,
    fileEBB = 0x0808080808080808,
    fileDBB = 0x1010101010101010,
    fileCBB = 0x2020202020202020,
    fileBBB = 0x4040404040404040,
    fileABB = 0x8080808080808080,

    rank1BB = 0xFF,
    rank2BB = 0xFF00,
    rank3BB = 0xFF0000,
    rank4BB = 0xFF000000,
    rank5BB = 0xFF00000000,
    rank6BB = 0xFF0000000000,
    rank7BB = 0xFF000000000000,
    rank8BB = 0xFF00000000000000,

    BlackSquaresBB = 0xAA55AA55AA55AA55,

    QueenSideBB = fileABB | fileBBB | fileCBB | fileDBB,
    KingSideBB  = fileEBB | fileFBB | fileGBB | fileHBB,
};

#define BB(sq) (1ULL << sq)

uint64_t pgm_read_64( void *ptr )
{
  uint64_t result;
  memcpy_P( &result, ptr, sizeof(uint64_t) );
  return result;
}

const  uint64_t maskVertical[64] PROGMEM = {0x101010101010100ULL, 0x202020202020200ULL, 0x404040404040400ULL, 0x808080808080800ULL, 0x1010101010101000ULL, 0x2020202020202000ULL, 0x4040404040404000ULL, 0x8080808080808000ULL, 0x101010101010001ULL, 0x202020202020002ULL, 0x404040404040004ULL, 0x808080808080008ULL, 0x1010101010100010ULL, 0x2020202020200020ULL, 0x4040404040400040ULL, 0x8080808080800080ULL, 0x101010101000101ULL, 0x202020202000202ULL, 0x404040404000404ULL, 0x808080808000808ULL, 0x1010101010001010ULL, 0x2020202020002020ULL, 0x4040404040004040ULL, 0x8080808080008080ULL, 0x101010100010101ULL, 0x202020200020202ULL, 0x404040400040404ULL, 0x808080800080808ULL, 0x1010101000101010ULL, 0x2020202000202020ULL, 0x4040404000404040ULL, 0x8080808000808080ULL, 0x101010001010101ULL, 0x202020002020202ULL, 0x404040004040404ULL, 0x808080008080808ULL, 0x1010100010101010ULL, 0x2020200020202020ULL, 0x4040400040404040ULL, 0x8080800080808080ULL, 0x101000101010101ULL, 0x202000202020202ULL, 0x404000404040404ULL, 0x808000808080808ULL, 0x1010001010101010ULL, 0x2020002020202020ULL, 0x4040004040404040ULL, 0x8080008080808080ULL, 0x100010101010101ULL, 0x200020202020202ULL, 0x400040404040404ULL, 0x800080808080808ULL, 0x1000101010101010ULL, 0x2000202020202020ULL, 0x4000404040404040ULL, 0x8000808080808080ULL, 0x1010101010101ULL, 0x2020202020202ULL, 0x4040404040404ULL, 0x8080808080808ULL, 0x10101010101010ULL, 0x20202020202020ULL, 0x40404040404040ULL, 0x80808080808080ULL};
const  uint64_t maskDiagonal[64] PROGMEM = {0x8040201008040200ULL, 0x80402010080400ULL, 0x804020100800ULL, 0x8040201000ULL, 0x80402000ULL, 0x804000ULL, 0x8000ULL, 0x0ULL, 0x4020100804020000ULL, 0x8040201008040001ULL, 0x80402010080002ULL, 0x804020100004ULL, 0x8040200008ULL, 0x80400010ULL, 0x800020ULL, 0x40ULL, 0x2010080402000000ULL, 0x4020100804000100ULL, 0x8040201008000201ULL, 0x80402010000402ULL, 0x804020000804ULL, 0x8040001008ULL, 0x80002010ULL, 0x4020ULL, 0x1008040200000000ULL, 0x2010080400010000ULL, 0x4020100800020100ULL, 0x8040201000040201ULL, 0x80402000080402ULL, 0x804000100804ULL, 0x8000201008ULL, 0x402010ULL, 0x804020000000000ULL, 0x1008040001000000ULL, 0x2010080002010000ULL, 0x4020100004020100ULL, 0x8040200008040201ULL, 0x80400010080402ULL, 0x800020100804ULL, 0x40201008ULL, 0x402000000000000ULL, 0x804000100000000ULL, 0x1008000201000000ULL, 0x2010000402010000ULL, 0x4020000804020100ULL, 0x8040001008040201ULL, 0x80002010080402ULL, 0x4020100804ULL, 0x200000000000000ULL, 0x400010000000000ULL, 0x800020100000000ULL, 0x1000040201000000ULL, 0x2000080402010000ULL, 0x4000100804020100ULL, 0x8000201008040201ULL, 0x402010080402ULL, 0x0ULL, 0x1000000000000ULL, 0x2010000000000ULL, 0x4020100000000ULL, 0x8040201000000ULL, 0x10080402010000ULL, 0x20100804020100ULL, 0x40201008040201ULL};
const  uint64_t maskAntidiagonal[64] PROGMEM = {0x0ULL, 0x100ULL, 0x10200ULL, 0x1020400ULL, 0x102040800ULL, 0x10204081000ULL, 0x1020408102000ULL, 0x102040810204000ULL, 0x2ULL, 0x10004ULL, 0x1020008ULL, 0x102040010ULL, 0x10204080020ULL, 0x1020408100040ULL, 0x102040810200080ULL, 0x204081020400000ULL, 0x204ULL, 0x1000408ULL, 0x102000810ULL, 0x10204001020ULL, 0x1020408002040ULL, 0x102040810004080ULL, 0x204081020008000ULL, 0x408102040000000ULL, 0x20408ULL, 0x100040810ULL, 0x10200081020ULL, 0x1020400102040ULL, 0x102040800204080ULL, 0x204081000408000ULL, 0x408102000800000ULL, 0x810204000000000ULL, 0x2040810ULL, 0x10004081020ULL, 0x1020008102040ULL, 0x102040010204080ULL, 0x204080020408000ULL, 0x408100040800000ULL, 0x810200080000000ULL, 0x1020400000000000ULL, 0x204081020ULL, 0x1000408102040ULL, 0x102000810204080ULL, 0x204001020408000ULL, 0x408002040800000ULL, 0x810004080000000ULL, 0x1020008000000000ULL, 0x2040000000000000ULL, 0x20408102040ULL, 0x100040810204080ULL, 0x200081020408000ULL, 0x400102040800000ULL, 0x800204080000000ULL, 0x1000408000000000ULL, 0x2000800000000000ULL, 0x4000000000000000ULL, 0x2040810204080ULL, 0x4081020408000ULL, 0x8102040800000ULL, 0x10204080000000ULL, 0x20408000000000ULL, 0x40800000000000ULL, 0x80000000000000ULL, 0x0ULL};
const  uint8_t rankAttack[512] PROGMEM = {0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f, 0x2, 0xfd, 0xfa, 0xf6, 0xee, 0xde, 0xbe, 0x7e, 0x6, 0x5, 0xfb, 0xf4, 0xec, 0xdc, 0xbc, 0x7c, 0x2, 0x5, 0xfa, 0xf4, 0xec, 0xdc, 0xbc, 0x7c, 0xe, 0xd, 0xb, 0xf7, 0xe8, 0xd8, 0xb8, 0x78, 0x2, 0xd, 0xa, 0xf6, 0xe8, 0xd8, 0xb8, 0x78, 0x6, 0x5, 0xb, 0xf4, 0xe8, 0xd8, 0xb8, 0x78, 0x2, 0x5, 0xa, 0xf4, 0xe8, 0xd8, 0xb8, 0x78, 0x1e, 0x1d, 0x1b, 0x17, 0xef, 0xd0, 0xb0, 0x70, 0x2, 0x1d, 0x1a, 0x16, 0xee, 0xd0, 0xb0, 0x70, 0x6, 0x5, 0x1b, 0x14, 0xec, 0xd0, 0xb0, 0x70, 0x2, 0x5, 0x1a, 0x14, 0xec, 0xd0, 0xb0, 0x70, 0xe, 0xd, 0xb, 0x17, 0xe8, 0xd0, 0xb0, 0x70, 0x2, 0xd, 0xa, 0x16, 0xe8, 0xd0, 0xb0, 0x70, 0x6, 0x5, 0xb, 0x14, 0xe8, 0xd0, 0xb0, 0x70, 0x2, 0x5, 0xa, 0x14, 0xe8, 0xd0, 0xb0, 0x70, 0x3e, 0x3d, 0x3b, 0x37, 0x2f, 0xdf, 0xa0, 0x60, 0x2, 0x3d, 0x3a, 0x36, 0x2e, 0xde, 0xa0, 0x60, 0x6, 0x5, 0x3b, 0x34, 0x2c, 0xdc, 0xa0, 0x60, 0x2, 0x5, 0x3a, 0x34, 0x2c, 0xdc, 0xa0, 0x60, 0xe, 0xd, 0xb, 0x37, 0x28, 0xd8, 0xa0, 0x60, 0x2, 0xd, 0xa, 0x36, 0x28, 0xd8, 0xa0, 0x60, 0x6, 0x5, 0xb, 0x34, 0x28, 0xd8, 0xa0, 0x60, 0x2, 0x5, 0xa, 0x34, 0x28, 0xd8, 0xa0, 0x60, 0x1e, 0x1d, 0x1b, 0x17, 0x2f, 0xd0, 0xa0, 0x60, 0x2, 0x1d, 0x1a, 0x16, 0x2e, 0xd0, 0xa0, 0x60, 0x6, 0x5, 0x1b, 0x14, 0x2c, 0xd0, 0xa0, 0x60, 0x2, 0x5, 0x1a, 0x14, 0x2c, 0xd0, 0xa0, 0x60, 0xe, 0xd, 0xb, 0x17, 0x28, 0xd0, 0xa0, 0x60, 0x2, 0xd, 0xa, 0x16, 0x28, 0xd0, 0xa0, 0x60, 0x6, 0x5, 0xb, 0x14, 0x28, 0xd0, 0xa0, 0x60, 0x2, 0x5, 0xa, 0x14, 0x28, 0xd0, 0xa0, 0x60, 0x7e, 0x7d, 0x7b, 0x77, 0x6f, 0x5f, 0xbf, 0x40, 0x2, 0x7d, 0x7a, 0x76, 0x6e, 0x5e, 0xbe, 0x40, 0x6, 0x5, 0x7b, 0x74, 0x6c, 0x5c, 0xbc, 0x40, 0x2, 0x5, 0x7a, 0x74, 0x6c, 0x5c, 0xbc, 0x40, 0xe, 0xd, 0xb, 0x77, 0x68, 0x58, 0xb8, 0x40, 0x2, 0xd, 0xa, 0x76, 0x68, 0x58, 0xb8, 0x40, 0x6, 0x5, 0xb, 0x74, 0x68, 0x58, 0xb8, 0x40, 0x2, 0x5, 0xa, 0x74, 0x68, 0x58, 0xb8, 0x40, 0x1e, 0x1d, 0x1b, 0x17, 0x6f, 0x50, 0xb0, 0x40, 0x2, 0x1d, 0x1a, 0x16, 0x6e, 0x50, 0xb0, 0x40, 0x6, 0x5, 0x1b, 0x14, 0x6c, 0x50, 0xb0, 0x40, 0x2, 0x5, 0x1a, 0x14, 0x6c, 0x50, 0xb0, 0x40, 0xe, 0xd, 0xb, 0x17, 0x68, 0x50, 0xb0, 0x40, 0x2, 0xd, 0xa, 0x16, 0x68, 0x50, 0xb0, 0x40, 0x6, 0x5, 0xb, 0x14, 0x68, 0x50, 0xb0, 0x40, 0x2, 0x5, 0xa, 0x14, 0x68, 0x50, 0xb0, 0x40, 0x3e, 0x3d, 0x3b, 0x37, 0x2f, 0x5f, 0xa0, 0x40, 0x2, 0x3d, 0x3a, 0x36, 0x2e, 0x5e, 0xa0, 0x40, 0x6, 0x5, 0x3b, 0x34, 0x2c, 0x5c, 0xa0, 0x40, 0x2, 0x5, 0x3a, 0x34, 0x2c, 0x5c, 0xa0, 0x40, 0xe, 0xd, 0xb, 0x37, 0x28, 0x58, 0xa0, 0x40, 0x2, 0xd, 0xa, 0x36, 0x28, 0x58, 0xa0, 0x40, 0x6, 0x5, 0xb, 0x34, 0x28, 0x58, 0xa0, 0x40, 0x2, 0x5, 0xa, 0x34, 0x28, 0x58, 0xa0, 0x40, 0x1e, 0x1d, 0x1b, 0x17, 0x2f, 0x50, 0xa0, 0x40, 0x2, 0x1d, 0x1a, 0x16, 0x2e, 0x50, 0xa0, 0x40, 0x6, 0x5, 0x1b, 0x14, 0x2c, 0x50, 0xa0, 0x40, 0x2, 0x5, 0x1a, 0x14, 0x2c, 0x50, 0xa0, 0x40, 0xe, 0xd, 0xb, 0x17, 0x28, 0x50, 0xa0, 0x40, 0x2, 0xd, 0xa, 0x16, 0x28, 0x50, 0xa0, 0x40, 0x6, 0x5, 0xb, 0x14, 0x28, 0x50, 0xa0, 0x40, 0x2, 0x5, 0xa, 0x14, 0x28, 0x50, 0xa0, 0x40};
const  int8_t lookupNTZ[64] = {
         0, 47,  1, 56, 48, 27,  2, 60,
        57, 49, 41, 37, 28, 16,  3, 61,
        54, 58, 35, 52, 50, 42, 21, 44,
        38, 32, 29, 23, 17, 11,  4, 62,
        46, 55, 26, 59, 40, 36, 15, 53,
        34, 51, 20, 43, 31, 22, 10, 45,
        25, 39, 14, 33, 19, 30,  9, 24,
        13, 18,  8, 12,  7,  6,  5, 63
};
int ntz(uint64_t x) {
    uint64_t y = x^(x-1);
    constexpr uint64_t debruijn = 0x03f79d71b4cb0a89;
    uint8_t z = (debruijn*y) >> 58;
    return lookupNTZ[z];
}

uint64_t rightBit(uint64_t x){
	return x & ((~x) + 1);
}

uint64_t shift(uint64_t bb, int dir){
    int h = dir & 7;
    bb = (h == 1) ? bb & ~fileABB
    : (h == 7) ? bb & ~fileHBB 
    : bb;
    return dir > 0 ? bb << dir : bb >> -dir;
}


uint64_t bit_bswap(uint64_t b) {
    b = ((b >> 8) & 0x00FF00FF00FF00FFULL) | ((b << 8) & 0xFF00FF00FF00FF00ULL);
    b = ((b >> 16) & 0x0000FFFF0000FFFFULL) | ((b << 16) & 0xFFFF0000FFFF0000ULL);
    b = ((b >> 32) & 0x00000000FFFFFFFFULL) | ((b << 32) & 0xFFFFFFFF00000000ULL);
    return b;
}

uint64_t attack(uint64_t pieces, uint32_t x, uint64_t mask) {
    uint64_t o = pieces & mask;
    return ((o - (1ull << x)) ^ bit_bswap(bit_bswap(o) - (0x8000000000000000ull >> x))) & mask; //Daniel 28.04.2022 - Faster shift. Replaces (1ull << (s ^ 56))
}

uint64_t horizontal_attack(uint64_t pieces, uint32_t x) {
    uint32_t file_mask = x & 7;
    uint32_t rank_mask = x & 56;
    uint64_t o = (pieces >> rank_mask) & 126;

    return ((uint64_t)pgm_read_byte(rankAttack + (o * 4 + file_mask))) << rank_mask;
}

uint64_t vertical_attack(uint64_t occ, uint32_t sq) {
    return attack(occ, sq, pgm_read_64(maskVertical + sq));
}

uint64_t diagonal_attack(uint64_t occ, uint32_t sq) {
    return attack(occ, sq, pgm_read_64(maskDiagonal + sq));
}

uint64_t antidiagonal_attack(uint64_t occ, uint32_t sq) {
    return attack(occ, sq, pgm_read_64(maskAntidiagonal + sq));
}

uint64_t bishop_attack(int sq, uint64_t occ) {
    return diagonal_attack(occ, sq) | antidiagonal_attack(occ, sq);
}

uint64_t rook_attack(int sq, uint64_t occ) {
    return vertical_attack(occ, sq) | horizontal_attack(occ, sq);
}

uint64_t queen_attack(int sq, uint64_t occ) {
    return bishop_attack(sq, occ) | rook_attack(sq, occ);
}

uint64_t knight_attack(int sq){
    uint64_t x = BB(sq);
    uint64_t moves = 0ULL;
    uint64_t temp = shift(shift(x, NORTH), NORTH);
    moves |= shift(temp, EAST) | shift(temp, WEST);

    temp = shift(shift(x, SOUTH), SOUTH);
    moves |= shift(temp, EAST) | shift(temp, WEST);

    temp = shift(shift(x, EAST), EAST);
    moves |= shift(temp, NORTH) | shift(temp, SOUTH);

    temp = shift(shift(x, WEST), WEST);
    moves |= shift(temp, NORTH) | shift(temp, SOUTH);

    return moves;

}

uint64_t king_attack(int sq){
    uint64_t x = BB(sq);
    return (shift(x, NORTH) | shift(x, SOUTH) | shift(x, EAST) | shift(x, WEST) 
        | shift(shift(x, NORTH), WEST) | shift(shift(x, NORTH), EAST)
        | shift(shift(x, SOUTH), WEST) | shift(shift(x, SOUTH), EAST));
}

// This will read the right most move from a piece and it's possible attacks

