// F(x) = (x * 7 + 1) * (x ^ 2 - 2 * pi)

#include <math.h>
#include <ctime>
#include <iostream>
#include <Windows.h>

using namespace std;

int main()
{
    float intervalStart, intervalEnd, intervalStep, y = 7, fpuResult = 0, cmathResult = 0;
    char retryAnswer;

    LARGE_INTEGER frequency, fpuStartTime, fpuEndTime, cmathStartTime, cmathEndTime;
    QueryPerformanceFrequency(&frequency);

    do
    {
        system("CLS");
        cout << "Interval start input: ";
        cin >> intervalStart;
        cout << "Interval end input: ";
        cin >> intervalEnd;
        cout << "Step input: ";
        cin >> intervalStep;

        if (intervalStart > intervalEnd) {
            auto temp = intervalStart;
            intervalStart = intervalEnd;
            intervalEnd = temp;
        }

        float x = intervalStart;
        QueryPerformanceCounter(&fpuStartTime);
        do
        {
            _asm {
                fld y
                fld x
                fmul
                fld1
                fadd

                fld x
                fld x
                fmul

                fldpi
                fsub
                fldpi
                fsub

                fmul

                fld fpuResult
                fadd

                fstp fpuResult
            }
            _asm fwait

            x += intervalStep;
        } while (x <= intervalEnd);
        QueryPerformanceCounter(&fpuEndTime);

        QueryPerformanceCounter(&cmathStartTime);
        x = intervalStart;
        do
        {
            cmathResult += (x * 7 + 1) * (pow(x, 2) - 2 * 3.14159265358979323846);

            x += intervalStep;
        } while (x <= intervalEnd);
        QueryPerformanceCounter(&cmathEndTime);

        float fpuTime = (fpuEndTime.QuadPart - fpuStartTime.QuadPart) * 1000.0f / frequency.QuadPart;
        float cmathTime = (cmathEndTime.QuadPart - cmathStartTime.QuadPart) * 1000.0f / frequency.QuadPart;

        cout << endl;
        cout << "FPU result: " << fpuResult << endl;
        cout << "FPU time: " << fpuTime << endl;
        cout << endl;
        cout << "CMATH result: " << cmathResult << endl;
        cout << "CMATH time: " << cmathTime << endl;

        cout << endl;
        cout << "Do you want to continue?(y/n) ";
        cin >> retryAnswer;
        system("CLS");
    } while (retryAnswer != 'n');

    return 0;
}
