#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int start_time;
    int finish_time;
    int response_time;
    int waiting_time;
    int turnaround_time;
} Process;

void swap(Process *a, Process *b) {
    Process temp = *a;
    *a = *b;
    *b = temp;
}

void sort_by_arrival(Process *processes, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (processes[j].arrival_time > processes[j + 1].arrival_time) {
                swap(&processes[j], &processes[j + 1]);
            }
        }
    }
}

int find_shortest_process(Process *processes, int n, int current_time) {
    int shortest_index = -1;
    int shortest_burst = INT_MAX;

    for (int i = 0; i < n; i++) {
        if (processes[i].arrival_time <= current_time && processes[i].remaining_time > 0) {
            if (processes[i].remaining_time < shortest_burst) {
                shortest_index = i;
                shortest_burst = processes[i].remaining_time;
            }
        }
    }

    return shortest_index;
}

void srtf(Process *processes, int n) {
    sort_by_arrival(processes, n);

    int current_time = 0;
    int completed_processes = 0;

    while (completed_processes < n) {
        int shortest_index = find_shortest_process(processes, n, current_time);

        if (shortest_index == -1) {
            current_time++;
            continue;
        }

        Process *current_process = &processes[shortest_index];

        if (current_process->response_time == -1) {
            current_process->response_time = current_time - current_process->arrival_time;
        }

        current_process->remaining_time--;
        current_time++;

        if (current_process->remaining_time == 0) {
            current_process->finish_time = current_time;
            current_process->waiting_time = current_process->finish_time - current_process->arrival_time - current_process->burst_time;
            current_process->turnaround_time = current_process->finish_time - current_process->arrival_time;

            completed_processes++;
        }
    }
}

void print_processes(Process *processes, int n) {
    for (int i = 0; i < n; i++) {
        printf("%d %d %d %d\n", processes[i].pid, processes[i].response_time, processes[i].waiting_time, processes[i].turnaround_time);
    }
}

void calculate_average_times(Process *processes, int n, float *average_waiting_time, float *average_turnaround_time) {
    float total_waiting_time = 0;
    float total_turnaround_time = 0;

    for (int i = 0; i < n; i++) {
        total_waiting_time += processes[i].waiting_time;
        total_turnaround_time += processes[i].turnaround_time;
    }

    *average_waiting_time = total_waiting_time / n;
    *average_turnaround_time = total_turnaround_time / n;
}

void write_output(Process *processes, int n, float average_waiting_time, float average_turnaround_time) {
    FILE *output_file = fopen("output.txt", "w");

    if (output_file == NULL) {
        printf("Error opening output file.\n");
        return;
    }

    for (int i = 0; i < n; i++) {
        fprintf(output_file, "%d %d %d %d\n", processes[i].pid, processes[i].response_time, processes[i].waiting_time, processes[i].turnaround_time);
    }

    fprintf(output_file, "%.2f\n", average_waiting_time);
    fprintf(output_file, "%.2f\n", average_turnaround_time);

    fclose(output_file);
}

int main() {
    FILE *input_file = fopen("input.txt", "r");

    if (input_file == NULL) {
        printf("Error opening input file.\n");
        return 1;
    }

    int n;
    fscanf(input_file, "%d", &n);

    Process *processes = (Process *)malloc(n * sizeof(Process));

    if (processes == NULL) {
        printf("Error allocating memory for processes.\n");
        fclose(input_file);
        return 1;
    }

    for (int i = 0; i < n; i++) {
        Process process;
        fscanf(input_file, "%d %d %d", &process.pid, &process.arrival_time, &process.burst_time);
        process.remaining_time = process.burst_time;
        process.start_time = -1;
        process.finish_time = -1;
        process.response_time = -1;
        process.waiting_time = -1;
        process.turnaround_time = -1;
        processes[i] = process;
    }

    fclose(input_file);

    srtf(processes, n);

    float average_waiting_time, average_turnaround_time;
    calculate_average_times(processes, n, &average_waiting_time, &average_turnaround_time);

    write_output(processes, n, average_waiting_time, average_turnaround_time);

    free(processes);

    return 0;
}
