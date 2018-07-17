#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"

#define MAX_BUFFER_LEN 80

taskval_t *event_list = NULL;
taskval_t *task = NULL;

int tick, quantum_size, dispatch_size; // all will be initialized in run_simulation fn
int d_count = 0; // keep track of ticks that have been spent dispatching.
int q_count = 0; // keep track of ticks that have been spent in a quantum.

/* status
 * 0 - idle
 * 1 - dispatch
 * 2 - in a quantum
 */
int status = 0;

void print_task(taskval_t *t, void *arg) {
    printf("task %03d: %5d %3.2f %3.2f\n",
        t->id,
        t->arrival_time,
        t->cpu_request,
        t->cpu_used
    );
}


void increment_count(taskval_t *t, void *arg) {
    int *ip;
    ip = (int *)arg;
    (*ip)++;
}


// do the heavy lifting here based on what our status int is set to.
void simulate() {
    switch(status) {
        case 1: // dispatch
            if (d_count == dispatch_size - 1) {
                d_count = 0;
                status = 2;
                printf("id=%d req=%.00f used=%.00f\n", task->id, task->cpu_request, task->cpu_used);
            } else {
                d_count++;
            }
            break;
        case 2: // quantum
            task->cpu_used++;
            printf("id=%d req=%.00f used=%.00f\n", task->id, task->cpu_request, task->cpu_used);
            break;
        case 0: // idle
        default:
            if (task == NULL) {
                printf("IDLE\n");
            } else {
                printf("DISPATCHING\n");
                status = 1;
            }
            break;
    }
}

void run_simulation(int qlen, int dlen) {
    taskval_t *ready_q = NULL;
    dispatch_size = dlen;
    quantum_size = qlen;

    printf("The first item in event_list: %d %d %f\n", event_list->id, event_list->arrival_time, event_list->cpu_request);

    for(tick = 0; tick < 20; tick++) {
        printf("[%05d] ", tick); // start by outputting the tick...

        if (tick == event_list->arrival_time) {
            task = peek_front(event_list);
            event_list = remove_front(event_list);
        }

        simulate(); // output will finish here.
    }
}


int main(int argc, char *argv[]) {
    char   input_line[MAX_BUFFER_LEN];
    int    i;
    int    task_num;
    int    task_arrival;
    float  task_cpu;
    int    quantum_length = -1;
    int    dispatch_length = -1;

    taskval_t *temp_task;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--quantum") == 0 && i+1 < argc) {
            quantum_length = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "--dispatch") == 0 && i+1 < argc) {
            dispatch_length = atoi(argv[i+1]);
        }
    }

    if (quantum_length == -1 || dispatch_length == -1) {
        fprintf(stderr,
            "usage: %s --quantum <num> --dispatch <num>\n",
            argv[0]);
        exit(1);
    }


    while(fgets(input_line, MAX_BUFFER_LEN, stdin)) {
        sscanf(input_line, "%d %d %f", &task_num, &task_arrival,
            &task_cpu);
        temp_task = new_task();
        temp_task->id = task_num;
        temp_task->arrival_time = task_arrival;
        temp_task->cpu_request = task_cpu;
        temp_task->cpu_used = 0.0;
        event_list = add_end(event_list, temp_task);
    }

#ifdef DEBUG
    int num_events;
    apply(event_list, increment_count, &num_events);
    printf("DEBUG: # of events read into list -- %d\n", num_events);
    printf("DEBUG: value of quantum length -- %d\n", quantum_length);
    printf("DEBUG: value of dispatch length -- %d\n", dispatch_length);
#endif

    run_simulation(quantum_length, dispatch_length);

    return (0);
}
