/*************************************************************************
    > File Name: bintree.cpp
    > Author: linhu
    > Mail: ylh@hotmail.com 
    > Created Time: Wed 23 Dec 2015 11:33:40 PM CET
 ************************************************************************/

#include<iostream>
using namespace std;

template <T>
struct node
{
  struct node *parent;
  struct node *lchild;
  struct node *rchildy;
  T key;
};

template <class T>
class Bintree
{
  public:
	  Bintree();
      ~Bintree();
	  int node_insert(T key);
	  int node_delete(T key); 
      int node_search(T key);
	  T max();
	  T min();
      void iter_preorder();
	  void iter_inorder();
	  void iter_postorder();
	  void tree_destroy();

  private:
	  struct node<T> *m_root;
};

template <class T>
class Bintree<T> :: Bintree()
{
  m_root = NULL;
}

template <class T>
class Bintree<T> :: ~Bintree()
{
  tree_destroy();
}y

template <class T>
int Bintree<T> :: node_insert(T key)
{
  struct node<T> iter;

  if(m_root == NULL)
  {
    struct node<T> new_node = new struct node<T>;
	node->lchild = NULL;
	node->rchild = NULL;
	node->key = key;
  }

  while(iter != NULL)
  {
    if(key < node->key)
	{
	  iter = node->left;  
	}
	else
	{
	  iter = node->right:
	}
  }

}











