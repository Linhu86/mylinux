#include <iostream>
using namespace std

template <class T>
class BSTNode{
    public:
        T key;            
        BSTNode *left;   
        BSTNode *right;   
        BSTNode *parent;

        BSTNode(T value, BSTNode *p, BSTNode *l, BSTNode *r):
            key(value),parent(),left(l),right(r) {}
};

template <class T>
class BSTree {
    private:
        BSTNode<T> *mRoot;

    public:
        BSTree();
        ~BSTree();

        void preOrder();
        
		void inOrder();
        
		void postOrder();

        BSTNode<T>* search(T key);
        
		BSTNode<T>* iterativeSearch(T key);

        T minimum();
        
		T maximum();

        // 找结点(x)的后继结点。即，查找"二叉树中数据值大于该结点"的"最小结点"。
        BSTNode<T>* successor(BSTNode<T> *x);
        // 找结点(x)的前驱结点。即，查找"二叉树中数据值小于该结点"的"最大结点"。
        BSTNode<T>* predecessor(BSTNode<T> *x);

        // 将结点(key为节点键值)插入到二叉树中
        void insert(T key);

        // 删除结点(key为节点键值)
        void remove(T key);

        // 销毁二叉树
        void destroy();

        // 打印二叉树
        void print();
    private:
        // 前序遍历"二叉树"
        void preOrder(BSTNode<T>* tree) const;
        // 中序遍历"二叉树"
        void inOrder(BSTNode<T>* tree) const;
        // 后序遍历"二叉树"
        void postOrder(BSTNode<T>* tree) const;

        // (递归实现)查找"二叉树x"中键值为key的节点
        BSTNode<T>* search(BSTNode<T>* x, T key) const;
        // (非递归实现)查找"二叉树x"中键值为key的节点
        BSTNode<T>* iterativeSearch(BSTNode<T>* x, T key) const;

        // 查找最小结点：返回tree为根结点的二叉树的最小结点。
        BSTNode<T>* minimum(BSTNode<T>* tree);
        // 查找最大结点：返回tree为根结点的二叉树的最大结点。
        BSTNode<T>* maximum(BSTNode<T>* tree);

        // 将结点(z)插入到二叉树(tree)中
        void insert(BSTNode<T>* &tree, BSTNode<T>* z);

        // 删除二叉树(tree)中的结点(z)，并返回被删除的结点
        BSTNode<T>* remove(BSTNode<T>* &tree, BSTNode<T> *z);

        // 销毁二叉树
        void destroy(BSTNode<T>* &tree);

        // 打印二叉树
        void print(BSTNode<T>* tree, T key, int direction);
};


template<class T> 
BSTree :: BSTree(): mRoot(NULL)
{
}


template<class T> 
BSTree :: ~BSTree()
{
  destroy();
}

template<class T>
void BSTree :: preOrder()
{ 
  preOrder(mRoot);
}

template<class T>
void BSTree :: preOrder(BSTNode<T>* tree) const
{
  if(tree != NULL)
  {
    cout << tree->key << endl;
    preOrder(tree->left);
    preOrder(tree->right);
  }
}

template<class T>
void BSTree :: inOrder()
{
  inOrder(mRoot);
}

template<class T>
void BSTree :: inOrder(BSTNode<T>* tree)
{
  if(tree != NULL)
  {
    inOrder(tree->left);
	cout << tree->key << endl;
    inOrder(tree->right);
  }
}

template<class T>
void BSTree :: postOrder()
{
  postOrder(mRoot);
}

template<class T>
void BSTree :: postOrder(BSTNode<T>* tree)
{
  if(tree != NULL)
  {
    postOrder(tree->left);
	postOrder(tree->right);
	cout << tree->key << endl;
  }
}

template<class T>
BSTNode<T> BSTree:: *search(T key)
{
  return iterativeSearch(mRoot, key);  
}

template<class T>
BSTNode<T> BSTree:: *search(BSTNode<T>*x, T key) const
{
  if(x == NULL && x->key = key)
  {
    return x;
  }

  if(x->key < key)
  {
    search(x->left, key);
  }
  else
  {
    search(x->right, key);
  }
}

template<class T>
BSTNode<T>* BSTree:: iterativeSearch(T key)
{
  iterativeSearch(mRoot); 
}

template<class T>
BSTNode<T>* BSTree:: iterativeSearch(BSTNode<T>* x, T key)
{
  while(x != NULL || x->key != key)
  {
	if(x->key < key)
	{
	  x = x->left;
	}
	else
	{
	  x = x->right;
	}
  }
  return x;
}

template<class T>
T BSTree::minimum()
{
  BSTNode *minNode = minimum(mRoot);

  if(minNode == NULL)
  {
    return NULL;
  }

  return minNode->key;
}

template<class T>
BSTNode<T> * BSTree :: minimum(BSTNode<T>* tree)
{
  if(tree == NULL)
  {
    return NULL;
  }

  while(tree->left != NULL)
  {
    tree = tree->left;
  }

  return tree;
}

template<class T>
T BSTree::maximum()
{
  BSTNode *maxNode = maximum(mRoot);

  if(maxNode == NULL)
  {
    return NULL;
  }

  return maxNode->key;
}

template<class T>
BSTNode<T> * BSTree :: maximum(BSTNode<T>* tree)
{
  if(tree == NULL)
  {
    return NULL;
  }

  while(tree->right != NULL)
  {
    tree = tree->right;
  }

  return tree;
}

template<class T>
void BSTree :: insert(BSTNode <T>* &tree, BSTNode<T> *z)
{
  BSTNode *y = NULL;
  BSTNode *x = tree;

  while(x != NULL)
  {
    y = x;
	if(z->key < x->key)
	{
	  x = x->left;
	}
	else
	{
	  x = x->right;
	}
  }

  z->parent = y;
  if(y == NULL)
  {
    mRoot = z;
  }
  else
  {
    if(z->key < y->key)
	{
	  y->left = z;
	}
	else
	{
	  y->right = z;
	}
  }

}

template<class T>
void BSTree :: insert(T key)
{
  BSTNode<T> *newNode = new BSTNode<T>;
  
  if(newNode == NULL)
  {
    return;
  }

  newNode->key = key;
  
  insert(mRoot, newNode);
}

template <class T>
BSTNode<T>* remove(BSTNode<T>* &tree, BSTNode<T> *z)
{
  BSTNode<T> *x=NULL;
  BSTNode<T> *y=NULL;

  if((z->left == NULL) || (z->right == NULL))
  {
    y = z;
  }
  else{
    y = successor(z);
  }

  if(y->left != NULL)
  {
    x = y->left;
  }
  else
  {
    x = y->right;
  }

  if(x != NULL)
  {
    x->parent = y->parent;
  }

  if(y->parent == NULL)
  {
    tree = x;
  }
  else if (y == y->parent->left)
  {
    y->parent->left = x;
  }
  else
  {
    y->parent->right = x;
  }

  if(y != z)
  {
    z->key = y->key;
  }

  return y;
}

template <class T>
void BSTree::remove(T key)
{
  BSTNode <T>* nodeDelete;
  BSTNode <T>* z;

  z = search(z, key);

  if(z != NULL)
  {
    nodeDelete = remove(mRoot, z);
	if(nodeDelete != NULL)
	{
	  delete nodeDelete;
	}
  }

}

template <class T>
void BSTree<T>::print(BSTNode<T>* tree, T key, int direction)
{
  if(tree != NULL)
  {
    if(direction==0)
	{
      cout << setw(2) << tree->key << " is root" << endl;
	}
    else
	{
      cout << setw(2) << tree->key << " is " << setw(2) << key << "'s "  << setw(12) << (direction==1?"right child" : "left child") << endl;
	}
	print(tree->left, tree->key, -1);
    print(tree->right,tree->key,  1);
  }
}

template <class T>
void BSTree<T>::print()
{
  if(mRoot != NULL)
  {
    print(mRoot, mRoot->key, 0);
  }
}

template<class T>
void BSTree :: destroy(BSTNode<T>* &tree)
{
  if(tree == NULL)
  {
    return;
  }

  if(tree->left != NULL)
  {
    destroy(tree->left);
  }
  else
  {
    destroy(tree->right);
  }
  delete tree;
  tree = NULL;
}

template<class T>
void BSTree :: destroy(BSTNode<T>* &tree)
{
  destroy(mRoot); 
}


static int arr[]= {1,5,4,3,2,6};
#define TBL_SIZE(a) ( (sizeof(a)) / (sizeof(a[0])) )

int main()
{
  int i, ilen;
  BSTree<int>* tree=new BSTree<int>();

  cout << "== 依次添加: ";
  ilen = TBL_SIZE(arr);
  for(i=0; i<ilen; i++) 
  {
    cout << arr[i] <<" ";
	tree->insert(arr[i]);
  }

  cout << "\n== 前序遍历: ";
  tree->preOrder();

  cout << "\n== 中序遍历: ";
  tree->inOrder();

  cout << "\n== 后序遍历: ";
  tree->postOrder();
  cout << endl;

  cout << "== 最小值: " << tree->minimum() << endl;
  cout << "== 最大值: " << tree->maximum() << endl;
  cout << "== 树的详细信息: " << endl;
  tree->print();

  cout << "\n== 删除根节点: " << arr[3];
  tree->remove(arr[3]);

  cout << "\n== 中序遍历: ";
  tree->inOrder();
  cout << endl;

  tree->destroy();
																							
  return 0;
}


