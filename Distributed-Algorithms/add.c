#include "types.h"
#include "stat.h"
#include "user.h"
int signed_atoi(char* s){
  int num =0;
  int pole =1;//pos
  if (*s =='-'){
    pole = -1;
    s+=1;
  }
  while(*s<='9'&& *s>='0'){
    num *= 10;
    num += *s-'0';
    s+=1;
  }
  return num*pole;
}

int main(int argc, char *argv[]) 
{
  //int num1, num2;
 
  //sscanf (argv[1],"%d",&num1);
  //sscanf (argv[2],"%d",&num2);

  if(argc != 3){
    printf(2, "two arguments required for add syscall\n");
    exit();
  }
  // char * str1;
  // str1=argv[1];
  // char* str2=argv[2];
  // int num1 =atoi(argv[1]);
  // int num2=atoi(argv[2]);
  // if(*str1=='-') num1=-num1;
  // if(*str2=='-') num2=-num2;
  printf(1,"Sum of the numbers are: %d\n", add(signed_atoi(argv[1]),signed_atoi(argv[2])));

  exit();
} 