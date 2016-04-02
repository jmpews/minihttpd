#include <stdio.h>
typedef  void ElemType;
typedef  int (*FindElemFunc)(ElemType *)
typedef struct node
{
    ElemType *data;
    struct node * next;
}ListNode;

int ListAppend(ListNode *,ElemType *);
ElemType *GetElem(ListNode *,ElemFunc);
ListNode *NewElemNode();

int ListAppend(ListNode *head,ElemType *elem)
{
    ListNode *data;
    data=NewElemNode();
    data->data=elem;
    data->next=head->next;
    head->next=data;
}

ElemType * GetElem(ListNode head,FindElemFunc func)
{
    ListNode *tmp=head;
    if(head==NULL)
        return NULL;
    do{
        if(func(tmp->data))
        {
            return tmp;
        }
    }while(tmp=tmp->next);
}

ListNode *NewElemNode(){
    ListNode *tmp;
    tmp=(ListNode *)malloc(sizeof(ListNode));
    tmp->data=NULL;
    tmp->next=NULL;
}

int func(ElemType *tmp){
    printf("%d\n", (SocketNode *)tmp->b);
    return 0;
}
struct sn
{
    char a;
    int b;
}SocketNode;

int int main(int argc, char const *argv[])
{

    ListNode *head;
    SocketNode s1,s2;
    s1.a='1';
    s1.b=1;
    s2.a='2';
    s2.b=2;

    head=NewElemNode();
    ListAppend(head,(ElemType *)&s1);
    ListAppend(head,(ElemType *)&s1);
    GetElem(head,func);
    return 0;
}
