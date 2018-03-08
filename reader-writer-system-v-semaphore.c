/*
The program contains 5 reader and 5 writer processes. Each writer writes a string like "0000000000", "1111111111" or "2222222222" to a shared memory area. 
Each reader reads the string from the shared memory area and display them. 
However, because of the race condition, the readers get strings with mixed characters such as "4011120001", or "32100011132" most of time. This is incorrect. 

Implement a multiple-readers-single-writer algorithm to prevent the race condition. 
The correct display results should look like "0000000000", "1111111111", etc. Use System V semaphores to solve the problem.
You don’t have to give readers and writers equal priority. Your code must be able to allow multiple readers to access the string concorrently.
(5) If you want to create a shared variable such as ReaderCount among processes, use the mmap() function.
*/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/times.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h> 
#include <sys/ipc.h>
#include <sys/sem.h>


#define FILE_SIZE 11
#define NO_PROC   10

int DelayCount = 0, readerID = 0, writerID = 0, Delay100ms = 0;
char* shared_buffer;
char* reader_cnt; // reader count
key_t shmkey, semkey;
int semkey_cnt, semkey_shared, semid_cnt, semid_shared; // two semaphores. One to control access to reader count, one to control access to shared buffer
FILE *file;

union semun { 
      int val; /* used for SETVAL only */ 
      struct semid_ds *buf; /* for IPC_STAT and IPC_SET */ 
      ushort *array; /* used for GETALL and SETALL */ 
}; 
union semun arg;
struct sembuf WAIT[0], SIGNAL[0]; // wait and signal operations for two semaphore

void basic_delay() {
   long i,j,k;
   for (i=0;i<200L;i++) {
       for (j=0;j<400L;j++) {
          k = k+i;
       }
   }
}
  // do some delays (in 100 ms/tick)
void delay(int delay_time) {
   int i,j;

   for (i=0; i<delay_time; i++) {
      for (j=0;j<Delay100ms;j++) {
          basic_delay();
      }
   }
}
int stop_alarm = 0;
static void sig_alrm(int signo) {
    stop_alarm = 1;
}

  // Since the speed of differet systems vary, we need to calculate the delay factor 
void calcuate_delay() {
    int i;
    struct  tms t1;
    struct  tms t2;
    clock_t t;
    long    clktck;
    double  td;

    printf(".... Calculating delay factor ......\n");
    stop_alarm = 0;
    if (signal(SIGALRM, sig_alrm) == SIG_ERR)
       perror("Set SIGALRM");
    alarm(5);  /* stop the following loop after 5 seconds */

    times(&t1);
    while (stop_alarm == 0) {
        DelayCount++;
        basic_delay();
    }    
    times(&t2);
    alarm(0);   // turn off the timer

    t = t2.tms_utime - t1.tms_utime; // Calcluate CPU time

    if ( (clktck = sysconf(_SC_CLK_TCK)) < 0 ) // fetch clock ticks per second
       perror("sysconf error");

    td = t / (double)clktck; // actual delay in seconds
    
    Delay100ms = DelayCount/td/10;

    if (Delay100ms == 0)
       Delay100ms++;

    printf(".... End calculating delay factor\n");
}
void reader() {
  printf("in reader() readerCnt=%d\n", reader_cnt[0]);
  semop(semid_cnt, WAIT, 1);
  reader_cnt[0]++; printf("Increment. readerCnt=%d\n", reader_cnt[0]);
  if (reader_cnt[0] == 1) semop(semid_shared, WAIT, 1);
  semop(semid_cnt, SIGNAL, 1);

  // read share data
  int i,j,n;
  char results[FILE_SIZE];
  srand(2);
  for (i=0; i<1; i++) {
      printf("Reader %d (pid = %d) arrives\n", readerID, getpid()); 
      for (j=0; j<FILE_SIZE; j++) {
         results[j] = shared_buffer[j]; 
         delay(1);  
      }
 
      results[j] = 0;
      printf("Reader %d gets results = %s\n", readerID, results);
  }
  semop(semid_cnt, WAIT, 1);
  reader_cnt[0]--; printf("Decrement. readerCnt=%d\n", reader_cnt[0]);
  if (reader_cnt[0] == 0) semop(semid_shared, SIGNAL, 1);
  semop(semid_cnt, SIGNAL, 1);
}

  // The writer. It tries to fill the buffer repeatly with the same digit 
void writer() {
  printf("in writer, readerCnt=%d\n", reader_cnt[0]);
  semop(semid_shared, WAIT, 1);
  // write to shared buffer
  int i,j,n;
  char data[FILE_SIZE];
  srand(1);
  for (j=0; j<FILE_SIZE-1; j++) {
      data[j]= writerID + '0';
  }
  data[j]= 0;
  for (i=0; i<1; i++) {
      printf("Writer %d (pid = %d) arrives, writing %s to buffer\n", writerID, getpid(), data);

      for (j=0; j<FILE_SIZE-1; j++) {
          shared_buffer[j]= data[j]; 
          delay(1);  
      }
      printf("Writer %d finishes\n", writerID);
  }

  semop(semid_shared, SIGNAL, 1);
}

void create_reader() {
    if (0 == fork()) {
        reader();
        exit(0);
    }
    readerID++;
}

void create_writer() {
    if (0 == fork()) {
        writer();
        exit(0);
    }
    writerID++;
}

void define_wait_signal() {
  //Defining WAIT
   WAIT[0].sem_num = 0;
   WAIT[0].sem_op = -1;
   WAIT[0].sem_flg = SEM_UNDO;
   //Defining SIGNAL
   SIGNAL[0].sem_num = 0;
   SIGNAL[0].sem_op = 1;
   SIGNAL[0].sem_flg = SEM_UNDO;
}
void main() { 
  int return_value;
  char InitData[]="0000000000\n";
  int i, fd;

  calcuate_delay();
  define_wait_signal();

  semkey_cnt = ftok(".", 'c');
  if (semkey_cnt == (key_t)-1) { perror("ftok: ftok failed\n"); exit(1); } else { fprintf(stderr, "ftok return %d\n", semkey_cnt); }

  semkey_shared = ftok(".", 's');
  if (semkey_shared == (key_t)-1) { perror("ftok: ftok failed\n"); exit(1); } else { fprintf(stderr, "ftok return %d\n", semkey_shared); }  

  arg.val = 1; // init two semaphores to 1
  semid_cnt = semget(semkey_cnt, 1, IPC_CREAT | 0666);
  semctl(semid_cnt, 0, SETVAL, arg);
  semid_shared = semget(semkey_shared, 1, IPC_CREAT | 0666);
  semctl(semid_shared, 0, SETVAL, arg);

  // The following code segment creates a memory region shared by all child processes 
  fd = open("race.dat", O_RDWR | O_CREAT | O_TRUNC, 0600);
  if ( fd < 0 ) { perror("race.dat "); exit(1);}
  write(fd, InitData, FILE_SIZE);
  unlink("race.dat");

  shared_buffer = mmap(0, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if ( shared_buffer == (caddr_t) -1) { perror("mmap"); exit(2);}

  fd = open("reader_cnt.dat", O_RDWR | O_CREAT | O_TRUNC, 0600);
  if ( fd < 0 ) { perror("race.dat "); exit(1);}
  write(fd, "0", 1);
  unlink("reader_cnt.dat");

  reader_cnt = mmap(0, 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  
  // Create some readers and writes (processes)
  create_reader();
  delay(1);
  create_writer();
  delay(1);
  create_reader();
  create_reader();
  create_reader();
  delay(1);
  create_writer();
  delay(1);
  create_reader();
  // Delay 15 seconds so all previous readers/writes can finish. This is to prevent writer starvation
  delay(150);

  create_writer();
  delay(1);
  create_writer();
  delay(1);
  create_reader();
  create_reader();
  create_reader();
  delay(1);
  create_writer();
  delay(1);
  create_reader();
  
  // Wait until all children terminate
  for (i=0; i<(readerID+writerID); i++) {
      wait(NULL);
  }
  // remove semaphores
  semctl(semid_cnt, 0, IPC_RMID);
  semctl(semid_shared, 0, IPC_RMID);
}
