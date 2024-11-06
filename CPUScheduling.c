#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100

typedef struct {
    int process_num;
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time;
} Process;

//Read & write to file
/*******************************************************************************/
void read_processes(char *filename, Process processes[], char *algorithm, int *time_quantum, int *num_processes){
    FILE *fp = fopen(filename, "r");
    if (fp == NULL){
        perror("Error opening input file.");
        exit(1);
    }

    fscanf(fp, "%s", algorithm);
    if(strcmp(algorithm, "RR") == 0){
        fscanf(fp, "%d", time_quantum);
    }
    fscanf(fp, "%d", num_processes);

    for(int i = 0; i < *num_processes; i++){
        fscanf(fp, "%d %d %d %d", &processes[i].process_num, &processes[i].arrival_time, 
               &processes[i].burst_time, &processes[i].priority);
        processes[i].remaining_time = processes[i].burst_time;
    }

    fclose(fp);
}

int write_output(char *algorithm, int schedule[][2], int schedule_size, float avg_waiting_time){
    FILE *fp = fopen("output.txt", "w");
    if (fp == NULL){
        perror("Error opening output file.");
        exit(1);
    }

    fprintf(fp, "%s\n", algorithm);
    for(int i = 0; i < schedule_size; i++){
        fprintf(fp, "%d %d\n", schedule[i][0], schedule[i][1]);
    }
    fprintf(fp, "%.2f\n", avg_waiting_time);

    fclose(fp);
}
/*******************************************************************************/

float calculate_avg_waiting_time(int waiting_time[], int num_processes){
    int total_waiting_time = 0;

    for(int i = 0; i < num_processes; i++){
        total_waiting_time += waiting_time[i];
    }

    return (float)total_waiting_time / num_processes;
}


//Scheduling algorithms
/*******************************************************************************/
void round_robin(Process processes[], int num_processes, int time_quantum, int schedule[][2], int *schedule_size){
    int current_time = 0;
    int completed = 0;
    int *queue = (int *)malloc(sizeof(int) * num_processes);
    int front = 0, rear = -1;

    for(int i = 0; i < num_processes; i++){
        if(processes[i].arrival_time == 0){
            queue[++rear] = i;
        }
    }

    while(completed < num_processes){
        if(front <= rear){
            int current_process = queue[front++];
            int time_slice = 0;

            while(time_slice < time_quantum && processes[current_process].remaining_time > 0){
                schedule[(*schedule_size)][0] = current_time;
                schedule[(*schedule_size)++][1] = processes[current_process].process_num;

                processes[current_process].remaining_time--;
                current_time++;
                time_slice++;

                for(int i = 0; i < num_processes; i++){
                    if(processes[i].arrival_time == current_time){
                        queue[++rear] = i;
                    }
                }
            }

            if(processes[current_process].remaining_time > 0){
                queue[++rear] = current_process;
            }
            else completed++;
        }
        else{
            current_time++;
            for(int i = 0; i < num_processes; i++){
                if(processes[i].arrival_time == current_time){
                    queue[++rear] = i;
                }
            }
        }
    }

    free(queue);
}

void calculate_waiting_time_RR(Process processes[], int num_processes, int schedule[][2], int schedule_size, int waiting_time[]){
    int *completion_time = (int *)malloc(sizeof(int) * num_processes);

    for(int i = 0; i < num_processes; i++){
        completion_time[i] = 0;
    }

    for(int i = schedule_size - 1; i >= 0; i--){
        int process_num = schedule[i][1] - 1;
        if(completion_time[process_num] == 0){
            completion_time[process_num] = schedule[i][0];
        }
    }

    for(int i = 0; i < num_processes; i++){
        waiting_time[i] = completion_time[i] - processes[i].arrival_time - processes[i].burst_time;
    }

    free(completion_time);
}
/*******************************************************************************/

int main(){
    Process processes[MAX_PROCESSES];
    char algorithm[20];
    int time_quantum;
    int num_processes;
    int schedule[MAX_PROCESSES * 10][2];
    int schedule_size = 0;
    int waiting_time[MAX_PROCESSES];
    float avg_waiting_time;

    read_processes("input1.txt", processes, algorithm, &time_quantum, &num_processes);
    round_robin(processes, num_processes, time_quantum, schedule, &schedule_size);
    //printf("%d\n", num_processes);
    //calculate_waiting_time_RR(processes, num_processes, schedule, schedule_size, waiting_time);
    //avg_waiting_time = calculate_avg_waiting_time(waiting_time, num_processes);
    //write_output(algorithm, schedule, schedule_size, avg_waiting_time);
    
}