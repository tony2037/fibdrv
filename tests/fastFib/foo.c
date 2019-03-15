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
    if (k == 2) {
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
    /* Base case */
    unsigned long long *f0, *f1;
    f0 = fast_fib(0);
    f1 = fast_fib(1);
    printf("f(0): [%llu] [%llu]\n", f0[1], f0[0]);
    printf("f(1): [%llu] [%llu]\n", f1[1], f1[0]);
    assert(f0[1] == 0 && f0[0] == 0 && f1[1] == 0 && f1[0] == 1);

    /* Using fast fibonacci formula case k = 2 */
    unsigned long long *f2;
    f2 = fast_fib(2);
    printf("f(2): [%llu] [%llu]\n", f2[1], f2[0]);
    assert(f2[1] == 0 && f2[0] == 1);

    /* Using fast fibonacci formula case k = 3 */
    unsigned long long *f3;
    f3 = fast_fib(3);
    printf("f(3): [%llu] [%llu]\n", f3[1], f3[0]);
    assert(f3[1] == 0 && f3[0] == 2);

    /* Using fast fibonacci formula case k = 4 */
    unsigned long long *f4;
    f4 = fast_fib(4);
    printf("f(4): [%llu] [%llu]\n", f4[1], f4[0]);
    assert(f4[1] == 0 && f4[0] == 3);

    /* Using fast fibonacci formula case k = 47 */
    /* should value = [0][2971215073] */
    unsigned long long *f47;
    f47 = fast_fib(47);
    printf("f(47): [%llu] [%llu]\n", f47[1], f47[0]);
    assert(f47[1] == 0 && f47[0] == 2971215073);

    /* Using fast fibonacci formula case k = 46 */
    /* should value = [0][1836311903] */
    unsigned long long *f46;
    f46 = fast_fib(46);
    printf("f(46): [%llu] [%llu]\n", f46[1], f46[0]);
    assert(f46[1] == 0 && f46[0] == 1836311903);

    /* Using fast fibonacci formula case k = 93 */
    /* should value = [0][12200160415121876738] */
    unsigned long long *f93;
    f93 = fast_fib(93);
    printf("f(93): [%llu] [%llu]\n", f93[1], f93[0]);
    assert(f93[1] == 0 && f93[0] == 12200160415121876738U);

    /* Using fast fibonacci formula case k = 94 */
    /* should value = [][] */
    unsigned long long *f94;
    f94 = fast_fib(94);
    printf("f(94): [%llu] [%llu]\n", f94[1], f94[0]);
    // assert(f93[1] == 0 && f93[0] == 12200160415121876738U);
}
