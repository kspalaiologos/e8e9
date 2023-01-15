
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

struct e8e9 {
    uint8_t cs;
	uint32_t x0, x1, i, k;
};

static struct e8e9 e8e9_init() {
    struct e8e9 s;
    s.x0 = 0;
    s.x1 = 0;
    s.i = 0;
    s.k = 5;
    s.cs = 0xFF;
    return s;
}

static int32_t e8e9_cache_byte(struct e8e9 * s, int32_t c) {
    int32_t d = s->cs & 0x80 ? -1 : (uint8_t)(s->x1);
    s->x1 >>= 8;
    s->x1 |= (s->x0 << 24);
    s->x0 >>= 8;
    s->x0 |= (c << 24);
    s->cs <<= 1;
    s->i++;
    return d;
}

static uint32_t e8e9_x_swap(uint32_t x) {
    x <<= 7;
    return (x >> 24) | ((uint8_t)(x >> 16) << 8) | ((uint8_t)(x >> 8) << 16) | ((uint8_t)(x) << (24 - 7));
}

static uint32_t e8e9_y_swap(uint32_t x) {
    x = ((uint8_t)(x >> 24) << 7) | ((uint8_t)(x >> 16) << 8) | ((uint8_t)(x >> 8) << 16) | (x << 24);
    return x >> 7;
}

static int32_t e8e9_fb(struct e8e9 * s, int32_t c) {
    uint32_t x;
    if (s->i >= s->k) {
        if ((s->x1 & 0xFE000000) == 0xE8000000) {
            s->k = s->i + 4;
            x = s->x0 - 0xFF000000;
            if (x < 0x02000000) {
                x = (x + s->i) & 0x01FFFFFF;
                x = e8e9_x_swap(x);
                s->x0 = x + 0xFF000000;
            }
        }
    }
    return e8e9_cache_byte(s, c);
}

static int32_t e8e9_bb(struct e8e9 * s, int32_t c) {
    uint32_t x;
    if (s->i >= s->k) {
        if ((s->x1 & 0xFE000000) == 0xE8000000) {
            s->k = s->i + 4;
            x = s->x0 - 0xFF000000;
            if (x < 0x02000000) {
                x = e8e9_y_swap(x);
                x = (x - s->i) & 0x01FFFFFF;
                s->x0 = x + 0xFF000000;
            }
        }
    }
    return e8e9_cache_byte(s, c);
}

static int32_t e8e9_flush(struct e8e9 * s) {
    int32_t d;
    if (s->cs != 0xFF) {
        while (s->cs & 0x80) e8e9_cache_byte(s, 0), ++s->cs;
        d = e8e9_cache_byte(s, 0);
        ++s->cs;
        return d;
    } else {
        s->x0 = 0;
        s->x1 = 0;
        s->i = 0;
        s->k = 5;
        s->cs = 0xFF;
        return -1;
    }
}

int main(int argc, char * argv[]) {
    if(argc != 2) {
        fprintf(stderr, "e8e9: argument expected.\n");
        exit(1);
    }

    struct e8e9 s = e8e9_init();
    int c, n;

    if(argv[1][0] == 'e') {
        while((c = getchar()) != EOF)
            if((n = e8e9_fb(&s, c)) >= 0)
                putchar(n);
    } else if(argv[1][0] == 'd') {
        while((c = getchar()) != EOF)
            if((n = e8e9_bb(&s, c)) >= 0)
                putchar(n);
    } else {
        fprintf(stderr, "usage:\ne8e9 e|d < input > output\n");
        exit(1);
    }

    while ((c = e8e9_flush(&s)) >= 0)
        putchar(c);
}
