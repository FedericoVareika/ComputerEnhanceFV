#include <mach/mach_time.h>
#include <sys/time.h>

static u64 get_os_timer_freq(void) {
    return 1000000;
}

static u64 read_os_timer(void) {
    struct timeval value;
    gettimeofday(&value, 0);

    u64 result = get_os_timer_freq() * (u64)value.tv_sec + (u64)value.tv_usec;
    return result;
}

u64 read_cpu_timer(void) {
    return mach_absolute_time();
}

u64 get_cpu_freq(u64 miliseconds_to_wait) {
    u64 os_freq = get_os_timer_freq();

    u64 cpu_start = read_cpu_timer();
    u64 os_start = read_os_timer();
    u64 os_end = 0;
    u64 os_elapsed = 0;
    while (os_elapsed < miliseconds_to_wait) {
        os_end = read_os_timer();
        os_elapsed = os_end - os_start;
    }

    u64 cpu_end = read_cpu_timer();
    u64 cpu_elapsed = cpu_end - cpu_start;
    u64 cpu_freq = 0;
    if (os_elapsed) {
        cpu_freq = os_freq * cpu_elapsed / os_elapsed;
    }

    return cpu_freq;
}
