// 26.    F(x) = (x * 7 + 1)  *  (x ^ 2 - 2 * pi)
#define _USE_MATH_DEFINES

#include <math.h>
#include <ctime>
#include <iostream>
#include <Windows.h>

using namespace std;

int main()
{
    float x, y = 7, fpuResult, cmathResult;
    char retryAnswer;

    // Time measurement variables
    LARGE_INTEGER frequency, fpuStartTime, fpuFinishTime, cmathStartTime, cmathFinishTime;
    QueryPerformanceFrequency(&frequency);

    do
    {
        system("CLS");
        cout << "F(X) = (X * 7 + 1)  *  (X ^ 2 - 2 * PI)" << endl;
        cout << "X variable input: ";
        cin >> x;

        QueryPerformanceCounter(&fpuStartTime);
        _asm {
            fld y           // ST(0) = 7
            fld x            // ST(0) = x, ST(1) = 7
            fmul            // ST(0) = x * 7
            fld1            // ST(0) = 1, ST(1) = x * 7
            fadd            // ST(0) = x * 7 + 1

            fld x            // ST(1) = x
            fld x            // ST(2) = x
            fmul            // ST(1) = x ^ 2
        
            fldpi           // ST(2) = pi
            fsub            // ST(1) = x ^ 2 - pi
            fldpi            // ST(2) = pi
            fsub            // ST(1) = x ^ 2 - 2 * pi

            fmul            // ST(0) = (x * 7 + 1)  *  (x ^ 2 - 2*  pi)

            fstp fpuResult
        }
        _asm fwait
        QueryPerformanceCounter(&fpuFinishTime);

        QueryPerformanceCounter(&cmathStartTime);
        cmathResult = (x * 7 + 1) * (pow(x, 2) - 2 * M_PI);
        QueryPerformanceCounter(&cmathFinishTime);

        float fpuTime = (fpuFinishTime.QuadPart - fpuStartTime.QuadPart) * 1000.0f / frequency.QuadPart;
        float cmathTime = (cmathFinishTime.QuadPart - cmathStartTime.QuadPart) * 1000.0f / frequency.QuadPart;

        cout << endl;
        cout << "FPU result: " << fpuResult << endl;
        cout << "FPU time: " << fpuTime << endl;
        cout << endl;
        cout << "CMATH result: " << cmathResult << endl;
        cout << "CMATH time: " << cmathTime << endl;

        cout << endl;
        cout << "Do you want to continue? (y/n): ";
        cin >> retryAnswer;
        system("CLS");
    } while (retryAnswer != 'n');

    return 0;
}
