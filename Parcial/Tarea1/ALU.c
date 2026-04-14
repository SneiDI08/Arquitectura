#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define N 1000000
#define NUM_OPS 8

enum { ADD, SUB, AND, OR, XOR, NOT, SHL, SHR };

typedef struct { uint32_t a, b; } Pair;

Pair pairs[N];

/*ALU*/
static inline uint32_t alu(int op, uint32_t a, uint32_t b) {
    switch (op) {
        case ADD: return a + b;
        case SUB: return a - b;
        case AND: return a & b;
        case OR:  return a | b;
        case XOR: return a ^ b;
        case NOT: return ~a;
        case SHL: return a << (b & 31);
        case SHR: return a >> (b & 31);
        default:  return 0;
    }
}

/*Tiempo*/
double now() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

/*Datos*/
void generar() {
    srand(42);
    for (int i = 0; i < N; i++) {
        pairs[i].a = rand();
        pairs[i].b = rand();
    }
}

/*Sin pipeline*/
double run_seq(long long total) {
    double t0 = now();
    volatile uint32_t sink = 0;

    for (int op = 0; op < NUM_OPS; op++)
        for (int i = 0; i < N; i++)
            sink ^= alu(op, pairs[i].a, pairs[i].b);

    return now() - t0;
}

/*Pipeline simple (4 etapas)*/
double run_pipe(long long total, long long *cycles_out) {
    double t0 = now();

    int stage[4] = {0};
    long long issued = 0, done = 0, cycles = 0;

    while (done < total) {
        if (stage[3]) { done++; stage[3] = 0; }

        for (int i = 3; i > 0; i--) {
            if (!stage[i]) {
                stage[i] = stage[i-1];
                stage[i-1] = 0;
            }
        }

        if (!stage[0] && issued < total) {
            stage[0] = 1;
            issued++;
        }

        cycles++;
    }

    *cycles_out = cycles;
    return now() - t0;
}

/*MAIN*/
int main() {
    generar();

    long long total = (long long)N * NUM_OPS;

    printf("Total instrucciones: %lld\n\n", total);

    /* Sin pipeline */
    double t1 = run_seq(total);
    printf("Sin pipeline:\n");
    printf("  Tiempo: %.4f s\n", t1);
    printf("  Throughput: %.2f MIPS\n\n", total / t1 / 1e6);

    /* Con pipeline */
    long long cycles;
    double t2 = run_pipe(total, &cycles);

    printf("Con pipeline:\n");
    printf("  Tiempo: %.4f s\n", t2);
    printf("  Ciclos: %lld\n", cycles);
    printf("  CPI: %.4f\n", (double)cycles / total);
    printf("  Throughput: %.2f MIPS\n\n", total / t2 / 1e6);

    printf("Speedup: %.2fx\n", t1 / t2);

    return 0;
}
