#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sched.h>
#include <sys/resource.h>
#include <ctype.h>

#include <arm_acle.h>

#define USEC_PER_SEC 1000000ULL
//#define TESTING
#ifdef TESTING
#define MAX_COUNT 1
#else
#define MAX_COUNT 1000000000ULL
#endif
//No need for RNG since output of hash is input for next round of hashing
//#define RNG

// Contains information about benchmark options.
typedef struct {
    int cpu_to_lock;
    int locked_freq;
} command_data_t;

void usage() {
    printf("--------------------------------------------------------------------------------\n");
    printf("Usage:");
    printf("    v8IntegerHash [--cpu_to_lock CPU] [--locked_freq FREQ_IN_KHZ]\n\n");
    printf("!!!!!!Lock the desired core to a desired frequency before invoking this benchmark.\n");
    printf(
          "Hint: Set scaling_max_freq=scaling_min_freq=FREQ_IN_KHZ. FREQ_IN_KHZ "
          "can be obtained from scaling_available_freq\n");
    printf("--------------------------------------------------------------------------------\n");
}

int processOptions(int argc, char **argv, command_data_t *cmd_data) {
    // Initialize the command_flags.
    cmd_data->cpu_to_lock = 0;
    cmd_data->locked_freq = 1;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            int *save_value = NULL;
            if (strcmp(argv[i], "--cpu_to_lock") == 0) {
                save_value = &cmd_data->cpu_to_lock;
        } else if (strcmp(argv[i], "--locked_freq") == 0) {
                save_value = &cmd_data->locked_freq;
            } else {
                printf("Unknown option %s\n", argv[i]);
                return -1;
            }
            if (save_value) {
                // Checking both characters without a strlen() call should be
                // safe since as long as the argument exists, one character will
                // be present (\0). And if the first character is '-', then
                // there will always be a second character (\0 again).
                if (i == argc - 1 ||
                    (argv[i + 1][0] == '-' && !isdigit(argv[i + 1][1]))) {
                    printf("The option %s requires one argument.\n", argv[i]);
                    return -1;
                }
                *save_value = (int)strtol(argv[++i], NULL, 0);
            }
    }
    }
    return 0;
}

// crc based implementation
/*00000000004002e8 <_Z9hash32crci>:
  4002e8:	1ac04be0 	crc32w	w0, wzr, w0
  4002ec:	d65f03c0 	ret

00000000004002f0 <_Z9hash64crcl>:
  4002f0:	aa1f03e8 	mov	x8, xzr
  4002f4:	9ac04d08 	crc32x	w8, w8, x0
  4002f8:	aa0803e0 	mov	x0, x8
  4002fc:	d65f03c0 	ret

0000000000400300 <_Z11hash6432crcl>:
  400300:	9ac05fe0 	crc32cx	w0, wzr, x0
  400304:	d65f03c0 	ret */

__attribute__ ((noinline))
uint32_t hash32crc(uint32_t v){
#ifdef TESTING
    fprintf(stderr, "input hash32crc v=%u\n", v);
#endif
    uint32_t hash = 0;
    //XOR the initial CRC with INT_MAX (crc32c_hw)
    //int hash = 0xf38cb90f; //0x0C7346F0 ^ 0xFFFFFFFF;

    hash = __crc32w(hash, v);
    v = hash;
#if 0
    fprintf(stderr, "output#1 hash32crc=%u\n", v);
    v = 1344;
    hash = 0;
    __asm ( "crc32w %w[hash],%w[hash],%w[v]"
            : [hash] "+r" (hash)
            : [v] "r" (v)
            :
          );
    v = hash;
    fprintf(stderr, "output#2 hash32crc=%u\n", v);
#endif

#ifdef TESTING
    fprintf(stderr, "output hash32crc=%u\n", hash);
#endif
    return v;
}

//integer is represented by 8 bytes (64 bit architecture)
__attribute__ ((noinline))
long hash64crc(long v){
#ifdef TESTING
    fprintf(stderr, "input hash64crc v=%ld\n", v);
#endif

#define REDUCE_COLLISION
#ifdef REDUCE_COLLISION
     long hash1 = 0, hash2 = 0;
    __asm ( "crc32w %w[hash1],%w[hash1],%w[v] \n\t"
            "lsr %x[v], %x[v], #32 \n\t"
            "crc32w %w[hash2],%w[hash2],%w[v] \n\t"
            "orr %x[v], %x[hash1], %x[hash2], lsl #32 \n\t"
            : [hash1] "+r" (hash1), [hash2] "+r" (hash2), [v] "+r" (v)
            :
            :
          );
#else
    long hash = 0;
    //long hash = 0xE7C3FD0E;
    //
    //Todo: No builtin macro currently available but can be added in future
    __asm ( "crc32x %w[hash],%w[hash],%w[v]"
            : [hash] "+r" (hash)
            : [v] "r" (v)
          );
     v = hash;
#endif
#ifdef TESTING
     fprintf(stderr, "Output hash64crc=%ld\n", v);
#endif
    return v;
}

//integer is represented by 4 bytes (32 bit architecture)
__attribute__ ((noinline))
uint32_t hash6432crc(long v){
    uint32_t hash = 0;
    //uint32_t hash = 0xE7C3FD0E;

    hash = __crc32cd(hash, v);
    v = hash;
    /*__asm ( "crc32cw %w[hash],%w[hash],%x[v]"
            : [hash] "+r" (hash)
            : [v] "r" (v)
          );*/
#ifdef TESTING
    fprintf(stderr, "Output hash6432crc=%d\n", hash);
#endif
    return v;
}


// see https://github.com/v8/v8/blob/master/src/base/functional.cc

__attribute__ ((noinline))
uint32_t hash32shift(uint32_t v){
    // "32 bit Mix Functions"
#ifdef TESTING
    fprintf(stderr, "input hash32shift v=%u\n", v);
#endif

    v = ~v + (v << 15);  // v = (v << 15) - v - 1;
    v = v ^ (v >> 12);
    v = v + (v << 2);
    v = v ^ (v >> 4);
    v = v * 2057;  // v = (v + (v << 3)) + (v << 11);
    v = v ^ (v >> 16);

#ifdef TESTING
    fprintf(stderr, "output hash32shift v=%u\n", v);
#endif

    return v;
}

__attribute__ ((noinline))
long hash64shift(long v){
    // "64 bit Mix Functions"
    v = ~v + (v << 21);  // v = (v << 21) - v - 1;
    v = v ^ (v >> 24);
    v = (v + (v << 3)) + (v << 8);  // v * 265
    v = v ^ (v >> 14);
    v = (v + (v << 2)) + (v << 4);  // v * 21
    v = v ^ (v >> 28);
    v = v + (v  << 31);
    return v;
}

__attribute__ ((noinline))
uint32_t hash6432shift(long v){
    // "64 bit to 32 bit Hash Functions"
    v = ~v + (v << 18);  // v = (v << 18) - v - 1;
    v = v ^ (v >> 31);
    v = v * 21;  // v = (v + (v << 2)) + (v << 4);
    v = v ^ (v >> 11);
    v = v + (v << 6);
    v = v ^ (v >> 22);
    return v;
}

inline uint32_t RotateRight32(uint32_t value, uint32_t shift) {
  if (shift == 0) return value;
  return (value >> shift) | (value << (32 - shift));
}


__attribute__ ((noinline))
uint32_t hash32combine(uint32_t& seed, uint32_t& value) {
  const uint32_t c1 = 0xCC9E2D51;
  const uint32_t c2 = 0x1B873593;

  value *= c1;
  value = RotateRight32(value, 15);
  value *= c2;

  seed ^= value;
  seed = RotateRight32(seed, 13);
  seed = seed * 5 + 0xE6546B64;
  return seed;
}

__attribute__ ((noinline))
long hash64combine(long& seed, long& value) {
  const uint64_t m = 0xC6A4A7935BD1E995;
  const uint32_t r = 47;

  value *= m;
  value ^= (value >> r);
  value *= m;

  seed ^= value;
  seed *= m;
  return seed;
}

int main(int argc, char **argv) {
    command_data_t cmd_data;

    if(processOptions(argc, argv, &cmd_data) == -1) {
        usage();
        return -1;
    }
    unsigned long long count = 0;
    float seconds_per_iteration, cycles_per_iteration;
    struct timeval begin_time, end_time, elapsed_time;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cmd_data.cpu_to_lock, &cpuset);
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) != 0) {
    perror("sched_setaffinity failed");
    return 1;
    }

    uint32_t seed = 66;
    uint32_t uint_value;
    long long_seed = 66;
    long long_value;

#ifdef RNG
    /* Intializes random number generator */
    srand(time(NULL));
#endif

    printf("--------------------------------------------------------------------------------\n");

    count = 0;
    uint_value = 1344;
    gettimeofday(&begin_time, NULL);
    while (count < MAX_COUNT) {
#ifndef TESTING
     uint_value = hash32shift ( uint_value );
#else
     int hashed = hash32shift ( uint_value );
     hash32shift (hashed);
#endif
      count++;
    }
    gettimeofday(&end_time, NULL);
    timersub(&end_time, &begin_time, &elapsed_time);
    fprintf(stderr, "hash32shift: %llu us\n", elapsed_time.tv_sec * USEC_PER_SEC + elapsed_time.tv_usec);
    seconds_per_iteration = (float) (elapsed_time.tv_sec * USEC_PER_SEC + elapsed_time.tv_usec) / (MAX_COUNT * USEC_PER_SEC);
    if (cmd_data.locked_freq != 0) {
        cycles_per_iteration =  (float) (seconds_per_iteration) * (1000. * cmd_data.locked_freq);
    fprintf(stderr, "hash32shift cycle/iterations: %f\n", cycles_per_iteration);
    }

    printf("--------------------------------------------------------------------------------\n");
    count = 0;
    long_value = 1344;
    gettimeofday(&begin_time, NULL);
    while (count < MAX_COUNT) {
      long_value = hash64shift( long_value );
      count++;
    }
    gettimeofday(&end_time, NULL);
    timersub(&end_time, &begin_time, &elapsed_time);
    fprintf(stderr, "hash64shift: %llu us\n", elapsed_time.tv_sec * USEC_PER_SEC + elapsed_time.tv_usec);
    seconds_per_iteration = (float) (elapsed_time.tv_sec * USEC_PER_SEC + elapsed_time.tv_usec) / (MAX_COUNT * USEC_PER_SEC);
    if (cmd_data.locked_freq != 0) {
        cycles_per_iteration =  (float) (seconds_per_iteration) * (1000. * cmd_data.locked_freq);
    fprintf(stderr, "hash64shift cycle/iterations: %f\n", cycles_per_iteration);
    }

    printf("--------------------------------------------------------------------------------\n");

    count = 0;
    long_value = 1344;
    gettimeofday(&begin_time, NULL);
    while (count < MAX_COUNT) {
      long_value = hash6432shift( long_value );
      count++;
    }
    gettimeofday(&end_time, NULL);
    timersub(&end_time, &begin_time, &elapsed_time);
    fprintf(stderr, "hash6432shift: %llu us\n", elapsed_time.tv_sec * USEC_PER_SEC + elapsed_time.tv_usec);
    seconds_per_iteration = (float) (elapsed_time.tv_sec * USEC_PER_SEC + elapsed_time.tv_usec) / (MAX_COUNT * USEC_PER_SEC);
    if (cmd_data.locked_freq != 0) {
        cycles_per_iteration =  (float) (seconds_per_iteration) * (1000. * cmd_data.locked_freq);
    fprintf(stderr, "hash6432shift cycle/iterations: %f\n", cycles_per_iteration);
    }


    printf("--------------------------------------------------------------------------------\n");

    count = 0;
    uint_value = 1344;
    gettimeofday(&begin_time, NULL);
    while (count < MAX_COUNT) {
      hash32combine(seed, uint_value );
      count++;
    }
    gettimeofday(&end_time, NULL);
    timersub(&end_time, &begin_time, &elapsed_time);
    fprintf(stderr, "hash32combine: %llu us\n", elapsed_time.tv_sec * USEC_PER_SEC + elapsed_time.tv_usec);
    seconds_per_iteration = (float) (elapsed_time.tv_sec * USEC_PER_SEC + elapsed_time.tv_usec) / (MAX_COUNT * USEC_PER_SEC);
    if (cmd_data.locked_freq != 0) {
        cycles_per_iteration =  (float) (seconds_per_iteration) * (1000. * cmd_data.locked_freq);
    fprintf(stderr, "hash32combine cycle/iterations: %f\n", cycles_per_iteration);
    }

    printf("--------------------------------------------------------------------------------\n");

    count = 0;
    long_value = 1344;
    gettimeofday(&begin_time, NULL);
    while (count < MAX_COUNT) {
      hash64combine(long_seed, long_value );
      count++;
    }
    gettimeofday(&end_time, NULL);
    timersub(&end_time, &begin_time, &elapsed_time);
    fprintf(stderr, "hash64combine: %llu us\n", elapsed_time.tv_sec * USEC_PER_SEC + elapsed_time.tv_usec);
    seconds_per_iteration = (float) (elapsed_time.tv_sec * USEC_PER_SEC + elapsed_time.tv_usec) / (MAX_COUNT * USEC_PER_SEC);
    if (cmd_data.locked_freq != 0) {
        cycles_per_iteration =  (float) (seconds_per_iteration) * (1000. * cmd_data.locked_freq);
    fprintf(stderr, "hash64combine cycle/iterations: %f\n", cycles_per_iteration);
    }

    printf("--------------------------------------------------------------------------------\n");
    count = 0;
    uint_value = 1344;
    gettimeofday(&begin_time, NULL);
    while (count < MAX_COUNT) {
#ifndef TESTING
     uint_value = hash32crc( uint_value );
#else
     uint32_t hashed = hash32crc( uint_value );
     //hash32crc(hashed);
     (void)hashed;
     hash32crc( uint_value );
#endif
      count++;
    }
    gettimeofday(&end_time, NULL);
    timersub(&end_time, &begin_time, &elapsed_time);
    fprintf(stderr, "hash32crc: %llu us\n", elapsed_time.tv_sec * USEC_PER_SEC + elapsed_time.tv_usec);
    seconds_per_iteration = (float) (elapsed_time.tv_sec * USEC_PER_SEC + elapsed_time.tv_usec) / (MAX_COUNT * USEC_PER_SEC);
    if (cmd_data.locked_freq != 0) {
        cycles_per_iteration =  (float) (seconds_per_iteration) * (1000. * cmd_data.locked_freq);
    fprintf(stderr, "hash32crc cycle/iterations: %f\n", cycles_per_iteration);
    }

    printf("--------------------------------------------------------------------------------\n");
    count = 0;
    long_value = 13441344;
    gettimeofday(&begin_time, NULL);
    while (count < MAX_COUNT) {
#ifndef TESTING
     long_value = hash64crc( long_value );
#else
     long hashed = hash64crc( long_value );
     //hash64crc(hashed);
     (void)hashed;
     hash64crc( long_value);
#endif
      count++;
    }
    gettimeofday(&end_time, NULL);
    timersub(&end_time, &begin_time, &elapsed_time);
    fprintf(stderr, "hash64crc: %llu us\n", elapsed_time.tv_sec * USEC_PER_SEC + elapsed_time.tv_usec);
    seconds_per_iteration = (float) (elapsed_time.tv_sec * USEC_PER_SEC + elapsed_time.tv_usec) / (MAX_COUNT * USEC_PER_SEC);
    if (cmd_data.locked_freq != 0) {
        cycles_per_iteration =  (float) (seconds_per_iteration) * (1000. * cmd_data.locked_freq);
    fprintf(stderr, "hash64crc cycle/iterations: %f\n", cycles_per_iteration);
    }

    printf("--------------------------------------------------------------------------------\n");

    count = 0;
    long_value = 1344;
    gettimeofday(&begin_time, NULL);
    while (count < MAX_COUNT) {
#ifndef TESTING
      long_value = hash6432crc( long_value );
#else
      hash6432crc( long_value );
      hash6432crc( long_value );
#endif
      count++;
    }
    gettimeofday(&end_time, NULL);
    timersub(&end_time, &begin_time, &elapsed_time);
    fprintf(stderr, "hash6432crc: %llu us\n", elapsed_time.tv_sec * USEC_PER_SEC + elapsed_time.tv_usec);
    seconds_per_iteration = (float) (elapsed_time.tv_sec * USEC_PER_SEC + elapsed_time.tv_usec) / (MAX_COUNT * USEC_PER_SEC);
    if (cmd_data.locked_freq != 0) {
        cycles_per_iteration =  (float) (seconds_per_iteration) * (1000. * cmd_data.locked_freq);
    fprintf(stderr, "hash6432crc cycle/iterations: %f\n", cycles_per_iteration);
    }

    printf("--------------------------------------------------------------------------------\n");

    return 0;
}
