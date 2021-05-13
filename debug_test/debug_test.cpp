#include <iostream>
#include <Windows.h>

template<class T>
void template_func(T test, int& b, const int& c)
{
    int a;
    std::cout << test << '\n';
    DebugBreak();
}

struct pod_test
{
    int a;
    float b;
};

int main()
{
    size_t w = 3;
    std::cout << "Hello World!\n";
    pod_test pod;
    int* test = nullptr;
    int g = 2;
    template_func(1, g, 3);
    DebugBreak();
    int a = 3;
    return a;
}
