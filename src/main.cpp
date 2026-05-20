#include "minispire/GameApp.h"

#include <exception>
#include <iostream>

int main()
{
    try {
        minispire::GameApp app;
        return app.run();
    } catch (const std::exception& ex) {
        std::cerr << "Mini Spire crashed: " << ex.what() << '\n';
        return 1;
    }
}
