#ifndef _LIB_H_
#define _LIB_H_

// Linked list structure
struct linkedList
{
  int val;
  char *nickName;
  struct linkedList *next;
};

// Перевод сокетов в неблокирующий режим
int setNonblock(int fileDescriptor);

// delete node from list
void delNodes(int socketNode, struct linkedList **List);

// Add nodes to list
void addNodes(int socketNode, struct linkedList **List);

// Nickname request and add to node
int nickNameRequest(int clientSocket, struct linkedList **List);

#endif //_LIB_H_
