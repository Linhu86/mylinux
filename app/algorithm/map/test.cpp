/*************************************************************************
    > File Name: test.cpp
    > Author: linhu
    > Mail: ylh@hotmail.com 
    > Created Time: Mon 21 Dec 2015 11:20:50 PM CET
 ************************************************************************/

#include<iostream>
#include<vector>
using namespace std;

void Show(int)
{
  cout << int << endl;
}
	

int main()
{
  vector<int> data(10);
  for_each(data.begin(), data.end(), Show);
  cout << endl;
  return 0;
}

