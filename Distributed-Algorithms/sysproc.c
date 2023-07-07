#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

///////////////
int trace_on=0;
int count_syscall[30]={0};
///////////////

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

//process list
int
sys_ps(void)
{
  ps();
  return 0;  // not reached
}



int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_add(int num1, int num2)
{
    //int num1, num2;
    argint(0,&num1); 
    argint(1, &num2);
  
    //cprintf("%d - is the sum of the numbers:",num1+num2);
    return num1+num2;
   
}


int sys_toggle(void)
{
  
    if(trace_on==1){
      trace_on=0;
      for(int i=0;i<28;i++) count_syscall[i]=0;
    }
    else if(trace_on==0) trace_on=1;
    return 0;
}

char syscall_names[28][20]={"sys_add","sys_chdir", "sys_close","sys_dup", "sys_exec","sys_exit","sys_fork",
 "sys_fstat","sys_getpid","sys_kill", "sys_link","sys_mkdir","sys_mknod", "sys_open","sys_pipe","sys_print_count",
 "sys_ps","sys_read", "sys_recv","sys_sbrk","sys_send", "sys_send_multi","sys_sleep","sys_toggle","sys_unlink","sys_uptime", "sys_wait","sys_write"};

int sys_print_count(void)
{
    //locking 
    if(trace_on!=1) return 0;
    for(int i=0; i <28;i++){
      if(count_syscall[i]!=0){
        cprintf("%s %d\n",syscall_names[i],count_syscall[i]);
      }

    }
    return 0;
}
/////////////////////
#define max_msg_size 8
#define max_queue_size 10

struct queue_element{
  int sender_pid;
  char msg[max_msg_size];

};


struct queue_element recv_queue[NPROC][max_queue_size];


struct spinlock recv_queue_lock;


int queue_size[NPROC]={0};

void init_recv_queue(void){
  // init spinlock
  initlock(&recv_queue_lock, "recv_queue_lock");
  for(int i=0;i<NPROC;i++){
    for(int j=0;j<max_queue_size;j++){
      recv_queue[i][j].sender_pid=-1;
      for(int k=0;k<max_msg_size;k++){
        recv_queue[i][j].msg[k]='\0';
      }
    }
    queue_size[i]=0;
  }
}

int sys_send(int sender_pid, int rec_pid, void* msg)
{
    // check assertions
    if(argint(0,&sender_pid)<0) return -1; 
    if(argint(1, &rec_pid)<0) return -1;
    // Assumption: The pids are less than NPROC
    if(sender_pid>=NPROC || rec_pid>=NPROC) return -1;
    if(sender_pid<0 ||rec_pid<0) return -1;
    
    // check if the queue is full
    if (queue_size[rec_pid]==max_queue_size) return -1;

   
    char* complete_message = (char*)msg;
    if (argptr(2, &complete_message, max_msg_size)<0)
      return -1;
    acquire(&recv_queue_lock);
    char* qmsg = recv_queue[rec_pid][queue_size[rec_pid]].msg;
    do {
      *qmsg++ = *complete_message;
      
    } while (*complete_message++ != '\0');
    recv_queue[rec_pid][queue_size[rec_pid]].sender_pid=sender_pid;
    queue_size[rec_pid]++;
    // cprintf("sender_pid: %d, rec_pid: %d, msg: %s\n",sender_pid, rec_pid, qmsg);
    // cprintf("queue_size: %d\n",queue_size[rec_pid]);
    release(&recv_queue_lock);
    return 0;
}

int sys_recv(void* msg)
{
    char* recv_msg = (char*)msg;
    if(argptr(0, &recv_msg, max_msg_size)<0) return -1;
    int rec_pid=myproc()->pid;
    // cprintf("I am %d\n", rec_pid);
    int recv=0;
    char* to_write = recv_msg;
    while(!recv){
      acquire(&recv_queue_lock);
      // cprintf("Q size %d\n",queue_size[rec_pid]);
      if(queue_size[rec_pid]!=0){
        
        recv=1;
       
        char* qmsg = recv_queue[rec_pid][0].msg;
        do {
          *to_write++ = *qmsg;
        } while (*qmsg++ != '\0');

        for(int i=0;i<queue_size[rec_pid]-1;i++){
          recv_queue[rec_pid][i]=recv_queue[rec_pid][i+1];
        }
        queue_size[rec_pid]--;
      }
      release(&recv_queue_lock);
    }

    return 0;
   
}

int multi_helper(int sender_pid, int rec_pid, void* msg)
{
    // Assumption: The pids are less than NPROC
    if(sender_pid>=NPROC || rec_pid>=NPROC) return -1;
    // if(argptr(2, (void*)&msg, max_msg_size)<0) return -1;
    if(sender_pid<0 ||rec_pid<0) return -1;
    // check if the queue is full
    if (queue_size[rec_pid]==max_queue_size) return -1;
    char* complete_message = (char*)msg;
    acquire(&recv_queue_lock);
    char* qmsg = recv_queue[rec_pid][queue_size[rec_pid]].msg;
    do {
      *qmsg++ = *complete_message;
    } while (*complete_message++ != '\0');
    queue_size[rec_pid]++;
    release(&recv_queue_lock);
    return 0;
}

int sys_send_multi(int sender_pid, int rec_pids[], void *msg)
{
  if(argint(0,&sender_pid)<0) return -1;
  if (argptr(1, (void*)&rec_pids, 8 * sizeof(int)) < 0) return -1;
  char *broadcast_msg = (char*)msg;
  if (argptr(2, &broadcast_msg, max_msg_size) < 0) return -1;
  for(int i = 0; i < 8; i++){
    if (rec_pids[i]<0) continue;
    multi_helper(myproc()->pid, rec_pids[i], broadcast_msg);
  }
  return 0;
}