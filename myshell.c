#define _CRT_SECURE_NO_WARNINGS

#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

/*argc: number of strings pointed by argv*/
/*char** argv: pointer to array of character arrays "strings"*/
struct command{
    char** argv;  
    int redirect_out;
    int redirect_in;
    int pipe;
    int back;

};

int prompt() {
	int status = 1;
	printf("my_shell$");
	return status;
}

char* input_read(int* length) {
	char* input;
	*length = 0;
	int max = 512;
	//char ch;

	input = malloc(sizeof(char) * max);

	// while ((ch = fgetc(stdin)) != '\n') {
	// 	input[*length] = ch;
	// 	(*length)++;
	// 	if (*length == max) {
	// 		max += max;
	// 		input = realloc(input, max);
	// 	}
	// }
	// input[*length] = '\0';

    if (fgets(input, 512, stdin)==NULL){
        printf("CTRL-D is pressed \n");
        exit(0);
    }
    //int i= *length;
    int i=strlen(input);
    *length = i-1;
    //printf("%i", *length);

	return input;
}

char** parse(char line[], int* length) {
	char** arg=NULL;
	*length = 0;
	int word_length = 32; //rare to get a 32 character word
	int max = 32;
	char* str;
    char line2[512];

	char* delim = " \n"; //include >&|

	arg = malloc(word_length * max * sizeof(char* ));
    strcpy(line2, line);
	str = strtok(line2, delim);
	while (str != NULL) {
        printf("arg: %i, %s \n", *length, str);
		arg[*length] = str;
		(*length)++;

		if (*length == max) {
			max += max;

			arg = realloc(arg, max * word_length);
			if (arg == NULL) {
				printf("ASSIGNED NULL POINTER\n");
			}
		}

		str = strtok(NULL, delim);
		
	}

    int y;
    for (y=0; y<(*length); y++){
        printf("arg: %i, %s \n", y, arg[y]);

    }

	return arg;
}

// int read_command(char* file_name){
//     int fd, nread;
//     char buf[1024];
//     fd = open(file_name, 0);
//     nread = read(fd, buf, 1024);
    
//     close(fd);
// }

int DeepCopy(char **a, char**b, int len){
    int i;
    printf("posttest: A: %i %s B: %i %s \n", 0, a[0], 0, b[0]);
    for (i=0; i<len; i++){
        printf("A: %i %s B: %i %s \n", i, a[i], i, b[i]);
        //char* ch1=malloc(strlen(b[i])+1);
        a[i]=malloc(strlen(b[i])+1);
        //a[i]=strdup(b[i]);
        strcpy(a[i], b[i]);
        printf("A: %i %s B: %i %s \n", i, a[i], i, b[i]);
    }

    return 1;
}

int alloc_argv(struct command* list, int length){
    int word_length=32;
    int arg_max=32;
    // int line_max=512;
    int i;
    for ( i=0; i< length; i++){

        list[i].argv=malloc(word_length * arg_max * sizeof(char*));
    }
    return 1;
}

int construct_commands(char* input, struct command* list,int width, int* struct_length){
    int word_length=32;
    int arg_max=32;
    int line_max=512;
    int* length=malloc(sizeof(int));

    char str3[512];
    //char str4[12];
    int iter=0;
    int index=0;

    int redirection_in;
    int redirection_out;
    int ri;
    int ro;

    //int first=0;
    int b=0;
    int error=0;
    
    char* cmd = malloc(line_max * sizeof(char));
    char** arg = malloc(word_length * arg_max * sizeof(char*));
    //char*** arg_list = malloc(sizeof(char**) * 32);
    int i;
    for (i=0; i<width; i++){
        //memset(cmd, 0, sizeof(cmd));

        cmd[index]=input[i];
        index++;
        printf("COMMANDS_iter: %i %s\n", iter, cmd);                        


        if (input[i]=='|'){
            cmd[index-1]= ' ';
            strcpy(str3, cmd);            
            arg=parse(str3,length);

            DeepCopy(list[iter].argv, arg, *length);
            //printf("%s \n", list[0].argv[0]);

            list[iter].redirect_out=0;
            list[iter].redirect_in=0;
            list[iter].pipe=1;
            list[iter].back=0;
            
            iter++;
            index=0;
            memset(cmd, 0, line_max * sizeof(char));
            memset(*arg, 0, sizeof(arg));

        }
        else if (input[i]=='<'){ //in redirection
            cmd[index-1]=' ';
            strcpy(str3, cmd);
            arg=parse(str3, length);

            DeepCopy(list[iter].argv, arg, *length);

            list[iter].redirect_out=0;
            list[iter].redirect_in=1;
            list[iter].pipe=0;
            list[iter].back=0;

            ri=1;
            redirection_in=iter; 

            iter++;
            index=0;
            memset(cmd, 0, 512* sizeof(char));
            memset(*arg, 0, sizeof(arg));  

            
                     
        } 
        else if (input[i]=='>'){ //out redirection
            //printf("copying arguments into list \n");
            cmd[index-1]=' ';
            strcpy(str3, cmd);
            arg=parse(str3, length);
            //printf("arg: %s", arg);

            DeepCopy(list[iter].argv, arg, *length);

            //int j=0;
            //while (arg[j]!=NULL){
            //printf("command %i ", counter);
            //printf("purpose %i ", (list+counter)->redirect_out);

            // printf("argv %i, %s \n",j, arg[j]);


            // j++;
            // }
            list[iter].redirect_out=1;
            list[iter].redirect_in=0;
            list[iter].pipe=0;
            list[iter].back=0;

            ro=1;
            redirection_out=iter; 


            iter++;
            index=0;
            memset(cmd, 0, 512 * sizeof(char));
            memset(*arg, 0, sizeof(arg));            

        }else if (input[i]=='&'){
            cmd[index-1]=' ';
            strcpy(str3, cmd);
            arg=parse(str3, length);

            DeepCopy(list[iter].argv, arg, *length);

            list[iter].redirect_out=0;
            list[iter].redirect_in=0;
            list[iter].pipe=0;
            list[iter].back=1;

            iter++;
            index=0;
            memset(cmd, 0, line_max * sizeof(char));
            memset(*arg, 0, sizeof(arg)); 

            b=1;           
           
        }
        // else{
        //     printf("error_comm \n");
        // }

        if(i==width-1 && input[i]!='&'){
            printf("LAST COMMAND\n");
            strcpy(str3, cmd);
            printf("COMMANDS: %s\n", cmd);
            printf("COMMANDS_B: %s\n", str3);
            arg=parse(str3, length);
            printf("test: argv %s \n", arg[0]);
            int j=0;
            while (arg[j]!=NULL){
                printf("command %i ", iter);
                //printf("purpose %i ", (list+counter)->redirect_out);

                printf("argv %i, %s \n",j, arg[j]);


                j++;
            }

            printf("posttest: A: %i %s B: %i %s \n", 0, list[iter].argv[0], 0, arg[0]);
            DeepCopy(list[iter].argv, arg, *length);   

            printf("test2: argv %s \n", arg[0]);
            j=0;
            while (arg[j]!=NULL){
                printf("command %i ", iter);
                //printf("purpose %i ", (list+counter)->redirect_out);

                printf("argv %i, %s \n",j, arg[j]);


                j++;
            }
            

            list[iter].redirect_out=0;
            list[iter].redirect_in=0;
            list[iter].pipe=0;
            list[iter].back=0;

            iter++;
            index=0;
            memset(cmd, 0, 512 * sizeof(char));
            memset(*arg, 0, sizeof(arg)); 
        }           
    }   

    if (b==1){

        if(input[width-1]!='&'){
            printf("ERROR: background process only applies to last command \n");
            error=1;
        }
    }

    if(ri==1){
        if(redirection_in!=0){
            printf("ERROR: redirection input only applies to first command \n");
            error=1;
        }        
    }

    if(ro==1){
        if(redirection_out!=iter-1){
            printf("ERROR: redirection output only applies to last command \n");
            error=1;
        }        
    }


    *struct_length=iter;

    int j=0;
        int counter=0;
        while(counter < *struct_length){
        //printf("command %i \n", counter);
        j=0;
        while ((list+counter)->argv[j]!=NULL){

            printf("command %i ", counter);

            printf("argv %i: %s \n", j, (list+counter)->argv[j]);
            j++;
        }
        counter++;
        }
    return error;
}

int custom_comm(char** arg){

    int success;
    if (strcmp(arg[0],"cd") == 0){
        if (arg[1]==NULL){
            printf("ERROR: no argument in custom args\n");
        }
        else if(arg[1]!=NULL){
            success=chdir(arg[1]);
            if(success!=0){
                printf("ERROR: cannot execute cd\n");
            }
        }
        else{
            printf("ERROR: error in command cd\n");
        }
        
    }
    else if(strcmp(arg[0],"exit")==0){
        printf("exit\n");
        exit(0);
        if(success!=0){
             printf("ERROR: cannot execute exit\n");
        }        
    }
    else{
        printf("ERROR: error in command exit\n");
    }

    return 1; // if successful
}

int execute_com(char** arg, int in) {
	pid_t pid;
    int wstatus;
    //int exec_success;
    
    pid = fork();
	if (pid > 0) {//parent

        pid = waitpid(pid, &wstatus, 0); //-1 means any child
	}
	else if (pid==0){//child
        if (in !=0){
            dup2(in, 0);
            
        
        }

        if (strcmp(arg[0], "cd")==0 || strcmp(arg[0], "exit")==0){

            custom_comm(arg);
        }
        else{
            if( execvp(arg[0], arg)<0){
                printf("ERROR: cannot perform simple exec \n");
                exit(EXIT_FAILURE);
            }
        } 
    }
    else{//error
        //printf("Oops something went wrong ...\n");
        perror("Error: error with pid \n");
    }
    
    return 1; //if successful
}

int pipe_connect (int in, int out, char** argv)
{
  pid_t pid;
  //int wstatus; 
  pid = fork ();
  if ( pid== 0)
    {
      if (in != 0)
        {
          dup2 (in, 0);
          close (in);
        }

      if (out != 1)
        {
          dup2 (out, 1);
          close (out);
        }
        
        execvp (argv [0], argv);
    }
    else if(pid>0){
        printf("parent is waiting in simple connect \n");
        wait(NULL);
    }
    else{
        printf("ERROR: error with pipe connection \n");
        exit(EXIT_FAILURE);
    }
    
  return pid;
}

int redirect_out(int in, int out, char** argv, char* file){
  pid_t pid;
  //int wstatus; 
  int fod;

  pid = fork ();
  if ( pid== 0)
    {
        if ((fod=open(file, O_WRONLY)) <0){
            printf("couldn't open output file \n");
            printf("ERROS: %i \n", errno);

            printf("%s \n" , strerror(errno));
            printf("%i \n", fod);
            exit(0);
        }

        if (in != 0)
        {
          dup2 (in, 0);
          close (in);
        }

      if (out != 1)
        {
          dup2 (out, 1);
          close (out);
        }

        close(1);
        dup2(fod, 1); 
        close(fod);

        // printf("argument of command: %s \n", argv[0]);
        if(execvp(argv[0], argv)<0){
            printf("ERROR: cannot execute writing \n");
            exit(EXIT_FAILURE);
        }
    }
    else if(pid>0){
        printf("parent of redirection is waiting \n");
        wait(NULL);
    }
    else{
        printf("ERROR: cannot execute redirection \n");
        exit(EXIT_FAILURE);
    }

  return pid;
}

int redirect_in(int in, int out, char** argv, char* file){
    pid_t pid;
  //int wstatus; 
  int fod;

  pid = fork ();

//   printf("FILE: %s \n", file);
  if ( pid== 0)
    {
        // if ((fod=open(file, O_WRONLY)==-1))
        // { 
        //     printf("errors\n");
        //     exit( EXIT_FAILURE);(fod=creat(file, 0644))
        // }
        if ((fod=open(file, O_RDONLY)) <0){
            printf("couldn't open output file \n");
            printf("ERROS: %i \n", errno);

            printf("%s \n" , strerror(errno));
            printf("%i \n", fod);
            exit(0);
        }

        if (in != 0)
        {
          dup2 (in, 0);
          close (in);
        }

        if (out != 1)
        {
          dup2 (out, 1);
          close (out);
        }

        close(0);
        dup2(fod, 0); 
        close(fod);

        // printf("argument of command: %s \n", argv[0]);
        if(execvp(argv[0], argv)<0){
            printf("ERROR: cannot execute writing \n");
            exit(EXIT_FAILURE);
        }

        //close(fod);


    }
    else if(pid>0){
        printf("parent of redirection is waiting \n");
        wait(NULL);
    }
    else{
        printf("ERROR: canot execute redirection \n");
        exit(EXIT_FAILURE);
    }

  return pid;
}

void signal_handler(int signal){
    int wstatus;
    pid_t orphan_child;
    //printf("orphan\n");
    while((orphan_child=waitpid(-1, &wstatus, WNOHANG))>0){
        //printf("Reaping the Orphans");
        
    }
    //wait(NULL);

    //return orphan_child;
}

int launch_back(char** arg, int in) {
	pid_t pid;
    //int wstatus;
    //int exec_success;
    
    pid = fork();
	if (pid > 0) {//parent
        printf("Background process running \n");
        signal(SIGCHLD, signal_handler);
        //signal(SIGCHLD, SIG_IGN);
        //while(1);
        //pid = waitpid(pid, &wstatus, 0); //-1 means any child
	}
	else if (pid==0){//child
        if (in !=0){
            dup2(in, 0);
        }
 
        if (strcmp(arg[0], "cd")==0 || strcmp(arg[0], "exit")==0){

            custom_comm(arg);
        }
        else{
            if( execvp(arg[0], arg)<0){
                printf("ERROR: cannot perform simple exec \n");
                exit(EXIT_FAILURE);
            }
        } 
    }
    else{//error
        //printf("Oops something went wrong ...\n");
        perror("Error: error with pid \n");
    }
    
    return 1; //if successful
}

int launch_prog(struct command* list, int struct_length){
    int counter=0;

    // int pid1, pid2;
    // int prev=0;
    // int next=1;

    //pid_t pid;
    int in, fd[2];
    in =0;
    // int i=0;
    // int j=0;

    //int fod;

    //close(fd[0]);
    //printf("%i \n", struct_length);
    while(counter< struct_length){
        printf("command %i \n", counter);
        int j=0;
        while ((list+counter)->argv[j]!=NULL){
            printf("command %i ", counter);
            printf("purpose %i ", (list+counter)->redirect_out);

            printf("argv %i: %s \n", j, (list+counter)->argv[j]);


            j++;
        }

        if (list[counter].pipe==1){
            //printf("a\n");
  
            pipe(fd);  

            pipe_connect (in, fd [1], (list+counter)->argv);

            close(fd[1]);
            
            in = fd[0];

        }
        else if(list[counter].redirect_out==1){// '>'
            printf("b\n");
            pipe(fd);
            redirect_out(in, fd[1], (list+counter)->argv, (list+counter+1)->argv[0]);

            close(fd[1]);
            in =fd[0];
  
            counter++;
        }
        else if(list[counter].redirect_in==1){// '<'
            printf("c\n");
            pipe(fd);
            redirect_in(in, fd[1], (list+counter)->argv, (list+counter+1)->argv[0]);
            close(fd[1]);
            in =fd[0];
         
            counter++;
        }
        else if(list[counter].back==1){
            printf("d\n");
            launch_back((list+counter)->argv, in);
        }
        else{
            //printf("else \n");
            execute_com((list+counter)->argv, in);
        }

        counter++;
    }
    return 1;
}

int empt_argv(struct command* list, int length){
    // int word_length=32;
    // int arg_max=32;
    // int line_max=512;
    int i;
    for (i=0; i< length; i++){
        list[0].argv=0;
    }
    return 1;
}

int main(int argc, char** argv) {

	
        char* line;
        //char* line2;
		//char** arg;
        int* length=malloc(sizeof(int));
        //char** command;
        //char** parameter;
        //int cmd_max=32;
        int com_err=0;
        struct command list[32]; 
        int* struct_length=malloc(sizeof(int));
        //int flag=0;
        //char c;

        int n_flag=0;
        // char* flags;
        int i;
        if(argc>1){
            for (i=1; i<argc; i++){
                if (strcmp(argv[i], "-n")==0){
                    n_flag=1;
                    //flags=argv[i];
                }
            }
        }
    
	while (true) {// check status
		alloc_argv(list, 32);

        if (n_flag==0){
            prompt();
        }

        line = input_read(length);
        printf("%s \n", line);
        *struct_length=0;

        com_err=construct_commands(line, list, *length, struct_length);
        
        // int j=0;
        // int counter=0;
        // while(counter < *struct_length){
        // //printf("command %i \n", counter);
        // j=0;
        // while ((list+counter)->argv[j]!=NULL){

        //     printf("command %i ", counter);

        //     printf("argv %i: %s \n", j, (list+counter)->argv[j]);
        //     j++;
        // }
        // counter++;
        // }

        if(com_err==1){
            continue;
        }
        launch_prog(list, *struct_length);
        
        

        empt_argv(list, 32);        
        
        line=memset(line, 0, strlen(line));

        
	}
	
	return 1;
}