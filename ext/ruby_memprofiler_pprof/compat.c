#include <errno.h>
#include <ruby.h>
#include <pthread.h>
#include <stdlib.h>

#include "ruby_memprofiler_pprof.h"


#if defined(HAVE_ARC4RANDOM)
uint32_t mpp_rand() {
    return arc4random();
}
void mpp_rand_init() {

}
#elif defined(HAVE_MRAND48_R) && defined(HAVE_GETENTROPY)
// Why? Why does glibc make it so darn difficult to just get random numbers in a way
// that isn't racy across different threads and doesn't depend on global state?
#include <sys/random.h>

static pthread_mutex_t mpp_rand_lock;
static struct drand48_data mpp_rand_buffer;

uint32_t mpp_rand() {
    pthread_mutex_lock(&mpp_rand_lock);
    long int result;
    int ret = mrand48_r(&mpp_rand_buffer, &result);
    if (ret < 0) {
        rb_bug("mrand48_r returned -1?");
    }
    pthread_mutex_unlock(&mpp_rand_lock);
    return (uint32_t)result;
}

void mpp_rand_init() {
    pthread_mutex_init(&mpp_rand_lock, NULL);
    memset(&mpp_rand_buffer, 0, sizeof(struct drand48_data));
    unsigned short int seedbuf[3];
    int ret;
    ret = getentropy(seedbuf, sizeof(seedbuf));
    if (ret == -1) {
        rb_bug("getentropy returned -1?");
    }
    ret = seed48_r(seedbuf, &mpp_rand_buffer);
    if (ret < 0) {
        rb_bug("seed48_r returned -1?");
    }
}
#else
#error "No suitable RNG implementation"
#endif

void *mpp_xcalloc(size_t sz) {
    void *mem = mpp_xmalloc(sz);
    memset(mem, 0, sz);
    return mem;
}

void *mpp_xmalloc(size_t sz) {
    void *mem = malloc(sz);
    if (!mem) {
        rb_sys_fail("failed to allocate memory in ruby_memprofiler_pprof gem");
    }
    return mem;
}

void mpp_free(void *mem) {
    free(mem);
}

void *mpp_realloc(void *mem, size_t newsz) {
    void *newmem = realloc(mem, newsz);
    if (!newmem) {
        rb_sys_fail("failed to allocate memory in ruby_memprofiler_pprof gem");
    }
    return newmem;
}

void mpp_pthread_mutex_lock(pthread_mutex_t *m) {
    if (pthread_mutex_lock(m) != 0) {
        rb_sys_fail("failed to lock mutex in ruby_memprofiler_pprof gem");
    }
}

void mpp_pthread_mutex_unlock(pthread_mutex_t *m) {
    if (pthread_mutex_unlock(m) != 0) {
        rb_sys_fail("failed to unlock mutex in ruby_memprofiler_pprof gem");
    }
}

int mpp_pthread_mutex_trylock(pthread_mutex_t *m) {
    int r = pthread_mutex_trylock(m);
    if (r != 0 && r != EBUSY) {
        rb_sys_fail("failed to trylock mutex in ruby_memprofiler_pprof gem");
    }
    return r;
}

void mpp_pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *attr) {
    if (pthread_mutex_init(m, attr) != 0) {
        rb_sys_fail("failed to init mutex in ruby_memprofiler_pprof gem");
    }
}

void mpp_pthread_mutex_destroy(pthread_mutex_t *m) {
    if (pthread_mutex_destroy(m) != 0) {
        rb_sys_fail("failed to destroy mutex in ruby_memprofiler_pprof gem");
    }
}


__attribute__ ((noreturn))
void mpp_assert_fail(
    const char *msg, const char *assertion, const char *file, const char *line, const char *fn
) {
    char final_msg_buf[1024];
    int32_t final_msg_ix = 0;
    int32_t final_msg_remaining = sizeof(final_msg_buf) - 2; // -1 for null terminator, -1 for trailing newline.

#define FINAL_MSG_BUF_APPEND(str)                                                               \
    do {                                                                                        \
        int32_t seg_len = (int32_t)strlen(str);                                                 \
        int32_t copy_n = (seg_len > final_msg_remaining) ? final_msg_remaining : seg_len;       \
        memcpy(final_msg_buf, str, copy_n);                                                    \
        final_msg_ix += seg_len;                                                                \
        final_msg_remaining -= seg_len;                                                         \
    } while (0)

    FINAL_MSG_BUF_APPEND("assertion failure in ruby_memprofiler_pprof gem: ");
    FINAL_MSG_BUF_APPEND(msg);
    FINAL_MSG_BUF_APPEND(" (");
    FINAL_MSG_BUF_APPEND(assertion);
    FINAL_MSG_BUF_APPEND("; at ");
    FINAL_MSG_BUF_APPEND(file);
    FINAL_MSG_BUF_APPEND(":");
    FINAL_MSG_BUF_APPEND(line);
    FINAL_MSG_BUF_APPEND(" ");
    FINAL_MSG_BUF_APPEND(fn);
    FINAL_MSG_BUF_APPEND(")");

    // We're guaranteed to have left enough space for this because we started final_msg_remaining off
    // with sizeof(buf) - 2.
    final_msg_remaining += 2;
    FINAL_MSG_BUF_APPEND("\n\0");

    __attribute__((unused)) size_t r =
        write(STDERR_FILENO, final_msg_buf, final_msg_ix + 1);
    abort();
}
