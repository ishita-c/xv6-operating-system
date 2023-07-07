#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) 

{
    //struct proc *p;
  //int num1, num2;
 
  //sscanf (argv[1],"%d",&num1);
  //sscanf (argv[2],"%d",&num2);

  if(argc >1){
    printf(2, "no arguments required for print count syscall\n");
    exit();
  }

  print_count();

  exit();
} 