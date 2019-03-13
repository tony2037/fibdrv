#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

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

    /* Borrow Case */
    k1[1] = 2;
    k2[1] = 1;
    k1[0] = 0;
    k2[0] = 1;
    unsigned long long *t = subtractor(k1, k2);
    if (t == NULL)
        printf("Negative\n");
    printf("Borrow Case: [%llu] [%llu] \n", t[1], t[0]);

    /* Negative Case */
    k1[1] = 2;
    k2[1] = 3;
    k1[0] = 0;
    k2[0] = 1;
    unsigned long long *k = subtractor(k1, k2);
    if (r == NULL)
        printf("Negative\n");
}
