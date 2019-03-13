#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static unsigned long long *multiplier(unsigned long long *k1,
                                      unsigned long long *k2)
{
    unsigned long long *r = malloc(2 * sizeof(unsigned long long));
    if (r == NULL) {
        printf("kmalloc error");
        return NULL;
    }
    r[0] = 0;
    r[1] = 0;
    size_t width = 8 * sizeof(unsigned long long);
    for (size_t i = 0; i < width; i++) {
        if ((k2[0] >> i) & 0x1) {
            r[1] += k1[1] << i;
            unsigned long long t = k1[0];
            (i == 0) ? (t = 0) : (t = t >> (width - i));
            r[1] += t;
            r[0] += k1[0] << i;
        }
    }
    for (size_t i = 0; i < width; i++) {
        if ((k2[1] >> i) & 0x1) {
            r[1] += k1[0] << i;
        }
    }
    return r;
}

int main(int argc, char **argv)
{
    unsigned long long k1[2] = {0};
    unsigned long long k2[2] = {0};
    /* Small number Case */
    k1[1] = 0;
    k2[1] = 0;
    k1[0] = 100;
    k2[0] = 200;
    unsigned long long *r = multiplier(k1, k2);
    if (r == NULL)
        printf("Error\n");
    printf("Normal Case: [%llu] [%llu] \n", r[1], r[0]);
    assert(r[1] == 0);
    assert(r[0] == 100 * 200);
}
