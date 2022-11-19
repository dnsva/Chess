
#ifdef _WIN32

#include <windows.h>
#include <cstdint>
#include <string>

bool init_term() {

    auto oh = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (oh == INVALID_HANDLE_VALUE){
        return false;
    }

    DWORD omode;
    if (!::GetConsoleMode(oh, &omode)){
        return false;
    }

    omode |= 0x0004;
    if (!::SetConsoleMode(oh, omode)){
        return false;
    }

    ::SetConsoleOutputCP(65001);
    ::SetConsoleCP(65001);

    return true;
}

#endif