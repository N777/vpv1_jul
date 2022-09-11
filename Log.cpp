/////////////////////////////////////////////////////////////
// Log.cpp - файл отладки-демонстрации использования класса 
// протоколирования, обработки, вывода результатов измерений 
// nvnulstu@gmail.com: февраль 2022
// Предполагается, что файл класса Log.h находится в материнском директории,
// а несколько использующих его приложений находятся в дочерних

#include <iostream>
#include <Windows.h>
#include <time.h>
#include "Log.h"

using namespace std;

#define NS_IN_SEC 1.E9 // число наносекунд в секунде
#define MCS_IN_SEC 1.E6 // число микросекунд в секунде

// Макросы повторения для генерации продуктов разворачивания циклов
#define Repeat10(x) x x x x x x x x x x 
#define Repeat100(x) Repeat10(Repeat10(x))
#define Repeat1000(x) Repeat100(Repeat10(x))
#define Repeat10000(x) Repeat100(Repeat100(x))

int nums = 10;
int counter = 1e4;

unsigned __int64 fibRecursive(unsigned __int64 n) {
    if (n < 2) return n;
    return fibRecursive(n - 1) + fibRecursive(n - 2);
}

unsigned long long getFrequency() {
    clock_t tclock = clock();
    while (clock() < tclock + 1); // ожидание конца начавшегося такта
    tclock = clock();
    unsigned long long tsc = __rdtsc();
    while (clock() < tclock + 1);	// ожидание конца начавшегося такта
    unsigned long long tscEnd = __rdtsc();
    unsigned long long tscDelta = tscEnd - tsc;// сколько тактов TSC прошло за один такт clock
    unsigned long long Fl = (tscDelta * CLOCKS_PER_SEC); // частота процессора
    tclock = clock(); tsc = __rdtsc();
    while (clock() < tclock + 1); // ожидание конца начавшегося такта
    tscEnd = __rdtsc(); tscDelta = tscEnd - tsc;// сколько тактов TSC прошло за один такт clock
    unsigned long long F2 = (tscDelta * CLOCKS_PER_SEC); // частота процессора
    return min(Fl, F2);
}

// Длительность одного clock-интервала с помощью QPC
double сlockIntervalUsingClock() {
    unsigned __int64 res;
    clock_t t_start, t_finish;
    __int64 t_code;
    clock_t t_clock = clock();
    while (clock() < t_clock + 1);
    t_start = clock();
    for (int i = 0; i < counter; i++)
    {
        res = fibRecursive(nums);
    }
    while (clock() < t_clock + 2);
    t_finish = clock();
    t_code = t_finish - t_start;
    return double(t_code) / CLOCKS_PER_SEC;
};


// Длительность одного clock-интервала с помощью QPC
double сlockIntervalUsingQPC() {
    unsigned __int64 res;
	LARGE_INTEGER t_start, t_finish, freq;
	__int64 t_code;
	QueryPerformanceFrequency(&freq);
	clock_t t_clock = clock();
	while (clock() < t_clock + 1);
	QueryPerformanceCounter(&t_start);
    for (int i = 0; i < counter; i++)
    {
        res = fibRecursive(nums);
    }
	while (clock() < t_clock + 2);
	QueryPerformanceCounter(&t_finish);
	t_code = t_finish.QuadPart - t_start.QuadPart;
	return double(t_code) / freq.QuadPart;
};

// Длительность одного сlock-интервала в тактах TSC
double сlockIntervalUsingTSC() {
    unsigned __int64 res;
	__int64 t_start, t_finish;
	clock_t t_clock = clock();
	while (clock() < t_clock + 1);
	t_start = __rdtsc();
    res = fibRecursive(nums);
	while (clock() < t_clock + 2);
	t_finish = __rdtsc();
	return double(t_finish - t_start) / getFrequency();
};

int main() {
	Log log;
	Log suplog;
	double scale = MCS_IN_SEC; // масштаб выводимых значений
	int nPasses = 5;  // число проходов при выполнении серий измерений
	setlocale(LC_CTYPE, "rus");
	cpuInfo();
    int point = 2;

    switch (point) {
        case 2:
        {
            log.config({ {PREC_AVG, 2},
                         {FILTR_MIN,0},{FILTR_MAX, 0} });
            log.series(true, 1, сlockIntervalUsingClock)
                    .calc().stat(scale, "Число микросекунд в одном clock-интервале через clock (без фильтрации)");

            log.series(true, 1000, сlockIntervalUsingClock)
                    .calc().stat(scale, "Оценка повторяемости 1000 измерений clock - интервала через clock без фильтрации");

            log.series(true, 1000, сlockIntervalUsingQPC)
                    .calc().stat(scale, "Оценка повторяемости 1000 измерений clock - интервала через QPC без фильтрации");

            log.series(true, 1000, сlockIntervalUsingTSC)
                    .calc().stat(NS_IN_SEC, "Оценка повторяемости 1000 измерений clock - интервала через TSC без фильтрации");
            break;
        }
    }

	return 0;
}