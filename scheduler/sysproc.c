#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

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
/////////////////////////////////////////

int
sys_sched_policy(int pid, int policy)
{
  if (argint(0, &pid) < 0 || argint(1, &policy)<0) return -22;
  int check_if_pid_present= set_sched_policy(pid, policy);
  if(check_if_pid_present==-22) return -22;

  if (policy==0){ //EDF
    int schedulable = is_edf_schedulable(pid);
    //cprintf("Got Schedulability %d\n", schedulable);
    if (schedulable == -22){
        //cprintf("Killing process %d as it is not schedulable\n", pid);
        kill(pid);
      }
    return schedulable;
  }else if(policy==1){ //RMS
    int schedulable = is_rms_schedulable(pid);
    //cprintf("Got Schedulability %d\n", schedulable);
    if (schedulable == -22){
        //cprintf("Killing process %d as it is not schedulable\n", pid);
        kill(pid);
      }
    return schedulable;
  }else{
    return -22;
  }
}

int
sys_exec_time(int pid, int exec_time)
{
  if (argint(0, &pid) < 0 || argint(1, &exec_time) < 0) return -22;
  return set_exec_time(pid, exec_time);
}

int
sys_rate(int pid, int rate)
{
  if (argint(0, &pid) < 0 || argint(1, &rate) < 0) return -22;
  if(rate<1 || rate>30) return -22;
  return set_rate(pid, rate);
 
}

int
sys_deadline(int pid, int deadline)
{
  if (argint(0, &pid) < 0 || argint(1, &deadline) < 0) return -22;
  return set_deadline(pid, deadline);
  
}