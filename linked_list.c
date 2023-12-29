#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>

Node *new_node(client_t *client) {
  Node *new_node = (Node *)malloc(sizeof(Node));
  if (new_node == NULL) {
    fprintf(stderr, "Error al asignar memoria para el nuevo nodo.\n");
    return NULL;
  }

  new_node->client = client;
  new_node->next = NULL;
  return new_node;
}

void insert_node(Node **head, Node *new_node) {
  if (new_node == NULL) {
    return;
  }

  new_node->next = *head;
  *head = new_node;
}

void delete_node(Node **head, Node *node) {
  if (*head == NULL || node == NULL) {
    return;
  }

  Node *current = *head;
  if (current == node) {
    *head = current->next;
    free(current);
    return;
  }

  while (current->next != NULL) {
    if (current->next == node) {
      Node *temp = current->next;
      current->next = current->next->next;
      free(temp);
      return;
    }
    current = current->next;
  }
}
