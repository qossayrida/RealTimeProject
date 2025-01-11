#ifndef INCLUDE_LINKED_LIST_H
#define INCLUDE_LINKED_LIST_H



// Define the Node structure
typedef struct Node {
    int group_id;
    int citizen_id;
    int time_spent;
    struct Node* next;
} Node;

typedef Node* ListPtr;

// Function to create a new node
Node* createNode(int group_id,int citizen_id, int time_spent) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    newNode->group_id = group_id;
    newNode->citizen_id = citizen_id;
    newNode->time_spent = time_spent;
    newNode->next = NULL;
    return newNode;
}

// Function to append a node to the linked list
void appendNode(Node** head,int group_id, int citizen_id, int time_spent) {
    Node* newNode = createNode(group_id,citizen_id, time_spent);
    if (*head == NULL) {
        *head = newNode;
        return;
    }
    Node* temp = *head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newNode;
}

// Function to print the linked list
void printList(Node* head) {
    Node* temp = head;
    while (temp != NULL) {
        printf("Group ID: %d, Citizen ID: %d, Time Spent: %d\n",temp->group_id,temp->citizen_id, temp->time_spent);
        temp = temp->next;
    }
}

// Function to free the linked list
void freeList(Node* head) {
    Node* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

#endif //INCLUDE_LINKED_LIST_H
