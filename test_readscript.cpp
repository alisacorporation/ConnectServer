#include "include/ReadScript.h"
#include <iostream>

int main() {
    CReadScript script;
    
    if (!script.Load("test.dat")) {
        std::cout << "Failed to load test.dat" << std::endl;
        return 1;
    }
    
    std::cout << "File loaded successfully" << std::endl;
    
    try {
        eTokenResult token = script.GetToken();
        std::cout << "GetToken() returned: " << (int)token << std::endl;
        
        if (token == TOKEN_NUMBER) {
            int num = script.GetNumber();
            std::cout << "GetNumber() returned: " << num << std::endl;
        } else {
            std::cout << "Token is not TOKEN_NUMBER!" << std::endl;
        }
    } catch (...) {
        std::cout << "Exception: " << script.GetError() << std::endl;
    }
    
    return 0;
}
