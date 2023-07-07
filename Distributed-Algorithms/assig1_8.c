#include "types.h"
#include "stat.h"
#include "user.h"

float string_to_float(char *str) {
    int i = 0;
    float integer_part = 0.0;
    float fraction_part = 0.0;
    float fraction_divisor = 1.0;
    while (str[i] >= '0' && str[i] <= '9') {
        integer_part = (integer_part * 10.0) + (float)(str[i++] - '0');
    }
    if (str[i++] == '.') {
        while (str[i] >= '0' && str[i] <= '9') {
            fraction_part = (fraction_part * 10.0) + (float)(str[i++] - '0');
            fraction_divisor *= 10.0;
        }
    }
    float result = (integer_part + (fraction_part / fraction_divisor));
    return result;
}



char* float_to_string(float value) {
  char* result = (char*) malloc(8 * sizeof(char));
  int integer_part = (int) value;
  float fractional_part = value - (float) integer_part;
  int i = 0;
  if (integer_part == 0) {
    result[i++] = '0';
  }
  while (integer_part > 0) {
    result[i++] = integer_part % 10 + '0';
    integer_part /= 10;
  }
  for (int k = 0, j = i - 1; k < j; k++) {
    char temp = result[k];
    result[k] = result[j];
    result[j--] = temp;
  }
  if (fractional_part > 0) {
    result[i++] = '.';
    while (i < 6 && fractional_part > 0) {
      fractional_part *= 10;
      int digit = (int) fractional_part;
      result[i++] = digit + '0';
      fractional_part -= (float) digit;
    }
  }
  result[i] = '\0';
  return result;
}



int int_to_str(int num, char* str) {
  int i = 0, rem, len = 0;
  // Count the number of digits in the integer
  int modify = num;
  while (modify != 0) {
    len++;
    modify /= 10;
  }

  while (num != 0) {
    rem = num % 10;
    str[len - i - 1] = rem + '0';
    i++;
    num /= 10;
  }

  // Add null terminator to the end of the string
  str[len] = '\0';
  return 1;
}

void print_variance(float xx)
{
  int beg=(int)(xx);
  int fin=(int)(xx*100)-beg*100;
  printf(1, "Variance of array for the file arr is %d.%d\n", beg, fin);
}



int
main(int argc, char *argv[])
{
	if(argc< 2){
		printf(1,"Need type and input filename\n");
		exit();
	}
	char *filename;
	filename=argv[2];
	int type = atoi(argv[1]);
	printf(1,"Type is %d and filename is %s\n",type, filename);

	//int tot_sum = 0;
	float variance = 0.0;	

	int size=1000;
	short arr[size];
	char c;
	int fd = open(filename, 0);
	for(int i=0; i<size; i++){
		read(fd, &c, 1);
		arr[i]=c-'0';
		read(fd, &c, 1);
	}	
  	close(fd);
  	// this is to supress warning
  	printf(1,"first elem %d\n", arr[0]);
  
  	//----FILL THE CODE HERE for unicast sum
	int parent_pid = getpid();
    int batch = size/8;
    int child_pids[8];
    // will fork 8 children and assign them a section to read
    for (int i = 0; i < 8; i++){
        int child_pid = fork();
        if (child_pid!=0){
            // parent here
            child_pids[i] = child_pid;
            int partition = i;
            char *buffer = (char*)malloc(8);
            // printf(1,"p%d\n", partition);
            int_to_str(partition, buffer);
            send(parent_pid,child_pid,buffer);
        }else{
            // child here
            char* received_message = (char*)malloc(8);
            recv(received_message);
            int partition = atoi(received_message);
            // printf(1,"%d\n", partition);
            int start_loc = partition*batch;
            int end_loc = (partition+1)*batch;
            int partial_sum = 0;
            for (int i = start_loc; i < end_loc; i++){
                partial_sum += arr[i];
            }
            char *buffer = (char*)malloc(8);
            int_to_str(partial_sum,buffer);
            send(getpid(), parent_pid, buffer);
            if(type == 0)
              exit();
            else {
              char* received_mean = (char*)malloc(8);
              recv(received_mean);
              float float_mean = string_to_float(received_mean);
              float partial_square_sum = 0.0;
              for (int i = start_loc; i < end_loc; i++){
                partial_square_sum += (arr[i]-float_mean)*(arr[i]-float_mean);
              }
              char* partial_square_sum_buffer = float_to_string(partial_square_sum);
              send(getpid(), parent_pid, partial_square_sum_buffer);
              exit();
            }
        }
    }
    int total_sum = 0;
    for (int i=0; i<8; i++){
        if(type == 0) wait();
        char* received_sum = (char*)malloc(8);
        recv(received_sum);
        total_sum += atoi(received_sum);
    }



  	//------------------

  	if(type==0){ //unicast sum
		printf(1,"Sum of array for file %s is %d\n", filename, total_sum);
	} else if (type==1){
      float mean = (float)total_sum/size;
      char * mean_buffer = float_to_string(mean);
      // printf(1,"Mean Value being sent %s\n",mean_buffer); -> PASSED
      send_multi(getpid(),child_pids,mean_buffer);
      float diff_square_sum = 0;
      for (int i=0; i<8; i++){
          wait();
          char* received_diff_sum = (char*)malloc(8);;
          recv(received_diff_sum);
          //print_variance(string_to_float(received_diff_sum));
          diff_square_sum += string_to_float(received_diff_sum);
      }
      variance = diff_square_sum/size;
      print_variance(variance);
    }
	exit();
}
////////////////////////////////////////

	
	

	

	
	

  	
    