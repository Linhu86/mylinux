/*************************************************************************
    > File Name: dfs.cpp
    > Author: linhu
    > Mail: ylh@hotmail.com 
    > Created Time: Mon 21 Dec 2015 11:32:17 PM CET
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <Stack>
#include <Queue>
using namespace std;
#define Element char
#define format "%c"

typedef struct Node
{
  Element data;
  struct Node *lchild;
  struct Node *rchild;
} *Tree;

int index = 0;

void treeNodeConstructor(Tree &root, Element data[])
{
  Element e = data[index++];

  if(e == "#")
  {
    root = NULL;
  }
  else
  {
    root = (Node *)malloc(sizeof(Node));
	root->data = e;
	treeNodeConstructor(root->lchild, data);
	treeNodeConstructor(root->rchild, data);
  }
}

void depthFirstSearch(Tree root)
{
  stack<Node *> nodeStack;
  nodeStack.push(root);
  Node *node;
  while(!nodeStack.empty())
  {
    node = nodeStack.top();
	printf(format, node->data);
    nodeStack.pop();
	if(node->rchild)
	{
      nodeStacck.push(node->rchild);
	}
	if(node->lchild)
	{
	  nodeStack.push(node->lchild);
	}
  }
}

void breadthFirstSearch(Tree root){
  queue<Node *> nodeQueue;  //使用C++的STL标准模板库
  nodeQueue.push(root);
  Node *node;
  while(!nodeQueue.empty()){
    node = nodeQueue.front();
    nodeQueue.pop();
	printf(format, node->data);
	if(node->lchild){
	  nodeQueue.push(node->lchild);  //先将左子树入队
    }
	if(node->rchild){
	  nodeQueue.push(node->rchild);  //再将右子树入队
	}
  }
}
  
int main()
{
  Element data[15] = {'A', 'B', 'D', '#', '#', 'E', '#', '#', 'C', 'F','#', '#', 'G', '#', '#'};
  Tree tree;
  treeNodeConstructor(tree, data);
  printf("深度优先遍历二叉树结果: ");
  depthFirstSearch(tree);
  printf("\n\n广度优先遍历二叉树结果: ");
  breadthFirstSearch(tree);
  return 0;
}



