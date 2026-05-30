#include "document.h"
#include<ostream>
#include<cstring>
#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::ifstream file("../king3.txt");
    
    if (!file.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return 1;
    }
    
    std::string line;
    int n=0;
    while (std::getline(file, line) && n<=3) {
        std::cout << line << std::endl;
        n++;
    }
    
    file.close();
    return 0;
}