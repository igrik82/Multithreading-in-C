#ifndef _LIB_H_
#define _LIB_H_

struct linkedList
{
  int val;
  struct linkedList *next;
};

// Перевод сокетов в неблокирующий режим
int setNonblock(int fileDescriptor);

// delete node from list
void delNodes(int socketNode, struct linkedList **List);

// Add nodes to list
void addNodes(int socketNode, struct linkedList **List);

// Getting maxValue from slave nodes
int getMaxInClientSocket(struct linkedList *List);

// Getting max of two numbers
int maxValue(int serverSocket, int clientsSocket);

#endif //_LIB_H_
