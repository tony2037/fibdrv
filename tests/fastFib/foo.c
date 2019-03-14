#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static unsigned long long *subtractor(unsigned long long *k1,
                                      unsigned long long *k2)
{
    /* Assume k1 >= k2, return positive, or NULL as fail */
    if (k1 == NULL || k2 == NULL)
        return NULL;
    if (k1[1] < k2[1])
        return NULL;
    if ((k1[1] == k2[1]) && (k1[0] < k2[0]))
        return NULL;
    unsigned long long *r = malloc(2 * sizeof(unsigned long long));
    if (r == NULL) {
        printf("kmalloc error");
        return NULL;
    }
    if (k1[0] < k2[0]) {
        /* Borrow */
        k1[1] -= 1;
        r[0] = ULONG_MAX + 1 - k2[0] + k1[0];
        r[1] = k1[1] - k2[1];
        return r;
    } else {
        r[1] = k1[1] - k2[1];
        r[0] = k1[0] - k2[0];
        return r;
    }
}

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

static unsigned long long *fast_fib(int k)
{
    if (k == 0) {
        unsigned long long *r = malloc(2 * sizeof(unsigned long long));
        if (r == NULL) {
            printf("kmalloc error");
            return NULL;
        }
        r[0] = 0;
        r[1] = 0;
        return r;
    }
    if (k == 1) {
        unsigned long long *r = malloc(2 * sizeof(unsigned long long));
        if (r == NULL) {
            printf("kmalloc error");
            return NULL;
        }
        r[0] = 1;
        r[1] = 0;
        return r;
    }
    /* f(2n) = 2 * f(n+1) * f(n) - [f(n)]^2 */
    /* f(2n+1) = [f(n+1)]^2 + [f(n)]^2 */
    if (k % 2) {
        /* Odd */
        unsigned long long *fn1, *fn;
        fn1 = fast_fib((k >> 1) + 1);
        fn = fast_fib(k >> 1);
        return adder(multiplier(fn1, fn1), multiplier(fn, fn));
    } else {
        unsigned long long *fn1, *fn, *front;
        unsigned long long two[2] = {0};
        two[0] = 2;
        fn1 = fast_fib((k >> 1) + 1);
        fn = fast_fib(k >> 1);
        front = multiplier(fn1, fn);
        front = multiplier(two, front);
        return subtractor(front, multiplier(fn, fn));
    }
}

int main(int argc, char **argv)
{
    unsigned long long k1[2] = {0};
    unsigned long long k2[2] = {0};
    /* Normal Case */
    k1[1] = 2;
    k2[1] = 1;
    k1[0] = 100;
    k2[0] = 1;
    unsigned long long *r = subtractor(k1, k2);
    if (r == NULL)
        printf("Negative\n");
    printf("Normal Case: [%llu] [%llu] \n", r[1], r[0]);
    assert(r[1] == 1);
    assert(r[0] == 99);

    /* Borrow Case */
    k1[1] = 2;
    k2[1] = 1;
    k1[0] = 0;
    k2[0] = 1;
    unsigned long long *t = subtractor(k1, k2);
    if (t == NULL)
        printf("Negative\n");
    printf("Borrow Case: [%llu] [%llu] \n", t[1], t[0]);
    assert(t[1] == 0);
    assert(t[0] == 0xFFFFFFFFFFFFFFFF);

    /* Negative Case */
    k1[1] = 2;
    k2[1] = 3;
    k1[0] = 0;
    k2[0] = 1;
    unsigned long long *k = subtractor(k1, k2);
    if (k == NULL)
        printf("Negative\n");
    assert(k == NULL);
}
