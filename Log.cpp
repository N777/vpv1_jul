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
int progression = 1;
int multi;
bool fib = true;

unsigned __int64 fibRecursive(unsigned __int64 n) {
    if (n < 2) return n;
    return fibRecursive(n - 1) + fibRecursive(n - 2);
}

long double A[10000000];
long double B[10000000];
long double X[10000000];
int point;
double my_func(unsigned __int64 i){
    if (fib){
        return fibRecursive(i);
    }
    double asd = 0;
    for (int j = 0; j < progression; ++j) {
        asd += A[i] * log(cos(X[i] - B[i]));
    }
    return asd;
}

void fillArr(long n) {
    for (int i = 0; i < n; i++) {
        A[i] = (rand() % 10000) / (10000 * 1.0);
        B[i] = (rand() % 10000) / (10000 * 1.0);
        X[i] = (rand() % 10000) / (10000 * 1.0);
    }
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
    double res;
    clock_t t_start, t_finish;
    __int64 t_code;
    clock_t t_clock = clock();
    while (clock() < t_clock + 1);
    t_start = clock();
    for (int i = 0; i < counter; i++)
    {
        res = my_func(nums);
    }
    while (clock() < t_clock + 2);
    t_finish = clock();
    t_code = t_finish - t_start;
    return double(t_code) / CLOCKS_PER_SEC;
};


// Длительность одного clock-интервала с помощью QPC
double сlockIntervalUsingQPC() {
    double res;
	LARGE_INTEGER t_start, t_finish, freq;
	__int64 t_code;
	QueryPerformanceFrequency(&freq);
	clock_t t_clock = clock();
	while (clock() < t_clock + 1);
	QueryPerformanceCounter(&t_start);
    for (int i = 0; i < counter; i++)
    {
        res = my_func(nums);
    }
	while (clock() < t_clock + 2);
	QueryPerformanceCounter(&t_finish);
	t_code = t_finish.QuadPart - t_start.QuadPart;
	return double(t_code) / freq.QuadPart;
};

// Длительность одного сlock-интервала в тактах TSC
double сlockIntervalUsingTSC() {
    double res;
	__int64 t_start, t_finish;
	clock_t t_clock = clock();
	while (clock() < t_clock + 1);
	t_start = __rdtsc();
    res = my_func(nums);
	while (clock() < t_clock + 2);
	t_finish = __rdtsc();
	return double(t_finish - t_start) / getFrequency();
};

int main() {
	Log log;
	Log suplog;
	double scale = MCS_IN_SEC; // масштаб выводимых значений
	int nPasses = 5;  // число проходов при выполнении серий измерений
    fillArr(10000);
	setlocale(LC_CTYPE, "rus");
	cpuInfo();
    log.config({ {PREC_AVG, 2},
                 {FILTR_MIN,0},{FILTR_MAX, 0} });
    point = 5;
    switch (point) {
        case 2:
        {
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
        case 3:{
            int max_sum = 0;
            double arr[2] = { 0,0 };
            while (arr[0] == 0 || arr[1] == 0) {
                int sum = 0;
                max_sum += 10;
                for (int i = 0; i < 2; i++)
                {
                    clock_t start = clock();
                    for (int i = 0; i < max_sum; i++)
                    {
                        sum += 1;
                    }
                    clock_t end = clock();
                    arr[i] = double(end - start) / CLOCKS_PER_SEC;
                }
            }
            cout << "clock = " << min(arr[0], arr[1]) << endl;

            max_sum = 0;
            arr[0] = 0;
            arr[1] = 0;
            while (arr[0] == 0 || arr[1] == 0) {
                LARGE_INTEGER t_start, t_finish, freq;
                int sum = 0;
                max_sum += 1;
                for (int i = 0; i < 2; i++)
                {
                    __int64 t_code;
                    QueryPerformanceFrequency(&freq);
                    QueryPerformanceCounter(&t_start);
                    for (int i = 0; i < max_sum; i++)
                    {
                        sum += 1;
                    }
                    QueryPerformanceCounter(&t_finish);
                    t_code = t_finish.QuadPart - t_start.QuadPart;
                    arr[i] = double(t_code) / freq.QuadPart;
                }
            }
            cout << "QPC = " << min(arr[0], arr[1]) << endl;

            max_sum = 0;
            arr[0] = 0;
            arr[1] = 0;
            while (arr[0] == 0 || arr[1] == 0) {
                LARGE_INTEGER t_start, t_finish, freq;
                int sum = 0;
                max_sum += 1;
                for (int i = 0; i < 2; i++)
                {
                    __int64 t_start, t_finish;
                    t_start = __rdtsc();
                    t_finish = __rdtsc();
                    arr[i] = double(t_finish - t_start) / getFrequency();
                }
            }
            cout << "TSC = " << min(arr[0], arr[1]) << endl;
            break;
        }
        case 4:{
            fib = false;
            counter = 1e5;
            for (int i = 0; i < 10; ++i) {
                log.series(true, 1000, сlockIntervalUsingClock, nums)
                        .calc().stat(scale, "Оценка повторяемости 1000 измерений clock - интервала через clock без фильтрации")
                        .print(O_NATURAL, scale, 10).print(O_MIN, scale, 10).print(O_MAX, scale, 10);
            }
            for (int i = 0; i < 10; ++i) {
                log.series(true, 1000, сlockIntervalUsingQPC, nums)
                        .calc().stat(scale, "Оценка повторяемости 1000 измерений clock - интервала через QPC без фильтрации")
                        .print(O_NATURAL, scale, 10).print(O_MIN, scale, 10).print(O_MAX, scale, 10);
            }

            for (int i = 0; i < 10; ++i) {
                log.series(true, 1000, сlockIntervalUsingTSC, nums)
                        .calc().stat(NS_IN_SEC, "Оценка повторяемости 1000 измерений clock - интервала через TSC без фильтрации")
                        .print(O_NATURAL, scale, 10).print(O_MIN, scale, 10).print(O_MAX, scale, 10);
            }
            break;
        }
        case 5:{
            fib = false;
            vector<double> v;
            progression = 10;
            multi = 10;
            for (int i = 0; i < 1000; ++i) {
                double res;
                clock_t t_start, t_finish;
                __int64 t_code;
                clock_t t_clock = clock();
                while (clock() < t_clock + 1);
                t_start = clock();
                for (int i = 0; i < counter; i++)
                {
                    res = my_func(i);
                }
                while (clock() < t_clock + 2);
                t_finish = clock();
                t_code = t_finish - t_start;
                progression += multi;
                v.push_back(double(t_code) / CLOCKS_PER_SEC);
            }
            double k = v[v.size()-1] / 1000;
            for (int i = 0; i < v.size(); ++i) {
                v[i] = abs(v[i] - k);
            }
            log.set(true, v).calc().stat(scale, "Оценка повторяемости 1000 измерений clock - интервала через clo");
            cout << "k = " << k * scale;


            progression = 10;
            multi = 10;
            for (int i = 0; i < 1000; ++i) {
                double res;
                LARGE_INTEGER t_start, t_finish, freq;
                __int64 t_code;
                QueryPerformanceFrequency(&freq);
                clock_t t_clock = clock();
                while (clock() < t_clock + 1);
                QueryPerformanceCounter(&t_start);
                for (int i = 0; i < counter; i++)
                {
                    res = my_func(i);
                }
                while (clock() < t_clock + 2);
                QueryPerformanceCounter(&t_finish);
                t_code = t_finish.QuadPart - t_start.QuadPart;
                progression += multi;
                v.push_back(double(t_code) / freq.QuadPart);
            }
            k = v[v.size()-1] / 1000;
            for (int i = 0; i < v.size(); ++i) {
                v[i] = abs(v[i] - k);
            }
            log.set(true, v).calc().stat(scale, "Оценка повторяемости 1000 измерений QPC");
            cout << "k = " << k * scale;

            progression = 10;
            multi = 10;
            for (int i = 0; i < 1000; ++i) {
                double res;
                __int64 t_start, t_finish;
                clock_t t_clock = clock();
                while (clock() < t_clock + 1);
                t_start = __rdtsc();
                res = my_func(i);
                while (clock() < t_clock + 2);
                t_finish = __rdtsc();
                progression += multi;
                v.push_back(double(t_finish - t_start) / getFrequency());
            }
            k = v[v.size()-1] / 1000;
            for (int i = 0; i < v.size(); ++i) {
                v[i] = abs(v[i] - k);
            }
            log.set(true, v).calc().stat(NS_IN_SEC, "Оценка повторяемости 1000 измерений TSC");
            cout << "k = " << k * NS_IN_SEC;
        }

    }

	return 0;
}