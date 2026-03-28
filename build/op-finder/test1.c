#include <stdio.h>
#include <stdbool.h>

int main() {
    int a = 10;
    int b = 5;
    int result = 0;
    bool flag1 = true;
    bool flag2 = false;

    result = a + b;     // +
    result = a - b;     // -
    result = a * b;     // *
    result = a / b;     // /
    result = a % b;     // %
    a++;                // ++ (Postfix)
    b--;                // -- (Postfix)
    a = !flag1;         // ! (Logical NOT)

    if (a < b) {}       // <
    if (a > b) {}       // >
    if (a == b) {}      // ==
    if (a != b) {}      // !=
    if (a >= b) {}      // >=
    if (a <= b) {}      // <=
    if (flag1 && flag2) {}  // && (Logical AND)
    if (flag1 || flag2) {}  // || (Logical OR)

    result = a >> 1;    // >> (Right Shift)
    result = a << 1;    // << (Left Shift)
    result = a & b;     // & (Bitwise AND)
    result = a ^ b;     // ^ (Bitwise XOR)
    result = a | b;     // | (Bitwise OR)

    return 0;
}
