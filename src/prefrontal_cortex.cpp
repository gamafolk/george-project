#include "modules/thalamus/thalamus.cpp"
#include "classes/interpreter/interpreter.cpp"
#include <iostream>

class PrefrontalCortex
{
public:
    static void boot();
};

void PrefrontalCortex::boot()
{
    Interpreter::boot();

    auto response = Interpreter::prompt("What do you know about the Sun?");

    std::cout << "Robo (LLM Responde): " << response << std::endl;
}
