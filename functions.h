#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED


void system_info(); 
void user_usage(int fd); 
void cpu_info(int fd);
int parameters(int nbr_samples, int freq); 
void system_usage(int fd); 
float get_cpu_useage(); 
void zero(char memory[][1024], int nbr_samples);
bool is_num(const char *str); 
void ParseArguments( int argc, char** argv, int* arguements);

#endif