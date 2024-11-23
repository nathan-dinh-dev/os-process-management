#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_PROCESSES 10
#define TIME_SLICE 3

typedef enum { READY, RUNNING, BLOCKED } ProcessState;

typedef struct {
    int processId;
    int parentProcessId;
    int programCounter;
    int priority;
    ProcessState state;
    int startTime;
    int timeUsed;
} PCB;

typedef struct {
    PCB *queue[MAX_PROCESSES];
    int front;
    int rear;
    int size;
} Queue;

// Global Variables
Queue readyQueue;
Queue blockedQueue;
PCB *runningProcess = NULL;
int processCounter = 0;
int systemTime = 0;

// Function Prototypes
void initializeQueue(Queue *q);
bool enqueue(Queue *q, PCB *process);
PCB *dequeue(Queue *q);
void createProcess(int parentProcessId);
void simulateTimeSlice();
void unblockProcess();
void printSystemState();

// Initialize a queue
void initializeQueue(Queue *q) {
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

// Add a process to the queue
bool enqueue(Queue *q, PCB *process) {
    if (q->size == MAX_PROCESSES) {
        printf("Queue is full. Cannot add process.\n");
        return false;
    }
    q->rear = (q->rear + 1) % MAX_PROCESSES;
    q->queue[q->rear] = process;
    q->size++;
    return true;
}

// Remove a process from the queue
PCB *dequeue(Queue *q) {
    if (q->size == 0) {
        printf("Queue is empty. Cannot dequeue.\n");
        return NULL;
    }
    PCB *process = q->queue[q->front];
    q->front = (q->front + 1) % MAX_PROCESSES;
    q->size--;
    return process;
}

// Create a new process
void createProcess(int parentProcessId) {
    PCB *newProcess = (PCB *)malloc(sizeof(PCB));
    newProcess->processId = ++processCounter;
    newProcess->parentProcessId = parentProcessId;
    newProcess->programCounter = 0;
    newProcess->priority = 0;
    newProcess->state = READY;
    newProcess->startTime = systemTime;
    newProcess->timeUsed = 0;
    enqueue(&readyQueue, newProcess);
    printf("Process %d created by parent %d.\n", newProcess->processId, parentProcessId);
}

// Simulate a time slice for the running process
void simulateTimeSlice() {
    if (runningProcess == NULL) {
        runningProcess = dequeue(&readyQueue);
        if (runningProcess == NULL) {
            printf("No process in the ready queue.\n");
            return;
        }
        runningProcess->state = RUNNING;
        printf("Process %d is now running.\n", runningProcess->processId);
    }

    runningProcess->timeUsed++;
    systemTime++;

    if (runningProcess->timeUsed % TIME_SLICE == 0) {
        printf("Time slice expired for process %d.\n", runningProcess->processId);
        runningProcess->state = READY;
        enqueue(&readyQueue, runningProcess);
        runningProcess = NULL; // Scheduler will pick the next process
    }
}

// Unblock a process from the blocked queue
void unblockProcess() {
    PCB *process = dequeue(&blockedQueue);
    if (process != NULL) {
        process->state = READY;
        enqueue(&readyQueue, process);
        printf("Process %d unblocked and moved to the ready queue.\n", process->processId);
    }
}

// Print the system state
void printSystemState() {
    printf("\nSystem State at Time %d:\n", systemTime);
    printf("Running Process: ");
    if (runningProcess) {
        printf("Process %d\n", runningProcess->processId);
    } else {
        printf("None\n");
    }

    printf("Ready Queue: ");
    for (int i = 0; i < readyQueue.size; i++) {
        int index = (readyQueue.front + i) % MAX_PROCESSES;
        printf("P%d ", readyQueue.queue[index]->processId);
    }
    printf("\n");

    printf("Blocked Queue: ");
    for (int i = 0; i < blockedQueue.size; i++) {
        int index = (blockedQueue.front + i) % MAX_PROCESSES;
        printf("P%d ", blockedQueue.queue[index]->processId);
    }
    printf("\n");
}

int main() {
    char command;
    initializeQueue(&readyQueue);
    initializeQueue(&blockedQueue);

    printf("Operating System Process Management Simulation - Phase 2\n");
    printf("Commands:\n");
    printf("C: Create a new process\n");
    printf("Q: Simulate time slice\n");
    printf("U: Unblock a process\n");
    printf("P: Print system state\n");
    printf("T: Terminate simulation\n");

    while (1) {
        printf("$ ");
        scanf(" %c", &command);

        switch (command) {
            case 'C':
                createProcess(runningProcess ? runningProcess->processId : 0);
                break;
            case 'Q':
                simulateTimeSlice();
                break;
            case 'U':
                unblockProcess();
                break;
            case 'P':
                printSystemState();
                break;
            case 'T':
                printf("Terminating simulation.\n");
                exit(0);
                break;
            default:
                printf("Invalid command. Try again.\n");
        }
    }

    return 0;
}
