#include <cassert>
#include <iostream>
#include "story/Choices.h"

using namespace hm;

int main() {
    std::cout << "[TEST] Running Ending Resolution Unit Tests..." << std::endl;

    // Test E1: Release
    GameChoices c1;
    c1.sparedAda = true;
    c1.sparedEli = true;
    c1.sparedChoir = true;
    c1.fragmentsFound = 9;
    assert(EndingResolver::resolve(c1) == EndingType::E1_Release);
    std::cout << "  ✓ E1 Release path OK" << std::endl;

    // Test E2: Consumed
    GameChoices c2;
    c2.burnedFiles = true;
    assert(EndingResolver::resolve(c2) == EndingType::E2_Consumed);
    std::cout << "  ✓ E2 Consumed path OK" << std::endl;

    // Test E3: Loop
    GameChoices c3;
    assert(EndingResolver::resolve(c3) == EndingType::E3_Loop);
    std::cout << "  ✓ E3 Loop path OK" << std::endl;

    // Test E4: Martyr
    GameChoices c4;
    c4.sparedAda = true;
    c4.sparedEli = true;
    c4.sparedChoir = true;
    c4.fragmentsFound = 3;
    assert(EndingResolver::resolve(c4) == EndingType::E4_Martyr);
    std::cout << "  ✓ E4 Martyr path OK" << std::endl;

    // Test E5: Revelation
    GameChoices c5;
    c5.confessedEarly = true;
    c5.fragmentsFound = 16;
    assert(EndingResolver::resolve(c5) == EndingType::E5_Revelation);
    std::cout << "  ✓ E5 Revelation path OK" << std::endl;

    std::cout << "[TEST] ALL ENDING RESOLUTION TESTS PASSED!" << std::endl;
    return 0;
}
