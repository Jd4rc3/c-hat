#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <pthread.h>

typedef struct {
  int sockfd;
  char *name;
} client_t;

typedef struct Node {
  client_t *client;
  struct Node *next;
  pthread_t thread;
} Node;

Node *new_node(client_t *client);

void insert_node(Node **head, Node *new_node);

void delete_node(Node **head, Node *node);

Node *search_node(Node **head, client_t *client);

void print_list(Node **head);

#endif

// TODO **Verificación de `malloc`**: En función `new_node` es una buena
// práctica verificar si `malloc` ha devuelto `NULL`, lo que indicaría que la
// asignación de memoria ha fallado. Por ejemplo:
//
//     ```c
//     Node *new_node = (Node *)malloc(sizeof(Node));
//     if (new_node == NULL) {
//         return NULL;
//     }
//     ```
//
// TODO **Eliminar nodo en lista enlazada**: Al eliminar un nodo de la lista
// se recorre la lista hasta encontrar el nodo anterior al nodo a eliminar.
// Esto lleva tiempo O(n). Si no necesitas mantener un orden específico en tu
// lista, podrías considerar eliminar el nodo al principio de la lista, lo que
// llevaría tiempo constante O(1).
//
// TODO **Inserción en lista enlazada**: En tu función `insert_node`, estás
// insertando nuevos nodos al final de la lista. Esto requiere recorrer toda
// la lista, lo que lleva tiempo O(n). Si no necesitas mantener un orden
// específico en tu lista, podrías considerar insertar al principio de la
// lista, lo que llevaría tiempo constante O(1).
//
// TODO **Manejo de errores**: Considerar agregar más manejo de errores en tu
// código. Por ejemplo, ¿qué debería hacer tu programa si se intenta eliminar
// un nodo que no está en la lista?
//
// TODO **Limpieza de memoria**: Asegúrate de liberar toda la memoria asignada
// con `malloc` cuando ya no es necesaria. Esto incluye los nodos de la lista
// enlazada y las estructuras `client_t`.
