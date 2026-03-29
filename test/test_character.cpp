#include "../src/Character.h"
#include <iostream>
#include <cassert>

int main() {
    Character c(10.0f, 20.0f, 5.0f, 5.0f, nullptr);

    assert(c.x == 10.0f);
    assert(c.y == 20.0f);
    assert(c.width == 5.0f);
    assert(c.height == 5.0f);
    assert(c.model == nullptr);

    std::cout << "Character tests passed!" << std::endl;
    return 0;
}
