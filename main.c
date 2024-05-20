# define _POSIX_C_SOURCE 200809L
// These are the libraries 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <stdbool.h>
#include <ctype.h>
#include<signal.h>
#include<unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>

// this links to the functions in funcitons.c 
# include "functions.h"
#define MAX_LINE_LEN 1024



/* 
This is a helper function for main that handles all the cases that involve a combination of the flags --sequential and --users/--system
*/
int print_sequential(int samples, int freq, int users, int system)

{  
    float *CPU_Usage  = malloc(sizeof(int)*7); 
    if (CPU_Usage == NULL){ 
        fprintf(stderr, "Space for the CPU_Usage function cannot be made.\n");
    }
    // Let's make a few arrays for the pipes 
    for (int i = 0; i < samples; i++)
    {
        fprintf(stdout, "Iteration %d>>>\n", i);
        // this will be the case of when all three flags are called at the same time
        if (system != 0 && users != 0)
        {
            // float memory[samples][4];
            // zero(memory, samples);
            // int param = parameters(samples, freq);
            // int sys_us = system_usage(memory, samples, i);
            //int users = user_usage();
        }

        // case wehre  onlt sequential and system are called a
        else if (system != 0)
        {
            // float memory[samples][4];
            // zero(memory, samples);
            // int param = parameters(samples, freq);
            // int sys_us = system_usage(memory, samples, i);
        }

        // case where only sequential and users are called
        else if (users != 0)
        {
            // float memory[samples][4];
            // zero(memory, samples);
            // int param = parameters(samples, freq);
            //int users = user_usage();
        }

        // the case where only sequential is called and so it calls all information and just formats it in sequential
        else
        {
            // float memory[samples][4];
            // zero(memory, samples);
            // int param = parameters(samples, freq);
            // int sys_us = system_usage(memory, samples, i);
            // //int users = user_usage();
            // int cpu = cpu_info(CPU_Usage);
            // int sys = system_info();
        }

        sleep(freq);
    }
    return 0;
}
void ctrl_z_handler() {
    // do nothing
}

void ctrl_c_handler() {
    char choice;
    fprintf(stdout, "Ctrl-C detected. Do you want to quit the program? 'y'(yes) or 'n'(no)\n");
    scanf(" %c", &choice);
    if (choice == 'y') {
        fprintf(stdout, "Exiting program.\n");
        exit(EXIT_SUCCESS);
    } 

    printf("Continuing program execution.\n");
    
}  


int main(int argc, char **argv)
{ 
    
    int system = 0;
    int user = 0;
    int sequential = 0;
    int freq = 1;
    int samples = 10;  


    int* arguements =  calloc(6,sizeof(int));   
    if (arguements== NULL){
        fprintf(stderr, "Could not malloc space for arguements\n");
    }  

    ParseArguments(argc, argv, arguements); 
    sequential = arguements[0];
    system = arguements[1] ; 
    user = arguements[2]; 
    freq = arguements[3];
    samples =  arguements[4]; 
    int compFacts = arguements[5];
    //arguements[5] = 0;  



    // this is the new handler for ctrl z
    struct sigaction zSignal;
    zSignal.sa_handler = ctrl_z_handler;
    zSignal.sa_flags = 0;
    sigemptyset(&zSignal.sa_mask);
    sigaction(SIGSTOP,&zSignal,NULL);    

    // this is the new handler for ctrl c
    struct sigaction cSignal;
    cSignal.sa_handler = ctrl_c_handler;
    cSignal.sa_flags = 0;
    sigemptyset(&cSignal.sa_mask);
    sigaction(SIGINT,&cSignal,NULL);  



    char memoryLines [samples][1024]; 
    zero(memoryLines, samples); 
    
    // this is the array of our pipes 
    int pipefds[3][2];    
    //this is to store our child PIDS 
    pid_t childPids[3];
    char buffer[MAX_LINE_LEN];
    
// THis is the over archinf for loop that will contain our freq and samples
    for (int j = 0; j<samples; j++){
         if (sequential !=0){
            printf("<<<<<<Iteration: %d>>>>>>>>\n", j);
        }  
        parameters(samples, freq); 
        
         // THIS IS WHERE WE CREATE OUR PIPES
        for (int i = 0; i < 3; i++) {
            if (pipe(pipefds[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        } 


    // this is our for loop where we will fork and write and read all our information for one iteration of sample 

        for (int i = 0; i < 3; i++) {
        childPids[i] = fork();
        if (childPids[i] == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } 
        else if (childPids[i] == 0) {  // Child process
            // Close read end of the pipe 
            close(pipefds[i][0]); 
            // here I will have a decision tree as what to write  
            if ( i== 0 ){ 
                // this the logic branch as to where we will launch the memory usgae function 
                system_usage(pipefds[i][1]); 
            }
            if (i==1){ 
                // THIS IS THE LOGICAL BRANCH FOR SESSIONS AND USERS

                user_usage(pipefds[i][1]); // calling the funciton and giving it the writing end 

            }
            if (i==2){
                // THIS IS THE LOGICAL BRANCH FOR CPU USE 
                cpu_info(pipefds[i][1]);

            }

            // close the write end of the pipe and then exit for the child 
            close(pipefds[i][1]); 
            exit(EXIT_SUCCESS);
        }  
         // now we are in the parent process 
        waitpid(childPids[i], NULL, 0);

        close(pipefds[i][1]);// want to close the reading end so the parent can read

        if (i!=0){
            while (read(pipefds[i][0], buffer, sizeof(buffer)) > 0){ // this meanns if it isnt the memory usage one 
                if (i==1 && user== 1){ 
                    printf("%s", buffer); 
                }  
                if (i==2 && system ==1){ 
                    printf("%s", buffer);
                }
            }
        } 
          if (i==0){  
            read(pipefds[i][0], buffer, sizeof(buffer));
            if (sequential != 0){ 
                zero(memoryLines,samples);
            }
            strcpy(memoryLines[j], buffer);  
            if (system !=0){  
                printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n"); 
                for(int l = 0; l<samples; l++){ 
                    printf("%s", memoryLines[l]); 
                }  
                printf("---------------------------------------\n");

            }
           
             
            
        }
        // Close the read end of the pipe
        close(pipefds[i][0]);

    }// end of the inner for loop
    if (compFacts!=0){ 
        system_info();
    }
     
    sleep(freq); 
        if (sequential==0){ 
            if (j== samples-1){
                break;
            }
            printf("\033[H\033[J");

        }



    }//end of the outer for loop

    return 0;
}



/*
TODO LIST: 

-impliment graphics, I have to figure out what i even need to do for this function and then impliment it  
 
-when the signal is interupts make sure to kill all the chikd porocesses 

-include error-checking for any possible circumstance where resources to be requested from the system may not be available and report them to standard error.
-there should be one process launched per function reporting memory utilization, connected users and CPU utilization. Pipes have to be used to communicate the information to the main function
the order of the output should be the same as A1 

so this is relatuively straight forward where we make our pipes in main
then we call our function in our child processes then exit the child process and then print all the information we read from the child process   

*/

