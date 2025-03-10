#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <ctime>

struct Character {
    std::string name;
    int health;
    int attack;
    int defense;

    void attackTarget(Character &target) {
        int damage = attack - target.defense;
        if (damage < 1) damage = 1; // Минимальный урон
        target.health -= damage;
        std::cout << name << " атакует " << target.name << " и наносит " << damage << " урона!\n";
    }
};

void printStatus(const Character &hero, const Character &monster) {
    std::cout << "\n--- Текущий статус ---\n";
    std::cout << hero.name << " (Здоровье: " << hero.health << ")\n";
    std::cout << monster.name << " (Здоровье: " << monster.health << ")\n";
    std::cout << "----------------------\n\n";
}


int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // Генерация случайных чисел

    Character hero = {"Герой", 100, 20, 5};
    Character monster = {"Монстр", 80, 15, 3};

    std::cout << "Игра 'Битва с монстром' началась!\n";

    while (hero.health > 0 && monster.health > 0) {
        printStatus(hero, monster);

        std::cout << "Выберите действие:\n1. Атаковать\n2. Пропустить ход\n";
        int choice;
        std::cin >> choice;

        if (std::cin.fail() || (choice != 1 && choice != 2)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Некорректный ввод! Попробуйте снова.\n";
            continue;
        }

        if (choice == 1) {
            hero.attackTarget(monster);
        }

        if (monster.health > 0) {
            monster.attackTarget(hero);
        }
    }

    printStatus(hero, monster);

    if (hero.health > 0) {
        std::cout << "Поздравляем! Вы победили монстра!\n";
    } else {
        std::cout << "Монстр победил! Попробуйте еще раз.\n";
    }

    return 0;
}