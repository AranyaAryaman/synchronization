#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sys/time.h>
#include <bits/pthreadtypes.h>

#define NUM_BUCKETS 5     // Buckets in hash table
#define NUM_KEYS 100000   // Number of keys inserted per thread
int num_threads = 1;      // Number of threads (configurable)
int keys[NUM_KEYS];

typedef struct _bucket_entry {
  int key;
  int val;
  struct _bucket_entry *next;
} bucket_entry;

bucket_entry *table[NUM_BUCKETS];
pthread_spinlock_t bucket_spin[NUM_BUCKETS];

void panic(char *msg) {
  printf("%s\n", msg);
  exit(1);
}

double now() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// Inserts a key-value pair into the table
void insert(int key, int val) {
  int i = key % NUM_BUCKETS;
  bucket_entry *e = (bucket_entry *) malloc(sizeof(bucket_entry));
  if (!e) panic("No memory to allocate bucket!");
  e->key = key;
  e->val = val;

  pthread_spin_lock(&bucket_spin[i]);
  e->next = table[i];
  table[i] = e;
  pthread_spin_unlock(&bucket_spin[i]);
}

// Retrieves an entry from the hash table by key
// Returns NULL if the key isn't found in the table
bucket_entry * retrieve(int key) {
  int i = key % NUM_BUCKETS;
  bucket_entry *b;

  pthread_spin_lock(&bucket_spin[i]);
  for (b = table[i]; b != NULL; b = b->next) {
    if (b->key == key) {
      pthread_spin_unlock(&bucket_spin[i]);
      return b;
    }
  }
  pthread_spin_unlock(&bucket_spin[i]);
  return NULL;
}

void * put_phase(void *arg) {
  long tid = (long) arg;
  int key = 0;

  // If there are k threads, thread i inserts
  //      (i, i), (i+k, i), (i+k*2)
  for (key = tid ; key < NUM_KEYS; key += num_threads) {
    insert(keys[key], tid);
  }

  return NULL;
}

void * get_phase(void *arg) {
  long tid = (long) arg;
  int key = 0;
  long lost = 0;

  for (key = tid ; key < NUM_KEYS; key += num_threads) {
    if (retrieve(keys[key]) == NULL) lost++;
  }
  printf("[thread %ld] %ld keys lost!\n", tid, lost);

  return (void *)lost;
}

int main(int argc, char **argv) {
  long i;
  pthread_t *threads;
  double start, end;

  if (argc != 2) {
    panic("usage: ./parallel_spin <num_threads>");
  }
  if ((num_threads = atoi(argv[1])) <= 0) {
    panic("must enter a valid number of threads to run");
  }

  srandom(time(NULL));
  for (i = 0; i < NUM_KEYS; i++)
    keys[i] = random();

  threads = (pthread_t *) malloc(sizeof(pthread_t)*num_threads);
  if (!threads) {
    panic("out of memory allocating thread handles");
  }

  // Initialize spinlocks
  for (i = 0; i < NUM_BUCKETS; i++) {
    pthread_spin_init(&bucket_spin[i], PTHREAD_PROCESS_PRIVATE);
  }

  // Insert keys in parallel
  start = now();
  for (i = 0; i < num_threads; i++) {
    pthread_create(&threads[i], NULL, put_phase, (void *)i);
  }

  // Barrier
  for (i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }
  end = now();

  printf("[main] Inserted %d keys in %f seconds\n", NUM_KEYS, end - start);

  // Reset the thread array
  memset(threads, 0, sizeof(pthread_t)*num_threads);

  // Retrieve keys in parallel
  start = now();
  for (i = 0; i < num_threads; i++) {
    pthread_create(&threads[i], NULL, get_phase, (void *)i);
  }

  // Collect count of lost keys
  long total_lost = 0;
  long *lost_keys = (long *) malloc(sizeof(long) * num_threads);
  for (i = 0; i < num_threads; i++) {
    pthread_join(threads[i], (void **)&lost_keys[i]);
    total_lost += lost_keys[i];
  }
  end = now();

  printf("[main] Retrieved %ld/%d keys in %f seconds\n", NUM_KEYS - total_lost, NUM_KEYS, end - start);

  // Destroy spinlocks
  for (i = 0; i < NUM_BUCKETS; i++) {
    pthread_spin_destroy(&bucket_spin[i]);
  }

  return 0;
}
