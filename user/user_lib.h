/*
 * header file to be used by applications.
 */

int printu(const char *s, ...);
int exit(int code);
void* naive_malloc();
void naive_free(void* va);
int fork();
void yield();
// @added for lab3_challenge2
int sem_new(int semaphore_num);
void sem_P(int semaphore);
void sem_V(int semaphore);
