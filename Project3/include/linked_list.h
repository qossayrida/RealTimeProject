#ifndef INCLUDE_LINKED_LIST_H
#define INCLUDE_LINKED_LIST_H



// Define the Node structure
typedef struct Node {
    int citizen_id;
    int time_spent;
    struct Node* next;
} Node;

typedef Node* ListPtr;

// Function to create a new node
Node* createNode(int citizen_id, int time_spent) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    newNode->citizen_id = citizen_id;
    newNode->time_spent = time_spent;
    newNode->next = NULL;
    return newNode;
}

// Function to append a node to the linked list
void appendNode(Node** head, int citizen_id, int time_spent) {
    Node* newNode = createNode(citizen_id, time_spent);
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
        printf("Citizen ID: %d, Time Spent: %d\n", temp->citizen_id, temp->time_spent);
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
