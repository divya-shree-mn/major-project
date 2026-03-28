#include <stdio.h>
#include <stdbool.h>

int calculate_sum_diff(int x, int y) {
    int sum = x + y;
    int diff = x - y;
    if (sum > diff) {
        return sum;
    }
    return diff;
}

int check_bitwise(int val) {
    int temp = val << 2;
    temp = temp & 0xF;
    if (temp != 0) {
        return 1;
    }
    return 0;
}

int check_modulus_and_division(int dividend, int divisor) {
    int q = dividend / divisor;
    int r = dividend % divisor;
    if (q >= 1) {
        return r;
    }
    return q;
}

int simple_multiplication(int val1, int val2) {
    int res = val1 * val2;
    return res;
}

int main() {
    int a = 10;
    int b = 5;
    int c = 2;
    int result = 0;
    bool flag1 = true;
    bool flag2 = false;

    result = a + b;
    result = a - b;
    result = a * b;
    result = a / b;
    result = a % b;
    a++;
    b--;
    a = !flag1;

    if (a < b) {}
    if (a > b) {}
    if (a == b) {}
    if (a != b) {}
    if (a >= b) {}
    if (a <= b) {}
    if (flag1 && flag2) {}
    if (flag1 || flag2) {}

    result = a >> 1;
    result = a << 1;
    result = a & b;
    result = a ^ b;
    result = a | b;

    result = calculate_sum_diff(a, b);
    result = check_bitwise(result);
    result = check_modulus_and_division(a, c);
    result = simple_multiplication(b, c);
    
    for (int i = 0; i < 5; i++) {
        result = a + b;
        result = a + b;
        result = a + b;
        result = a + b;
        result = a + b;
    }

    for (int j = 0; j < 3; j++) {
        if (j == 1) {
            flag1 = false;
        } else {
            flag1 = true;
        }
    }
    
    for (int k = 0; k < 4; k++) {
        result = result ^ k;
        result = result | (k << 1);
    }
    
    for (int l = 0; l < 6; l++) {
        a = a * 2;
        b = b / 2;
        a++;
        b--;
    }

    for (int m = 0; m < 2; m++) {
        if (m == 0 && flag1) {
            flag2 = false;
        } else if (m == 1 || !flag1) {
            flag2 = true;
        }
    }
    
    a = 10; b = 5;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    
    flag1 = true; flag2 = false;
    if (a < b) {} if (a > b) {} if (a == b) {}
    if (flag1 && flag2) {} if (flag1 || flag2) {}
    result = a >> 1; result = a << 1; result = a & b; result = a ^ b; result = a | b;
    
    a = 10; b = 5;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    
    flag1 = true; flag2 = false;
    if (a < b) {} if (a > b) {} if (a == b) {}
    if (flag1 && flag2) {} if (flag1 || flag2) {}
    result = a >> 1; result = a << 1; result = a & b; result = a ^ b; result = a | b;
    
    a = 10; b = 5;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    
    flag1 = true; flag2 = false;
    if (a < b) {} if (a > b) {} if (a == b) {}
    if (flag1 && flag2) {} if (flag1 || flag2) {}
    result = a >> 1; result = a << 1; result = a & b; result = a ^ b; result = a | b;
    
    a = 10; b = 5;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    
    flag1 = true; flag2 = false;
    if (a < b) {} if (a > b) {} if (a == b) {}
    if (flag1 && flag2) {} if (flag1 || flag2) {}
    result = a >> 1; result = a << 1; result = a & b; result = a ^ b; result = a | b;
    
    a = 10; b = 5;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    
    flag1 = true; flag2 = false;
    if (a < b) {} if (a > b) {} if (a == b) {}
    if (flag1 && flag2) {} if (flag1 || flag2) {}
    result = a >> 1; result = a << 1; result = a & b; result = a ^ b; result = a | b;
    
    a = 10; b = 5;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    
    flag1 = true; flag2 = false;
    if (a < b) {} if (a > b) {} if (a == b) {}
    if (flag1 && flag2) {} if (flag1 || flag2) {}
    result = a >> 1; result = a << 1; result = a & b; result = a ^ b; result = a | b;
    
    a = 10; b = 5;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    
    flag1 = true; flag2 = false;
    if (a < b) {} if (a > b) {} if (a == b) {}
    if (flag1 && flag2) {} if (flag1 || flag2) {}
    result = a >> 1; result = a << 1; result = a & b; result = a ^ b; result = a | b;
    
    a = 10; b = 5;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    
    flag1 = true; flag2 = false;
    if (a < b) {} if (a > b) {} if (a == b) {}
    if (flag1 && flag2) {} if (flag1 || flag2) {}
    result = a >> 1; result = a << 1; result = a & b; result = a ^ b; result = a | b;
    
    a = 10; b = 5;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    result = a + b; result = a - b; result = a * b; result = a / b;
    a++; b--;
    
    flag1 = true; flag2 = false;
    if (a < b) {} if (a > b) {} if (a == b) {}
    if (flag1 && flag2) {} if (flag1 || flag2) {}
    result = a >> 1; result = a << 1; result = a & b; result = a ^ b; result = a | b;
    
    a = 10; b = 5;
    result = a + b; 
    result = a - b; 
    result = a * b; 
    result = a / b;
    a++; 
    b--;
    result = a + b; 
    result = a - b; 
    result = a * b; 
    result = a / b;
    a++; 
    b--;
    
    flag1 = true; flag2 = false;
    if (a < b) {} if (a > b) {} if (a == b) {}
    if (flag1 && flag2) {} if (flag1 || flag2) {}
    result = a >> 1; result = a << 1; result = a & b; result = a ^ b; result = a | b;
    
    result = a + b; 
    result = a - b; 
    result = a * b; 
    result = a / b;
    a++; 
    b--;
    result = a + b; 
    result = a - b; 
    result = a * b; 
    result = a / b;
    a++; 
    b--;
    
    flag1 = true; flag2 = false;
    if (a < b) {} if (a > b) {} if (a == b) {}
    if (flag1 && flag2) {} if (flag1 || flag2) {}
    result = a >> 1; result = a << 1; result = a & b; result = a ^ b; result = a | b;

    return 0;
}
