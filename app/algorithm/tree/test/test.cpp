/*************************************************************************
    > File Name: test.cpp
    > Author: linhu
    > Mail: ylh@hotmail.com 
    > Created Time: Sun 20 Dec 2015 05:28:31 PM CET
 ************************************************************************/


#include <stdio.h>

int main()
{
  char str[] = "http:www.huawei.com";
  volatile int *ptr ;
  *ptr= 10;
  int a = *ptr;
  int c = -5;
  printf("%d, %d.\n", a*a, sizeof(str));
}

