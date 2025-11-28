
#include <iostream>
#include <random>

int main()
{
    std::random_device rd;   
    std::mt19937 gen(rd());
    std::cout << "Hello World!\n";
    int x;
    std::cout << "Input number: ";
    std::cin >> x;
    std::cout << x << " Your num" << std::endl;
    std::uniform_int_distribution<> dist1(1, 10);
    int y = dist1(gen);
    std::cout << y << " My num" << std::endl;
    if (x == y) {
        std::cout << "You win";
    }
    else {
        std::cout << x - y << " Your error";
    }
    return 0;
}
