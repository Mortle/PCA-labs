#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 16
#define IT_COUNT 5000000
#define MMX_REG_SIZE 8

void array_print(__int16* a, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}

void fill_arrays(__int16* a0, __int16* a1, size_t size) {
    srand((unsigned)time(0));

    for (size_t i = 0; i < size; i++) {
        a0[i] = rand();
        a1[i] = rand();

        if (i % 2) {
            a0[i] *= -1;
            a1[i] *= -1;
        }
    }
}

int main() {
    __int16 a0[ARRAY_SIZE];
    __int16 a1[ARRAY_SIZE];
    __int16 result[ARRAY_SIZE];

    fill_arrays(a0, a1, ARRAY_SIZE);

    printf("First array:\n");
    array_print(a0, ARRAY_SIZE);
    printf("Second array:\n");
    array_print(a1, ARRAY_SIZE);

    clock_t finish;
    clock_t start = clock();

    for (size_t i = 0; i < IT_COUNT; i++) {
        for (size_t j = 0; j < ARRAY_SIZE; j++) {
            result[j] = a0[j] | a1[j];
        }
    }

    finish = clock();
    printf("\n\tC language\n");
    printf("Result:\n");
    array_print(result, ARRAY_SIZE);
    printf("Elapsed time = %f seconds\n", (double)(finish - start) / CLOCKS_PER_SEC);

    memset(result, 0, sizeof(__int16) * ARRAY_SIZE);

    start = clock();

    for (size_t i = 0; i < IT_COUNT; i++) {
        for (size_t j = 0; j < ARRAY_SIZE; j++) {
            _asm {
                push eax
                push ebx
                push ecx

                mov ebx, j
                add ebx, j
                mov ax, word ptr a0[ebx]
                mov cx, word ptr a1[ebx]
                or ax, cx
                mov word ptr result[ebx], ax

                pop ecx
                pop ebx
                pop eax
                }
        }
    }

    finish = clock();

    printf("\n\tAssembly language\n");
    printf("Result:\n");
    array_print(result, ARRAY_SIZE);
    printf("Elapsed time = %f seconds\n", (double)(finish - start) / CLOCKS_PER_SEC);

    memset(result, 0, sizeof(__int16) * ARRAY_SIZE);

    start = clock();

    for (size_t i = 0; i < IT_COUNT; i++) {
        for (size_t j = 0; j < ARRAY_SIZE; j += MMX_REG_SIZE / sizeof(__int16)) {
            _asm {
                push ebx
                mov ebx, j
                add ebx, j

                movq mm0, a0[ebx]
                movq mm1, a1[ebx]
                por mm0, mm1
                movq result[ebx], mm0

                pop ebx
                emms
            }
        }
    }

    finish = clock();

    printf("\n\tAssembly language with MMX instructions\n");
    printf("Result:\n");
    array_print(result, ARRAY_SIZE);
    printf("Elapsed time: %f seconds\n", (double)(finish - start) / CLOCKS_PER_SEC);

    system("pause");
    return 0;
}
