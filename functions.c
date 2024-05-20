#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>
#include <unistd.h>
#include <utmp.h>
#include <stdbool.h>
#include <ctype.h> 
# include "functions.h"

#define MAX_LINE_LEN 1024

/*        The <sys/resource.h> header shall define the following symbolic
    machine architecture
    time that the system has been running since its last reboot
*/
void system_info()
{

    struct utsname system_data;
    if (uname(&system_data) < 0)
    { // This is to check if the  the system data was fetched
        fprintf(stderr, "There was an error fetching the system information./n");
        return ;
    }
    printf( "### System Information ###\n");
    printf("System Name = %s\n", system_data.sysname);

    printf("Machine Name = %s\n", system_data.machine);

    printf("Version = %s\n", system_data.version);

    printf("Release = %s\n", system_data.release);

    printf("Architec = %s\n", system_data.machine);

    // This segment is for reading from the uptime file and then printing the time

    FILE *fileptr = NULL;
    char line[MAX_LINE_LEN];
    char *uptime_ch = NULL;

    fileptr = fopen("/proc/uptime", "r");

    if (fileptr == NULL)
    { //  need to think about how i am going to deal with stuff like this 
        fprintf(stderr, "There was an error in opening the file");
        return ;
    }

    fgets(line, MAX_LINE_LEN, fileptr);
    uptime_ch = strtok(line, " ");
    int uptime = atoi(uptime_ch);
    int days = uptime / (24 * 60 * 60);
    uptime -= days * (24 * 60 * 60);
    int hours = (uptime) / (60 * 60);
    uptime -= hours * (60 * 60);
    int minutes = uptime / 60;
    uptime -= minutes * 60;
    int seconds = uptime;

    printf("System running since last reboot: ");
    printf("%d days %d:%d:%d", days, hours, minutes, seconds);
    printf("(%d:%d:%d)\n", days * 24 + hours, minutes, seconds);
    printf("---------------------------------------\n");
    fclose(fileptr);     
}

void user_usage(int fd){
    struct utmp *user_usage;
    user_usage = NULL;
    setutent();
    char buffer[MAX_LINE_LEN];
    sprintf(buffer, "###Session/Users###\n");
    write(fd, buffer, MAX_LINE_LEN); 


    while ((user_usage = (getutent())) != NULL)
    { 
        if ((user_usage->ut_type) == USER_PROCESS)
        {  
            sprintf(buffer, "  %s\t %s\t %s\n", user_usage->ut_user, user_usage->ut_line, user_usage->ut_host); 
            write(fd, buffer,MAX_LINE_LEN );
        }
    }
    endutent(); 
    sprintf(buffer,"---------------------------------------\n"); 
    write(fd, buffer,MAX_LINE_LEN);
}

int parameters(int nbr_samples, int freq)
{
    /*This function prints 3 statements */
    printf("Nbr of sampels %d -- every %d secs \n", nbr_samples, freq);
    struct rusage self_usage;
    getrusage(RUSAGE_SELF, &self_usage);
    if (getrusage(RUSAGE_SELF, &self_usage) != 0)
    {
        fprintf(stderr,"There was an error getting the memory of the application.\n");
    }
    else
    {
        printf("Memory usage: %ld kilobytes\n", self_usage.ru_maxrss);
    }
    printf("---------------------------------------\n");
    return 0;
}

void system_usage(int fd)
{
    float memory[4];
    char memUse[MAX_LINE_LEN]; 
    struct sysinfo info;
    int gig_conv = 1000000000;
    sysinfo(&info);
    if (sysinfo(&info) < 0)
    {
        fprintf(stderr, "There was en error in fetching the system usage");
    }
    memory[3] = (info.totalram + info.totalswap) / gig_conv;
    memory[2] = ((info.totalram + info.totalswap) - (info.freeram + info.freeswap)) / gig_conv;
    memory[1] = info.totalram / gig_conv;
    memory[0] = (info.totalram - info.freeram) / gig_conv;
    sprintf(memUse,"%.2f GB / %.2f GB -- %.2f GB / %.2f GB\n", memory[0], memory[1], memory[2], memory[3]);
    write (fd, memUse, MAX_LINE_LEN); 
}

 
  /*
    THis is helper function of cpu_info that calculates the cpu usage using the /proc/stat file  and returns the cpu usage as a percentage
    Credit : INDT "Gathering cpu utilization from /proc/stat"
    
    The cpu calculations specified in the instruction are: 
    total cpu utilization time: Ti = useri + nicei + sysi + idlei + IOwaiti + irqi + softirqi 
    idle time: Ii = idlei  
    Ui = Ti - Ii 

    CPU utilization are (U2-U1)/(T2-T1) X 100  

    */
float get_cpu_useage()
{
    FILE *stat_ptr = NULL;
    double cpu_info[7]; 
    double cpu_info_curr[7];
    double T1= 0;// equivalent to the T1 from the formula given in the instructuons   
    double T2 = 0;
    stat_ptr = fopen("/proc/stat", "r");
    if (stat_ptr == NULL)
    {
        fprintf(stderr, "Failed to read /proc/stat file");
        return -1;
    }
    fscanf(stat_ptr, "%*s %le %le %le %le %le %le %le", &cpu_info[0], &cpu_info[1], &cpu_info[2], &cpu_info[3], &cpu_info[4], &cpu_info[5], &cpu_info[6]);
    sleep(1/3); 
    fscanf(stat_ptr, "%*s %le %le %le %le %le %le %le", &cpu_info_curr[0], &cpu_info_curr[1], &cpu_info_curr[2], &cpu_info_curr[3], &cpu_info_curr[4], &cpu_info_curr[5], &cpu_info_curr[6]);

    fclose(stat_ptr);
    for (int i = 0; i < 7; i++)
    {
        T1 += cpu_info[i];
        T2 += cpu_info_curr[i];
        
    }

    double I1 = cpu_info[3];
    double I2 = cpu_info_curr[3];
    double U1 = T1 - I1;  
    double U2 = T2 - I2; 
    double result = ((U2 - U1)/(T2-T1)) *100;
    return result;
    printf("%.2f\n", result);

    
}


/*
This fucntion prints opens cpuinfo file and then print the number of cores from that file
*/

void cpu_info(int fd)
{
  
    FILE *cpu_data = NULL;
    char line[MAX_LINE_LEN];
    char buffer[MAX_LINE_LEN]; 
    char *num_cores = NULL;
    cpu_data = fopen("/proc/cpuinfo", "r");
    if (cpu_data == NULL)
    {
        fprintf(stderr, "Failed to open  cpuinfo file \n");
    }

    while (fgets(line, MAX_LINE_LEN, cpu_data) != NULL)
    {
        num_cores = strtok(line, ":");
        if (strncmp(num_cores, "cpu cores", 9) == 0)
        {
            num_cores = strtok(NULL, ":");
            break;
        }
    }
    sprintf(buffer,"Number of cores:%s", num_cores); 
    write(fd, buffer, MAX_LINE_LEN);
    
    fclose(cpu_data);

    sprintf(buffer,"The CPU usage is: %.2f%%\n", get_cpu_useage());
    write(fd, buffer, MAX_LINE_LEN);  
    sprintf(buffer, "---------------------------------------\n");
    write(fd, buffer, MAX_LINE_LEN); 
}

bool is_num(const char *str)
{
    // Check if the string is empty
    if (str == NULL || *str == '\0')
    {
        return false;
        // Return false if the string is empty
    }

    // Iterate through each character of the string
    int i = 0;
    while (*(str + i) != '\0')
    {
        // If any character is not a digit, return false
        if (!isdigit(*str))
        {
            return false;
        }
        i++; // Move to the next character
    }

    // If all characters are digits, return true
    return true;
}

void zero(char memory[][1024], int nbr_samples)
{
    for (int i = 0; i < nbr_samples; i++)
    { 
        memory[i][0] = '\n';
        memory[i][1] = '\0';
    }
}

 
 
void ParseArguments( int argc, char** argv, int* arguements){ 
    int numofArgs = argc;
    int system = 0;
    int user = 0;
    int sequential = 0;
    int freq = 1;
    int sampels = 10; 
    int comp_Facts = 0;

    if (argc<2){ 
        arguements[0] = 0;
        arguements[1] = 1 ; 
        arguements[2] = 1; 
        arguements[3] = freq;
        arguements[4] = sampels;
        arguements[5] = 1; 
        return;
    }

    
    if (argc > 2)
    {
        // this if statment checks if the last two arguements are numbers if they are it makes those
        // the numb// end of the multiple argument if statmenter of samples and frequency
        if (is_num(argv[argc - 2]) == true && is_num(argv[argc - 1]) == true)
        {
            sampels = atoi(argv[argc - 2]);
            freq = atoi(argv[argc - 1]);
            numofArgs = argc - 2; 
        }
    }
    // this is the main function it takes in the command line arguments and decides which functions to call
    if (argc > 1)
    {
        for (int i = 1; i < numofArgs; i++)
        {
            char *command = NULL;
            command = strtok(argv[i], "=");
            // do come code to isolate for samples and tdelay then check if they are called and if they are write some more code for the value inserted
            if (strcmp(argv[i], "--system") == 0) 
            {
                system = 1;
            } 
            
            else if (strcmp(argv[i], "--user") == 0)
            { 
                
                user = 1;
            }
            else if (strcmp(argv[i], "--sequential") == 0)
            { 
                
                sequential = 1;
            }

            else if (strcmp(command, "--tdelay") == 0)
            { 
                
                command = strtok(NULL, "=");
                int delay = atoi(command);
                if (delay != 0)
                {
                    freq = delay;
                }
            }
            else if (strcmp(command, "--samples") == 0)
            { 
                
                command = strtok(NULL, "=");
                int smple = atoi(command);
                if (smple == 0)
                {
                    sampels = 10;
                }
                else
                {
                    sampels = smple;
                }
            }
            else
            {
                fprintf(stderr, "There was an unrecognized command. \n");
                break;
            }

        } // end of the for loop checking for arguments
    } 

    if ( user==1 || system ==1 ){
        comp_Facts =0; 
    } 
    arguements[0] = sequential;
    arguements[1] = system ; 
    arguements[2] = user; 
    arguements[3] = freq;
    arguements[4] = sampels;
    arguements[5] = comp_Facts;

}






// need to change the fucntions that write to an array and return the array instead of printing directly  
// also make functions that can be called to print those arrays  

// for CPU_info I need to make a pipe that will read info from the parent as well as writing info to the pipe

// the idea is that we make the pipe, then when we need to call the functions we fork wait to get back an array of info and then read the information 
