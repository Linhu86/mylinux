/*************************************************************************
    > File Name: rbtree.cpp
    > Author: linhu
    > Mail: ylh@hotmail.com 
    > Created Time: Sat 19 Dec 2015 10:31:19 PM CET
 ************************************************************************/

#include<iostream>
using namespace std;

enum RBTColor{RED, BLACK};

template <class T>
class RBTNode
{
  public:
    RBTNode(T value, RBTColor c, RBTNode *p, RBTNode *l, RBTNode *r):
		  key(value), color(c), parent(p), left(l), right(r)
	{}

  private:
	RBTColor color;
    T key;
	RBTNode *left;
	RBTNode *right;
	RBTNode *parent;
};

template <class T>
class RBTree
{
  private:
    RBTNode<T> *mRoot;

  public:
	RBTree();
    ~RBTree();
	void preOrder();
	void inOrder();
	void postOrder();

	RBTNode<T> *search(T key);
	RBTNode<T> *iterativeSearch(T key);
  
	T minimum();
	T maximum();

	RBTNode<T> *successor(RBTNode<T> *x);
	RBTNode<T> *predecessor(RBTNode<T> *x);

	void insert(T key);

	void remove(T key);

	void destroy();

	void print();

  private:
    
	void preOrder(RBTNode<T>* tree)const;
	void inOrder(RBTNode<T>* tree)const;
	void postOrder(RBTNode<T>* tree)const;

	RBTNode<T> *search(RBTNode<T> *x, T key)const;
	RBTNode<T> *iterativeSearch(RBTNode<T>*x, T key)const;

    RBTNode<T> *minimum(RBTNode<T>* tree);
	RBTNode<T> *maximum(RBTNode<T>* tree);

    void leftRotate(RBTNode<T> *&root, RBTNode<T> *x);
	void rightRotate(RBTNode<T> *&root, RBTNode<T> *y);
    void insert(RBTNode<T> *&root, RBTNode<T> *node);
    void insertFixUp(RBTNode<T> *&root, RBTNode<T>*node);
    void remove(RBTNode<T>* &root, RBTNode<T> *node);
    void removeFixUp(RBTNode<T>* &root, RBTNode<T> *node, RBTNode<T> *parent);
	void destroy(RBTNode<T>* &tree);
	void print(RBTNode<T>* tree, T key, int direction);

#define rb_parent(r) ((r)->parent)
#define rb_color(r) ((r)->color)
#define rb_is_red(r) ((r)->color == RED)
#define rb_is_black(r) ((r)->color == BLACK)
#define rb_parent(r) ((r)->parent)
#define rb_parent(r) ((r)->parent)
#define rb_set_black(r)  do { (r)->color = BLACK; } while (0)
#define rb_set_red(r)  do { (r)->color = RED; } while (0)
#define rb_set_parent(r,p)  do { (r)->parent = (p); } while (0)
#define rb_set_color(r,c)  do { (r)->color = (c); } while (0)

};

void RBTree::leftRotate(RBTNode<T>*&root,RBTNode<T>*x)
{
  RBTNode<T> *y = x->right;

  x->right = y->left;

  if(NULL != y->left)
  {
    y->left->parent = x;
  }

  y->parent = x->parent;

  /* check if x is root node. */
  if(NULL == x->parent)
  {
    y = mRoot;
  }
  else
  {
    if(x->parent->left == x)
	{
	  y = x->parent->left;
	}
	else
	{
	  y = x->parent->right;
	}
  }

  y->left = x;
  x->parent = y;
}

void RBTree::rightRotate(RBTNode<T>*&root, RBTNode<T>*x)
{
  RBTNode<T> *y = x->left;
  
  x->left = y->right;

  if(NULL == y->right)
  {
    y->right->parent = x;
  }

  if(x->parent == NULL)
  {
    y = mRoot;
  }
  else
  {
    if(x->parent->left == x)
	{
	  x->parent->left = y;
	}
	else
	{
	  x->parent->right = y;
	}
  }

  y->right = x;
  x->parent = y;
}















