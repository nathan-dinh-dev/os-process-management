#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_PROCESSES 10
#define TIME_SLICE 3
#define MAX_INSTRUCTIONS 10

typedef enum { READY, RUNNING, BLOCKED } ProcessState;

typedef struct {
    char instruction[10];
    int argument;
} Instruction;

typedef struct {
    int processId;
    int parentProcessId;
    int programCounter;
    int priority;
    ProcessState state;
    int startTime;
    int timeUsed;
    Instruction program[MAX_INSTRUCTIONS];
    int programLength;
    int accumulator; // CPU register for arithmetic operations
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
void executeInstruction(PCB *process);

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
    newProcess->accumulator = 0;
    newProcess->programLength = 5;

    // Sample instructions
    strcpy(newProcess->program[0].instruction, "S");
    newProcess->program[0].argument = 100;
    strcpy(newProcess->program[1].instruction, "A");
    newProcess->program[1].argument = 10;
    strcpy(newProcess->program[2].instruction, "D");
    newProcess->program[2].argument = 5;
    strcpy(newProcess->program[3].instruction, "P");
    newProcess->program[3].argument = 0;
    strcpy(newProcess->program[4].instruction, "E");
    newProcess->program[4].argument = 0;

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

    // Execute one instruction
    executeInstruction(runningProcess);

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

// block a process from and put it in blocked queue
void blockRunningProcess() {
    if (runningProcess == NULL) {
        printf("No running process to block.\n");
        return;
    }

    printf("Process %d blocked.\n", runningProcess->processId);
    runningProcess->state = BLOCKED;
    enqueue(&blockedQueue, runningProcess);
    runningProcess = NULL; // No process is running after blocking
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

// Execute an instruction for the given process
void executeInstruction(PCB *process) {
    if (process->programCounter >= process->programLength) {
        printf("Process %d has completed execution.\n", process->processId);
        runningProcess = NULL;
        return;
    }

    Instruction currentInstruction = process->program[process->programCounter];
    printf("Process %d executing: %s %d\n", process->processId, currentInstruction.instruction, currentInstruction.argument);

    if (strcmp(currentInstruction.instruction, "S") == 0) {
        process->accumulator = currentInstruction.argument;
    } else if (strcmp(currentInstruction.instruction, "A") == 0) {
        process->accumulator += currentInstruction.argument;
    } else if (strcmp(currentInstruction.instruction, "D") == 0) {
        process->accumulator -= currentInstruction.argument;
    } else if (strcmp(currentInstruction.instruction, "P") == 0) {
        printf("Process %d accumulator: %d\n", process->processId, process->accumulator);
    } else if (strcmp(currentInstruction.instruction, "E") == 0) {
        printf("Process %d has ended.\n", process->processId);
        runningProcess = NULL;
        return;
    }

    process->programCounter++;
}

int main() {
    char command;
    initializeQueue(&readyQueue);
    initializeQueue(&blockedQueue);

    printf("Operating System Process Management Simulation - Phase 3\n");
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
            case 'B':
                blockRunningProcess();
                break;
            default:
                printf("Invalid command. Try again.\n");
        }
    }

    return 0;
}
