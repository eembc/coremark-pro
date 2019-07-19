#include "pthread.h"

int pthread_mutex_init(pthread_mutex_t *mutex,void *p) {
	return 1;
}
int pthread_mutex_lock(pthread_mutex_t *mutex) {
	return 1;
}
int pthread_mutex_trylock(pthread_mutex_t *mutex) {
	return 1;
}
int pthread_mutex_unlock(pthread_mutex_t *mutex) {
	return 1;
}
int pthread_mutex_destroy(pthread_mutex_t *mutex) {
	return 1;
}
int pthread_cond_init(pthread_cond_t *cond,void *p) {
	return 0;
}
int pthread_cond_signal(pthread_cond_t *cond) {
	return 0;
}
int pthread_cond_broadcast(pthread_cond_t *cond) {
	return 0;
}
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex){
	return 0;
}
int pthread_cond_destroy(pthread_cond_t *cond) {
	return 0;
}
int pthread_create(pthread_t * thread,void *p, void *(*start_routine)(void *), void * arg) {
	start_routine(arg);
	return 1;
}
int pthread_join(pthread_t pthread_thread, void **thread_return) {
	return 1;
}
long clock_gettime(int X, struct timespec *t) {
	t->tv_sec=0;
	t->tv_nsec=0;
	return 0;
}
char *strdup(const char *p) {
	return 0;
}
