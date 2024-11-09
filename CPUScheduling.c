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
    int completion_time;
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
        fscanf(fp, "%d %d %d %d", &processes[i].process_num, &processes[i].arrival_time, &processes[i].burst_time, &processes[i].priority);
        processes[i].remaining_time = processes[i].burst_time;
    }

    fclose(fp);
}

int write_output(char *algorithm, int time_quantum, int schedule[][2], int schedule_size, float avg_waiting_time){
    FILE *fp = fopen("output.txt", "w");
    if (fp == NULL){
        perror("Error opening output file.");
        exit(1);
    }

    if(strcmp(algorithm, "RR") == 0){
        fprintf(fp, "%s %d\n", algorithm, time_quantum);
    }
    else fprintf(fp, "%s\n", algorithm);
    
    for(int i = 0; i < schedule_size; i++){
        fprintf(fp, "%d %d\n", schedule[i][0], schedule[i][1]);
    }
    fprintf(fp, "AVG Waiting Time: %.2f\n", avg_waiting_time);

    fclose(fp);
}
/*******************************************************************************/

//Priority queue as binary heap
/*******************************************************************************/
typedef struct {
    int process_index;
    int burst_time;
    int arrival_time;
    int priority;
    int process_num;
} PQNode;

void swap(PQNode *a, PQNode *b){
    PQNode temp = *a;
    *a = *b;
    *b = temp;
}

void heapify(PQNode pq[], int n, int i, char *key){
    int smallest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if(strcmp(key, "burst_time") == 0){
        if(left < n && pq[left].burst_time < pq[smallest].burst_time){
            smallest = left;
        }
        if(right < n && pq[right].burst_time < pq[smallest].burst_time){
            smallest = right;
        }
        if(left < n && pq[left].burst_time == pq[smallest].burst_time && pq[left].arrival_time < pq[smallest].arrival_time){
            smallest = left;
        }
        if(right < n && pq[right].burst_time == pq[smallest].burst_time && pq[right].arrival_time < pq[smallest].arrival_time){
            smallest = right;
        }
        if(left < n && pq[left].burst_time == pq[smallest].burst_time && pq[left].arrival_time == pq[smallest].arrival_time && pq[left].process_num < pq[smallest].process_num){
            smallest = left;
        }
        if(right < n && pq[right].burst_time == pq[smallest].burst_time && pq[right].arrival_time == pq[smallest].arrival_time && pq[right].process_num < pq[smallest].process_num){
            smallest = right;
        }
    } else if(strcmp(key, "priority") == 0){
        if (left < n && pq[left].priority < pq[smallest].priority) {
            smallest = left;
        }
        if (right < n && pq[right].priority < pq[smallest].priority) {
            smallest = right;
        }
        if (left < n && pq[left].priority == pq[smallest].priority && pq[left].burst_time < pq[smallest].burst_time) { 
            smallest = left;
        }
        if (right < n && pq[right].priority == pq[smallest].priority && pq[right].burst_time < pq[smallest].burst_time) {
            smallest = right;
        }
        if (left < n && pq[left].priority == pq[smallest].priority && pq[left].burst_time == pq[smallest].burst_time && pq[left].process_num < pq[smallest].process_num) {
            smallest = left;
        }
        if (right < n && pq[right].priority == pq[smallest].priority && pq[right].burst_time == pq[smallest].burst_time && pq[right].process_num < pq[smallest].process_num) {
            smallest = right;
        }
    }

    if(smallest != i){
        swap(&pq[i], &pq[smallest]);
        heapify(pq, n, smallest, key);
    }
}

PQNode extract_min(PQNode pq[], int *n, char *key){
    PQNode min = pq[0];
    pq[0] = pq[(*n) - 1];
    (*n)--;
    heapify(pq, *n, 0, key);
    return min;
}

void insert(PQNode pq[], int *n, PQNode new_node){
    (*n)++;
    int i = (*n) - 1;
    pq[i] = new_node;

    while(i != 0 && pq[(i - 1) / 2].burst_time > pq[i].burst_time){
        swap(&pq[i], &pq[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}
/*******************************************************************************/

//Round Robin
/*******************************************************************************/
void round_robin(Process processes[], int num_processes, int time_quantum, int schedule[][2], int *schedule_size){
    int current_time = 0;
    int completed = 0;
    int queue_capacity = num_processes;
    int *queue = (int *)malloc(sizeof(int) * queue_capacity);
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

            schedule[(*schedule_size)][0] = current_time; 
            schedule[(*schedule_size)++][1] = processes[current_process].process_num;

            while(time_slice < time_quantum && processes[current_process].remaining_time > 0){
                processes[current_process].remaining_time--;
                current_time++;
                time_slice++;

                for(int i = 0; i < num_processes; i++){
                    if(processes[i].arrival_time == current_time){
                        if(rear == queue_capacity - 1){
                            queue_capacity *= 2;
                            queue = realloc(queue, sizeof(int) * queue_capacity);
                            if(queue == NULL){
                                perror("Error reallocating memory for queue.");
                                exit(1);
                            }
                        } 
                        queue[++rear] = i;
                    }
                }
            }

            if(processes[current_process].remaining_time > 0){
                if(rear == queue_capacity - 1){
                    queue_capacity *= 2;
                    queue = realloc(queue, sizeof(int) * queue_capacity);
                    if(queue == NULL){
                        perror("Error reallocating memory for queue.");
                        exit(1);
                    }
                }
                queue[++rear] = current_process;
            }
            else{
                completed++;
                processes[current_process].completion_time = current_time;
            }
        }
        else{
            current_time++;
            for(int i = 0; i < num_processes; i++){
                if(processes[i].arrival_time == current_time){
                    if(rear == queue_capacity - 1){
                        queue_capacity *= 2;
                        queue = realloc(queue, sizeof(int) * queue_capacity);
                        if(queue == NULL){
                            perror("Error reallocating memory for queue.");
                            exit(1);
                        }
                    }
                    queue[++rear] = i;
                }
            }
        }
    }

    free(queue);
}
/*******************************************************************************/

//Shortest Job First
/*******************************************************************************/
void shortest_job_first(Process processes[], int num_processes, int schedule[][2], int *schedule_size){
    int current_time = 0;
    int completed = 0;
    PQNode pq[MAX_PROCESSES];
    int pq_size = 0;

    while(completed < num_processes){
        for(int i = 0; i < num_processes; i++){
            if(processes[i].arrival_time <= current_time && processes[i].remaining_time > 0){
                PQNode new_node = {i, processes[i].burst_time, processes[i].arrival_time, processes[i].priority, processes[i].process_num};
                insert(pq, &pq_size, new_node);
                
                processes[i].remaining_time = 0;
            }
        }

        if(pq_size > 0){
            PQNode current_node = extract_min(pq, &pq_size, "burst_time");
            int current_process = current_node.process_index;

            schedule[(*schedule_size)][0] = current_time;
            schedule[(*schedule_size)++][1] = processes[current_process].process_num;

            current_time += processes[current_process].burst_time;

            completed ++;
            processes[current_process].completion_time = current_time;
        }
        else current_time++;
    }
}
/*******************************************************************************/

//Priority w/o Preemption
/*******************************************************************************/
void priority_no_preemption(Process processes[], int num_processes, int schedule[][2], int *schedule_size){
    int current_time = 0;
    int completed = 0;
    PQNode pq[MAX_PROCESSES];
    int pq_size = 0;
    int current_process = -1;

    while(completed < num_processes){
        int highest_priority = 9999;
        int next_process = -1;

        for(int i = 0; i < num_processes; i++){
            if(processes[i].arrival_time <= current_time && processes[i].remaining_time > 0 && processes[i].priority < highest_priority){
                highest_priority = processes[i].priority;
                next_process = i;
            } else if(processes[i].arrival_time <= current_time && processes[i].remaining_time > 0 && processes[i].priority == highest_priority && processes[i].process_num < processes[next_process].process_num){
                next_process = i;
            }
        }

        if(next_process != -1){
            PQNode new_node = {next_process, processes[next_process].burst_time, processes[next_process].arrival_time, processes[next_process].priority, processes[next_process].process_num};
            insert(pq, &pq_size, new_node);
            processes[next_process].remaining_time = 0;
        }

        if(pq_size > 0){
            PQNode current_node = extract_min(pq, &pq_size, "priority");
            int current_process = current_node.process_index;

            schedule[(*schedule_size)][0] = current_time;
            schedule[(*schedule_size)++][1] = processes[current_process].process_num;

            current_time += processes[current_process].burst_time;

            completed ++;
            processes[current_process].completion_time = current_time;
        }
        else current_time++;
    }
}
/*******************************************************************************/

//Priority with Preemption
/*******************************************************************************/
void priority_with_preemption(Process processes[], int num_processes, int schedule[][2], int *schedule_size){
    int current_time = 0;
    int completed = 0;
    PQNode pq[MAX_PROCESSES];
    int pq_size = 0;
    int current_process = -1;

    while(completed < num_processes){
        for(int i = 0; i < num_processes; i++){
            if(processes[i].arrival_time == current_time && processes[i].remaining_time > 0){
                if(current_process != -1 && processes[i].priority < processes[current_process].priority){
                    schedule[(*schedule_size)][0] = current_time; 
                    schedule[(*schedule_size)++][1] = processes[i].process_num;

                    PQNode new_node = {current_process, processes[current_process].remaining_time, processes[current_process].arrival_time, processes[current_process].priority, processes[current_process].process_num};
                    insert(pq, &pq_size, new_node);

                    current_process = i;
                } else {
                    PQNode new_node = {i, processes[i].remaining_time, processes[i].arrival_time, processes[i].priority, processes[i].process_num};
                    insert(pq, &pq_size, new_node);
                }
            }
        }

        if(current_process == -1 && pq_size > 0){
            PQNode current_node = extract_min(pq, &pq_size, "priority");
            current_process = current_node.process_index;

            schedule[(*schedule_size)][0] = current_time;
            schedule[(*schedule_size)++][1] = processes[current_process].process_num;
        }

        if(current_process != -1){
            processes[current_process].remaining_time--;
        }

        if(current_process != -1 && processes[current_process].remaining_time == 0){
            completed++;
            processes[current_process].completion_time = current_time + 1;
            current_process = -1;
        }

        current_time++;
    }
}
/*******************************************************************************/


void calculate_waiting_time(Process processes[], int num_processes, int waiting_time[]){
    
    for(int i = 0; i < num_processes; i++){
        waiting_time[i] = processes[i].completion_time - processes[i].arrival_time - processes[i].burst_time;
    }
}

float calculate_avg_waiting_time(int waiting_time[], int num_processes){
    int total_waiting_time = 0;

    for(int i = 0; i < num_processes; i++){
        total_waiting_time += waiting_time[i];
    }

    return (float)total_waiting_time / num_processes;
}

int main(){
    Process processes[MAX_PROCESSES];
    char algorithm[20];
    int time_quantum;
    int num_processes;
    int schedule[MAX_PROCESSES * 10][2];
    int schedule_size = 0;
    int waiting_time[MAX_PROCESSES];
    float avg_waiting_time;

    // issues with input number: 6, 16

    read_processes("input.txt", processes, algorithm, &time_quantum, &num_processes);

    if(strcmp(algorithm, "RR") == 0){
        round_robin(processes, num_processes, time_quantum, schedule, &schedule_size);
    } else if(strcmp(algorithm, "SJF") == 0){
        shortest_job_first(processes, num_processes, schedule, &schedule_size);
    } else if(strcmp(algorithm, "PR_noPREMP") == 0){
        priority_no_preemption(processes, num_processes, schedule, &schedule_size);
    } else if(strcmp(algorithm, "PR_withPREMP") == 0){
        priority_with_preemption(processes, num_processes, schedule, &schedule_size);
    }

    calculate_waiting_time(processes, num_processes, waiting_time);
    avg_waiting_time = calculate_avg_waiting_time(waiting_time, num_processes);
    write_output(algorithm, time_quantum, schedule, schedule_size, avg_waiting_time);

}