//#ifndef BQueue_
//#define BQueue_
#define MAX_QUEUE_SIZE 500
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    //int c;
    int row,col;
    //int L_count;
    //int left_x, right
} element;

// queue 만든다. 
//typedef struct queue *queue_pointer;
typedef struct queue {
    element item;
    struct queue * link; // 자기 참조... 다음 원소 가리켜야 하니까..
} *queue_pointer;

void addq(queue_pointer *front, queue_pointer *rear, element item)
{
   queue_pointer temp = (queue_pointer) malloc(sizeof(queue));
   temp->item = item; // 추가되는 현재 원소값 
   temp->link = NULL; // 현재가 마지막 이므로...
   if (*front) // 처음 시작이 널이 아니라면.. 
       (*rear)->link = temp;
   else *front = temp; // 이제 처음 만들어진거지..
   *rear = temp;
}

element deleteq(queue_pointer *front, queue_pointer *rear)
{
    queue_pointer temp=*front;
    element item;
    item.row = -1;
    if (*front==NULL)
    {
        //free(rear2);
        *rear = NULL;
        //printf ("\n queue is empty");
        return item;
    }
    item = temp->item; // 처음값 빼내야 하니까..
    *front = temp->link; // 다음값 연결해주고..
    free(temp); // 지우고.
    return item; // 지워지는 값 반환한다.
}
//#endif
