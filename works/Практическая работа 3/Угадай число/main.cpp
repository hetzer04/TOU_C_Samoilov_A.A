#include <iostream>
#include <random>
#include <windows.h>

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 100);

    int secretNumber = distrib(gen);
    int guess;
    int attempts = 0;
    bool guessed = false;

    std::cout << "Игра 'Угадай число'!\n";
    std::cout << "Я загадал число от 1 до 100. Попробуйте угадать!\n";

    while (!guessed)
    {
        std::cout << "Введите число: ";
        std::cin >> guess;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Ошибка! Введите число.\n";
            continue;
        }

        attempts++;

        if (guess < secretNumber)
        {
            std::cout << "Загаданное число больше.\n";
        }
        else if (guess > secretNumber)
        {
            std::cout << "Загаданное число меньше.\n";
        }
        else
        {
            std::cout << "Поздравляю! Вы угадали число за " << attempts << " попыток.\n";
            guessed = true;
        }
    }

    return 0;
}
