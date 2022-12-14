#include <iostream>
#include <mutex>
#include <thread>
#include<string>
#include <clocale>
#include <vector>
#include <string.h>
#include <stdlib.h>

using namespace std;

mutex mtx;
static int gardenBeds[40]; //состояния каждой ячейки. 1-требует полива. 0-полита.

// Функция, которая позволяет генерировать некоторое количество осушенных
// ячеек в течение некоторого времени параллельно с работой садовника
void changeGarden(int count) {
    for (int i = 0; i < count; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1200));
        int index = rand() % 40;
        gardenBeds[index] = 1;
        string outp = to_string(index) + " грядку надо полить.\n";
        cout << outp;
    }
}

// Функция, которая меняет статус грядки на политую.
void waterCell(int ind, int threadId, FILE *fout, char *out) {
    if (gardenBeds[ind] == 0) {
        return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    string outp = to_string(ind) + " полито " + to_string(threadId) + " садовником.\n";
    gardenBeds[ind] = 0;
    if (strcmp(out, "") == 0) {
        cout << outp;
    } else {
        fprintf(fout, "%s", outp.c_str());
    }
}

// функция по присвоению той или иной ячейки одному из садовников.
// initialSeedBedNumber - изначально неполитых грядок.
// threadId - номер потока.
void chooseCellToWater(int initialSeedBedNumber, int threadId, FILE *fout, char *out) {
    for (int j = 0; j < initialSeedBedNumber; j++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        mtx.lock();
        for (int i = 0; i < 40; ++i) {
            // добавляем в задачу полива неполитую грядку.
            if (gardenBeds[i] == 1) {
                waterCell(i, threadId, fout, out);
                break;
            }
            if (i == 39) {
                if (strcmp(out, "") == 0) {
                    cout << "Всё полито!\n";
                } else {
                    fprintf(fout, "Всё полито!\n");
                }
            }
        }
        mtx.unlock();
    }
}

int GetRandomNumber(int min, int max)
{
    // Получить случайное число - формула
    int num = min + rand() % (max - min + 1);

    return num;
}


int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    char *key = "";
    FILE *key2 = NULL;
    if (strcmp(argv[1], "-r") == 0) {
        int x;
        x = GetRandomNumber(3, 8);
        int actionsNum = x;
        cout << endl << "Грядок требуют полива: " << actionsNum << endl;
        // вводим кол-во случаев, когда грядка захочет быть политой
        if (actionsNum <= 0)
            return 1;
        // данный поток в определенное время делает ячейку "не политой".
        thread gardenChanger(changeGarden, actionsNum * 2);
        // эти потоки проверяют состояние грядок и, при необходимости, начинают полив. Одновременно поливает
        // один садовник.
        thread gardener1(chooseCellToWater, actionsNum, 1, key2, key);
        thread gardener2(chooseCellToWater, actionsNum, 2, key2, key);
        gardenChanger.join();
        gardener1.join();
        gardener2.join();
    } else if (strcmp(argv[1], "-f") == 0) {
        FILE *input = fopen(argv[2], "r");
        FILE *fout = fopen(argv[3], "w");
        if (input == NULL) {
            printf("Некорректный или несуществующий файл!\n");
            return 0;
        }
        int actionsNum;
        fscanf(input, "%d", &actionsNum); // вводим кол-во случаев, когда грядка захочет быть политой
        if (actionsNum <= 0)
            return 1;
        // данный поток с какой-то переодичностью делает какую-то клетку не политой
        thread gardenChanger(changeGarden, actionsNum * 2);
        // эти потоки мониторят грядки и в случае нахождения не политой - поливают
        // причем поливает только 1 из двух садовников
        thread gardener1(chooseCellToWater, actionsNum, 1, fout, argv[3]);
        thread gardener2(chooseCellToWater, actionsNum, 2, fout, argv[3]);
        gardenChanger.join();
        gardener1.join();
        gardener2.join();
        fclose(input);
        fclose(fout);
    } else if ((strcmp(argv[1], "-s") == 0)) {
        int actionsNum;
        cin >> actionsNum; // вводим кол-во случаев, когда грядка захочет быть политой
        if (actionsNum < 0) {
            cout << "Некорректный ввод. Введите неотрицательное число.";
            return 1;
        }

        // данный поток с какой-то переодичностью делает какую-то клетку не политой
        thread gardenChanger(changeGarden, actionsNum * 2);
        // эти потоки мониторят грядки и в случае нахождения не политой - поливают
        // причем поливает только 1 из двух садовников
        thread gardener1(chooseCellToWater, actionsNum, 1, key2, key);
        thread gardener2(chooseCellToWater, actionsNum, 2, key2, key);
        gardenChanger.join();
        gardener1.join();
        gardener2.join();
    }
    return 0;
}