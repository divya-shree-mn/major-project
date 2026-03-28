typedef unsigned short UInt16;

const UInt16 m1[3][4] = {
{0x01, 0x02, 0x03, 0x04},
{0x05, 0x06, 0x07, 0x08},
{0x09, 0x0A, 0x0B, 0x0C}
};

const UInt16 m2[4][5] = {
{0x01, 0x02, 0x03, 0x04, 0x05},
{0x06, 0x07, 0x08, 0x09, 0x0A},
{0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
{0x10, 0x11, 0x12, 0x13, 0x14}
};

int main(void)
{
int m, n, p;
volatile UInt16 m3[3][5];
for(m = 0; m < 3; m++) // 1 + 3 * 2 // 3 * 3
{
for(p = 0; p < 5; p++) // 3 + 15 * 2 // 15 * 3
{
m3[m][p] = 0; // 15: BasicOperation(operation_name='=', type_lhs='unsigned short', type_rhs='unsigned short', type_result='unsigned short')
for(n = 0; n < 4; n++) // 15 + 60 * 2 // 60 * 3
{
m3[m][p] += m1[m][n] * m2[n][p];
}
}
}
return 0;
}

