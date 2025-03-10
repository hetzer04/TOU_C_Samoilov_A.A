#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <windows.h>

void generateStory()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::vector<std::string> heroes = {"смелый рыцарь", "хитрый вор", "волшебник", "отважный пират", "дерзкий исследователь"};
    std::vector<std::string> places = {"в далёком королевстве", "на заброшенной фабрике", "в густом лесу", "на просторах космоса", "у подножия гор"};
    std::vector<std::string> actions = {"победил дракона", "обнаружил сокровища", "выиграл битву", "устроил бал", "раскрыл древнюю тайну"};
    std::vector<std::string> details = {"с волшебным мечом", "на летающем ковре", "под звуки волшебной музыки", "с удивительной силой", "в сопровождении магического существа"};

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 4);

    std::string story = heroes[distrib(gen)] + " " + places[distrib(gen)] + ", " + actions[distrib(gen)] + " " + details[distrib(gen)] + ".";

    std::cout << "Сгенерированная история:\n"
              << story << "\n";

    char save;
    std::cout << "Сохранить историю в файл? (Y/N): ";
    std::cin >> save;

    if (save == 'Y' || save == 'y')
    {
        std::ofstream file("stories.txt", std::ios::app);
        if (file.is_open())
        {
            file << story << "\n";
            file.close();
            std::cout << "История сохранена!\n";
        }
        else
        {
            std::cout << "Ошибка сохранения!\n";
        }
    }
}

int main()
{
    char choice;
    do
    {
        generateStory();
        std::cout << "Хотите сгенерировать новую историю? (Y/N): ";
        std::cin >> choice;
    } while (choice == 'Y' || choice == 'y');

    return 0;
}
