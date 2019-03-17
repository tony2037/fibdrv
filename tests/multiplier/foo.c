#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>


static unsigned long long *adder(unsigned long long *k1, unsigned long long *k2)
{
    unsigned long long *r = malloc(2 * sizeof(unsigned long long));
    if (r == NULL) {
        printf("kmalloc error");
        return NULL;
    }
    char carry = 0;
    if ((ULONG_MAX - k2[0]) < k1[0])
        carry = 1;
    r[0] = k1[0] + k2[0];
    r[1] = k1[1] + k2[1] + (unsigned long long) (carry);
    return r;
}

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
            unsigned long long tmp[2] = {0};
            tmp[1] = 0;
            tmp[0] = k1[0] << i;
            r = adder(r, tmp);
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
    printf("Small number Case: [%llu] [%llu] \n", r[1], r[0]);
    assert(r[1] == 0);
    assert(r[0] == 100 * 200);

    /* Small number same, number*/
    unsigned long long *t = multiplier(k1, k1);
    unsigned long long *k = multiplier(k2, k2);
    if (t == NULL || k == NULL)
        printf("Error\n");
    printf("Small number, same, Case t: [%llu] [%llu] \n", t[1], t[0]);
    printf("Small number, same, Case k: [%llu] [%llu] \n", k[1], k[0]);
    assert(t[1] == 0);
    assert(t[0] == 100 * 100);
    assert(k[1] == 0);
    assert(k[0] == 200 * 200);

    /* carry case 1 */
    k1[1] = 0;
    k2[1] = 0;
    k1[0] = 0xFFFFFFFFFFFFFFFF;
    k2[0] = 2;
    unsigned long long *c1 = multiplier(k1, k2);
    printf("Carry Case 1: [%llu] [%llu] \n", c1[1], c1[0]);
    assert(c1[1] == 1);
    assert(c1[0] == 0xFFFFFFFFFFFFFFFE);

    /* carry case 2 */
    k1[1] = 0;
    k2[1] = 1;
    k1[0] = 0xFFFFFFFFFFFFFFFF;
    k2[0] = 0;
    unsigned long long *c2 = multiplier(k1, k2);
    unsigned long long *c2_ = multiplier(k2, k1);
    printf("Carry Case 2: [%llu] [%llu] \n", c2[1], c2[0]);
    assert(c2[1] == 0xFFFFFFFFFFFFFFFF);
    assert(c2[0] == 0);
    assert(c2[0] == c2_[0]);
    assert(c2[1] == c2_[1]);

    /* carry case 3 */
    k1[1] = 0;
    k1[0] = 7778742049;  // f(49)
    k2[1] = 0;
    k2[0] = 4807526976;  // f(48)
    unsigned long long *c3 = multiplier(k1, k2);
    printf("Carry Case 3: [%llu] [%llu] \n", c3[1], c3[0]);
    assert(c3[1] == 0x2 && c3[0] == 503024092493910592);
}
