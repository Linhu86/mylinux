//FileName: DFS   
//图的深度优先遍历 ---递归与非递归实现  
#include"Graphlnk.h"  
#include"Graphmtx.h"  
#include<iostream>  
#include<stack>  
using namespace std;  
  
template<class T ,class E>  
void DFS(Graphmtx<T,E> &G, const T &v,int judge)//非递归算法，参数int i用以区分递归与非递归  
{  
    cout<<"-------非递归算法被调用-------"<<endl;  
    stack<int> s;  
    int i,loc,out,next;  
    int n = G.NumberOfVertices(); //顶点数  
    bool *visited = new bool[n];  
    for(i = 0;i<n;i++)  
        visited[i] = false;  
    loc = G.getVertexPos(v);  
    s.push(loc);  
    visited[loc]=true; //标记  
    while(!s.empty())  
    {  
        out = s.top();  
        s.pop();  
        cout<<G.getValue(out)<<" ";  
        next = G.getFirstNeighbor(out); //将所有未被标记的邻接点压入堆栈  
        while(next!=-1)  
        {  
            if(visited[next]==false)  
            {  
                s.push(next);  
                visited[next] = true;  
            }  
              
            next = G.getNextNeighbor(out,next);  
        }  
    }  
    delete []visited;  
}  
template<class T ,class E>  
void DFS(Graphmtx<T,E> &G, const T &v)  
//void DFS(Graphlnk<T,E> &G, const T &v)  
{  
    cout<<"-------递归算法被调用-------"<<endl;  
    int i, loc, n =G.NumberOfVertices();  
    bool *visited = new bool[n];  
    for (i = 0;i<n;i++)  
        visited[i] = false;  
    loc = G.getVertexPos(v);  
    DFS(G , loc , visited);  
    delete []visited;  
}  
  
template<class T ,class E>  
void DFS(Graphmtx<T,E> &G, int v, bool  *visited) //递归实现算法  
//void DFS(Graphlnk<T,E> &G, int v, bool  *visited)  
{  
    cout<< G.getValue(v)<< " ";  
    visited[v] = true;  
    int w = G.getFirstNeighbor(v);  
    while(w!=-1)  
    {  
        if(visited[w]==false)  
        {     
            DFS(G, w, visited);  
        }  
        w = G.getNextNeighbor(v,w);       
    }  
}  
  
  
void test_DFS()  
{  
    char first;  
    Graphmtx<char,int> g(30);  
    //Graphlnk<char ,int> g(30);  
    g.inputGraph();  
    g.outputGraph();  
    cout<<"输入深度优先搜索的第一个点："<<endl;  
    cin>>first;  
    //DFS(g,first);//递归调用  
    DFS(g,first,1);//非递归调用  
} 
