#ifndef _PTHREAD_STUB_H
#define _PTHREAD_STUB_H
typedef long pthread_t;		/* identify a thread */
typedef long pthread_mutex_t;		/* identify a mutex */
typedef long pthread_cond_t;		/* identify a condition variable */
int pthread_mutex_init(pthread_mutex_t *mutex,void *p);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_cond_init(pthread_cond_t *cond,void *p);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_create(pthread_t * thread,void *p, void *(*start_routine)(void *), void * arg);
int pthread_join(pthread_t thread, void **thread_return);
#define CLOCK_REALTIME 1
struct timespec {
	long  tv_sec; 
	long  tv_nsec;
};
long clock_gettime(int X, struct timespec *t);
char *strdup(const char *);

#endif
