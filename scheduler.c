#include <stdio.h>
#include <stdlib.h>

int INDEX;

typedef struct {
	int num;
	int arrival;
	int burst;
	int remaining;
	int ref_index;
	int ready;
	int wait;
	int faulty;
	int m;
	int* ref;
} process;

typedef struct {
	int FRONT;
	int REAR;
	int SIZE;
	int num;
	process *arr;
} queue;

void enqueue(queue *q, process X) {
	if(q->FRONT == (q->REAR + 1) % q->SIZE) {
		printf("Queue is full!\n");
	}
	else {
		if(q->FRONT == -1) {
			q->FRONT = 0;
			q->REAR = 0;
			q->arr[q->REAR] = X;
		}
		else {
			q->REAR = (q->REAR+1) % q->SIZE;
			q->arr[q->REAR] = X;
		}
		
		q->num++;
	}
}

void enqueue_front(queue *q, process X)
{
	if(q->FRONT == (q->REAR + 1) % q->SIZE)
		printf("Queue is full!\n");
		
	else {
		if(q->FRONT == -1) {
			q->FRONT = 0;
			q->REAR = 0;
			q->arr[q->FRONT] = X;
		}
		else {
			q->FRONT = (q->FRONT + q->SIZE-1) % q->SIZE;
			q->arr[q->FRONT] = X;
		}
		
		q->num++;
	}
}

process dequeue(queue *q) {
	if(q->FRONT != -1) {
		process X = q->arr[q->FRONT];
		
		if(q->FRONT == q->REAR)
		{
			q->FRONT = -1;
			q->REAR = -1;
		}
		else
			q->FRONT = (q->FRONT+1) % q->SIZE;
		
		q->num--;
		
		return X;
	}
}

void swap(int x, int* frames, int frame_num) {
	printf("Swapped %d in place of %d\n", x, frames[INDEX]);
	frames[INDEX] = x;
	INDEX = (INDEX + 1) % frame_num;
}

int check_fault(process* p, int* frames, int frame_num){
	int temp = p->ref_index;
	
	for(int i = p->ref_index; i < p->m; i++) {
		int found = 0;
		
		for(int j = 0; j < frame_num; j++) {
			if(frames[j] == p->ref[i]) {
				printf("Process %d hit page %d\n", p->num, p->ref[i]);
				found = 1;
				p->ref[i] = -1;
				p->ref_index++;
				break;
			}
		}
		
		if (found == 0) {
			p->faulty = p->ref[i];
			
			if(i == temp)
				return 1;
			
			return 2;
		}
	}
		
	return 0;
}

void start(process* p, int n, int* frames, int num, int wait_time) {
	queue ready;
	ready.FRONT = -1;
	ready.REAR = -1;
	ready.SIZE = n;
	ready.num = 0;
	ready.arr = malloc(n * sizeof(process));
	
	queue wait;
	wait.FRONT = -1;
	wait.REAR = -1;
	wait.SIZE = n;
	wait.num = 0;
	wait.arr = malloc(n * sizeof(process));
	
	float avg_tt = 0;
	float avg_wt = 0;
	int fault = 0;
	int temp = -1;
	
	for(int time = 0, count = 0; count != n; time++) {
		if(temp != time)
			printf("%d ms:\n", time);
		temp = time;
		
		for(int k=0; k<wait.num; k++) {
			process X = dequeue(&wait);
			
			X.wait--;
			
			if(X.wait == 0) {
				enqueue(&ready, X);
				swap(X.faulty, frames, num);
				printf("Process %d entered ready queue\n", X.num);
			}
			else
				enqueue(&wait, X);
		}
		
		for(int i = 0; i < n; i++) {
			if(p[i].arrival <= time && p[i].remaining > 0 && p[i].ready == 0) {
				enqueue(&ready, p[i]);
				printf("Process %d entered ready queue\n", p[i].num);
				p[i].ready = 1;
			}
		}
		
		if(ready.FRONT != -1) {
			process P = dequeue(&ready);
			
			int check = check_fault(&P, frames, num);
		
			switch(check) {
				case 0:
					break;
				case 1:
					fault++;
					P.wait = wait_time + 1;
					time--;
					enqueue(&wait, P);
					break;
				case 2:
					fault++;
					P.wait = wait_time + 1;
					enqueue(&wait, P);
					break;
			}
			
			if(check != 1) {
				P.remaining--;
				printf("Process %d executed for 1 ms\n", P.num);
				
				if(check == 2) {
					printf("Process %d missed page %d\n", P.num, P.faulty);
					printf("Process %d entered waiting queue\n", P.num);
				}

				if(P.remaining == 0) {
					count++;
					avg_tt += time + 1 - P.arrival;
					avg_wt += time + 1 - P.arrival - P.burst;
				} else if (check == 0) {	
					enqueue_front(&ready, P);
				}
			} else {
				printf("Process %d missed page %d\n", P.num, P.faulty);
				printf("Process %d entered waiting queue\n", P.num);
			}
		} else
			printf("Skipping 1 ms\n");
	}
	
	printf("\nAverage TURNAROUND TIME = %.2f\n", (avg_tt / n));
	printf("Average WAITING TIME = %.2f\n", (avg_wt / n));
	printf("No of PAGE FAULTS = %d\n", fault);
}

void main() {
	int n, num, wait_time;
	INDEX = 0;
	
	printf("Enter the no of frames: ");
	scanf("%d", &num);
	
	int frames[num];
	
	printf("Enter the current pages in main memory: ");
		for(int i=0; i<num; i++)
			scanf("%d", &frames[i]);
	
	printf("\nEnter no of processes: ");
	scanf("%d", &n);
	
	process process[n];
	
	for(int i=0; i<n; i++) {
		process[i].num = i+1;
		process[i].faulty = -1;
		
		printf("\nEnter arrival time of process %d: ", i+1);
		scanf("%d", &process[i].arrival);
		
		printf("Enter burst time of process %d: ", i+1);
		scanf("%d", &process[i].burst);
		
		process[i].remaining = process[i].burst;
		
		printf("Enter length of page reference string of process %d: ", i+1);
		scanf("%d", &process[i].m);
		
		process[i].ref_index = 0;
		process[i].ready = 0;
		
		printf("Enter the page reference string of process %d: ", i+1);
		
		process[i].ref = (int*) malloc(process[i].m * sizeof(int));
		
		for(int j=0 ; j<process[i].m; j++)
			scanf("%d", &process[i].ref[j]);
	}
	
	printf("\nEnter page replacement time: ");
	scanf("%d", &wait_time);
	printf("\n");
	start(process, n, frames, num, wait_time);
}
