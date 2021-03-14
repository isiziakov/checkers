#include <iostream>
#include <Windows.h>
#include <time.h>
#include <ctime>
#include <locale.h>
#include <stdio.h>
#include <conio.h>


struct cell_field // информация о клетке поля
{
    bool king; // является ли дамкой
    int checker_color; // занята ли шашкой: 1 - белой, 2 - черной, 0 - пуста
    char letter; // буквенное обозначение клетки
    int number; // числовое обозначение клетки
};

struct game_information // информация об игре
{
    cell_field field[32]; // информация о 32-ух клетках, которые могут занимать шашки
    int player; // номер текущего игрока
    bool game_over; // закончена ли игра
};

struct game_save // сохранение игры
{
    game_information information; // информация об игре
    char name[31]; // имя сохранения
    char date[11]; // дата создания
    int type; // игра против игрока (1) или компьютера (2)
    bool active; // содержит ли сохранение информацию
};

struct game_saves // 20 доступных ячеек для сохранения
{
    game_save save[20];
};

struct bot_game_settings // настройки игры компьютера
{
    int turns; // количество ходов, на которое анализирует компьютер
    int attack; // бонус к очкам за съедение шашек
    int king; // бонус к очкам за становление дамкой и съедение дамки
    int position; // бонус к очкам за занятие / оставление позиций (верхний / нижний ряд, главная диагональ, клетки а7 и h2)
}settings;

struct can_turn // структура для хранения позиции клетки в формате буква-цифра
{
    char letter;
    int number;
};

struct bot_turn // структура для хранения ходов компьютера (несколько для съедения нескольких шашек подряд)
{
    can_turn first[13]; // первая клетка
    can_turn second[13]; // вторая клетка
    bool eat = false; // ест ли шашки в этом ходу
};

void Open_file(char filename[]) // открытие файла со справкой / правилами
{
    FILE* file;
    char str[512];
    fopen_s(&file, filename, "rt");
    if (file) // если файл найден
    {
        while (feof(file) == 0)
        {
            fgets(str, 512, file);
            printf("%s\n", str); // вывод
        }
        fflush(stdin);
        fclose(file); // закрытие файла
    }
    else // возможно, если файлы были удалены вручную
    {
        printf("Ошибка! Файлы игры повреждены\n");
    }
    system("pause");
}

void Change_settings() // изменение настроек поведения компьютера
{
    char buffer[100];
    FILE* file;
    fopen_s(&file, "Settings", "rb+"); // существование файла проверяется при запуске
    do
    {
        printf("Доступные параметры - команда для изменения\n");
        printf("Приоритет компьютера на защиту (-1) / атаку (1), текущее %d - attack\n", settings.attack);
        printf("Приоритет компьютера на проведение шашек в дамки (от -1 до 1), текущее %d - king\n", settings.king);
        printf("Приоритет на занятие шашками выгодных позиций от (от -1 до 1), текущее %d - pos\n", settings.position);
        printf("Глубина, на которую компьютер анализирует свой ход, чем больше тем медленние, но лучше ходит компьютер (1..4), текущее %d - turn\n", settings.turns);
        printf("Для изменения параметра введите команду и через пробел новое значение\n");
        printf("Завершение и сохранение изменений - exit\n\\");
        gets_s(buffer);
        if (buffer[0] == 'a' && buffer[1] == 't' && buffer[2] == 't' && buffer[3] == 'a' && buffer[4] == 'c' && buffer[5] == 'k') // определение, какой параметр изменять
        {
            if (buffer[7] == '-' && buffer[8] == '1') // на сколько изменять
            {
                settings.attack = -1;
            }
            else
            {
                if (buffer[7] == '0')
                {
                    settings.attack = 0;
                }
                else
                {
                    if (buffer[7] == '1')
                    {
                        settings.attack = 1;
                    }
                    else
                    {
                        printf("Неверное значение\n");
                    }
                }
            }
        }
        if (buffer[0] == 'k' && buffer[1] == 'i' && buffer[2] == 'n' && buffer[3] == 'g')
        {
            if (buffer[5] == '-' && buffer[6] == '1')
            {
                settings.king = -1;
            }
            else
            {
                if (buffer[5] == '0')
                {
                    settings.king = 0;
                }
                else
                {
                    if (buffer[5] == '1')
                    {
                        settings.king = 1;
                    }
                    else
                    {
                        printf("Неверное значение\n");
                    }
                }
            }
        }
        if (buffer[0] == 'p' && buffer[1] == 'o' && buffer[2] == 's')
        {
            if (buffer[4] == '-' && buffer[5] == '1')
            {
                settings.position = -1;
            }
            else
            {
                if (buffer[4] == '0')
                {
                    settings.position = 0;
                }
                else
                {
                    if (buffer[4] == '1')
                    {
                        settings.position = 1;
                    }
                    else
                    {
                        printf("Неверное значение\n");
                    }
                }
            }
        }
        if (buffer[0] == 't' && buffer[1] == 'u' && buffer[2] == 'r' && buffer[3] == 'n')
        {
            if (buffer[5] > '0' && buffer[5] < '5')
            {
                settings.turns = buffer[5] - '0';
            }
            else
            {
                printf("Неверное значение\n");
            }
        }
    } while (!strstr(buffer, "exit")); // пока не все изменения внесены
    fwrite(&settings, sizeof(bot_game_settings), 1, file); // сохранение изменений в файл
    fflush(stdin);
    fclose(file);
}

void Getting_settings() // получение настроек из файла
{
    FILE* file;
    fopen_s(&file, "Settings", "rb");
    if (file)
    {
        fread(&settings, sizeof(bot_game_settings), 1, file);
    }
    else // если файла нет, создать его и заполнить значениями по умолчанию
    {
        settings.attack = 0;
        settings.king = 0;
        settings.position = 0;
        settings.turns = 3;
        fopen_s(&file, "Settings", "wb");
        fwrite(&settings, sizeof(bot_game_settings), 1, file);
    }
    fflush(stdin);
    fclose(file);
}

bool Be_in_turns(can_turn turns[], char first[], int counter) // проверяется есть ли клетка с позицией first в списке ходов (по конкретному номеру позиции в списке)
{
    if (first[0] == turns[counter].letter && first[1] == '0' + turns[counter].number)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Be_in_turns(can_turn turns[], char first[]) // проверяется есть ли клетка с позицией first в списке ходов полным перебором
{
    for (int i = 0; i < 15; i++)
    {
        if (first[0] == turns[i].letter && first[1] == turns[i].number + '0')
        {
            return true;
        }
    }
    return false;
}

void Print_Saves(game_saves saves) // вывод списка сохранений
{
    for (int i = 0; i < 20; i++)
    {
        printf("%-2d ", i + 1);
        printf("%-30s ", saves.save[i].name);
        printf("%s ", saves.save[i].date);
        if (saves.save[i].type == 1)
        {
            printf("Игрок против игрока");
        }
        if (saves.save[i].type == 2)
        {
            printf("Игрок против компьютера");
        }
        printf("\n");
    }
}

void Save(game_information information, int type) // сохранение игры
{
    time_t current_time;
    tm* tm_time = new tm();
    int number;
    game_saves saves;
    char save_name[100];
    FILE* file;
    fopen_s(&file, "Saves", "rb+");
    if (file) // есть ли файл
    {
        fread(&saves, sizeof(game_saves), 1, file);
        Print_Saves(saves);
        printf("Введите номер ячейки для сохранения, 0 - отмена\n\\");
        do
        {
            scanf_s("%d", &number); // считывание номера ячейки
            getchar();
        } while (number < 0 || number > 20); // пока введен неверный номер ячейки
        if (number != 0) // если не выход
        {
            number -= 1; // ячейки в файле нумеруются от 0 до 19
            printf("Введите название сохранения (максимум 30 символов)\n\\");
            gets_s(save_name); // ввод названия сохранения
            saves.save[number].active = true; // ячейка содержит сохранение
            saves.save[number].type = type; // тип игры в сохранении (с другим игроком или компьютером)
            saves.save[number].information = information; // сохранение информации о текущей игре
            current_time = time(NULL); // определение даты создания сохранения
            localtime_s(tm_time, &current_time);
            saves.save[number].date[0] = '0' + tm_time->tm_mday / 10;
            saves.save[number].date[1] = '0' + tm_time->tm_mday % 10;
            tm_time->tm_mon += 1;
            saves.save[number].date[3] = '0' + tm_time->tm_mon / 10;
            saves.save[number].date[4] = '0' + tm_time->tm_mon % 10;
            tm_time->tm_year += 1900;
            saves.save[number].date[6] = '0' + tm_time->tm_year / 1000;
            saves.save[number].date[7] = '0' + tm_time->tm_year / 100 % 10;
            saves.save[number].date[8] = '0' + tm_time->tm_year % 100 / 10;
            saves.save[number].date[9] = '0' + tm_time->tm_year % 10; 
            for (int i = 0; i < 30; i++)
            {
                saves.save[number].name[i] = save_name[i];
            }
            fseek(file, 0, SEEK_SET);
            fwrite(&saves, sizeof(game_saves), 1, file);
        }
        fflush(stdin);
        fclose(file);
    }
    else 
    {
        fopen_s(&file, "Saves", "wb"); // создание файла
        for (int i = 0; i < 20; i++) // заполнение всех ячеек значениями по умолчанию
        {
            saves.save[i].active = false;
            strcpy_s(saves.save[i].date, "00.00.0000");
            saves.save[i].type = 0;
            strcpy_s(saves.save[i].name, "Пусто");
        }
        fwrite(&saves, sizeof(game_saves), 1, file); // запись в фвйл
        fflush(stdin);
        fclose(file);
        Save(information, type); // вызов для сохранения игры в созданный файл
    }
}

int Load(game_information& information) // загрузка сохранения, возвращает тип загруженной игры
{
    int number;
    game_saves saves;
    FILE* file;
    fopen_s(&file, "Saves", "rb");
    if (file) // есть ли файл с сохранениями
    {
        fread(&saves, sizeof(game_saves), 1, file);
        Print_Saves(saves); // вывести все ячейки с сохранениями
        printf("Введите номер ячейки для загрузки, 0 - отмена\n");
        do
        {
            printf("\\");
            scanf_s("%d", &number); // выбрать номер ячейки для загрузки
            getchar(); // считывание символа перехода на новую строку
            number -= 1; // уменьшение номера ячейки, т.к. они нумеруются от 0 до 19
            if (number > -1 && number < 20 && saves.save[number].active == 0) // если ячейка не содержит сохранения
            {
                printf("Ячейка пуста\n");
                number = -2;
            }
        } while (number < -1 || number > 19); // пока ячейка пуста или введен неверный номер
        fflush(stdin);
        fclose(file);
        if (number != -1) // если выбрана ячейка
        {
            information = saves.save[number].information;
        }
        else // отмена загрузки
        {
            return 0;
        }
    }
    else
    {
        printf("Сохранений нет\n");
        return 0;
    }
    return saves.save[number].type; // вернуть тип сохранения (игра против игрока / компьютера)
}

void Clear(game_information& information) // очистка поля игры
{
    information.player = 1; // первый ход - белые
    information.game_over = false;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            information.field[i * 4 + j].king = false; // дамок нет
            if (i < 3) // первые 3 поля черные
            {
                information.field[i * 4 + j].checker_color = 2;
            }
            if (i > 4) // последние 3 - белые
            {
                information.field[i * 4 + j].checker_color = 1;
            }
            if (i == 3 || i == 4) // два поля в середине - пустые
            {
                information.field[i * 4 + j].checker_color = 0;
            }
            information.field[i * 4 + j].number = i + 1; // запись цифрового обозначения клетки
            information.field[i * 4 + j].letter = 'a' + j * 2 + (i % 2); // запись буквенного обозначения клетки
        }
    }
}

int finding_index(char cell[]) // определение индекса клетки в information.field[] по ее позиции
{
    int number = cell[1] - '1'; // перевод номера из char в int
    int count;
    int letter = (cell[0] - 'a' - number % 2) / 2; // определение координаты по горизонтали (буква)
    return 4 * number + letter; // нахождение индекса
}

bool Check_index(game_information& information, char position[], int index) // проверка совпадает ли клетка с найденным индексом. Используется для отсечения пустых клеток (а2, b1 и т.д.)
{
    if (index < 32 && information.field[index].letter == position[0] && information.field[index].number == position[1] - '0') // если индекс меньше 32 (т.к. клеток 32 и нумерация с 0)
    {                                                                                                                         // и номер и буква клетки совпадают с записанными в ячейке для этого индекса
        return true;
    }
    else
    {
        return false;
    }
}

bool Moving(game_information& information, char first[], char second[], int color) // перемешение шашки из клетки first в клетку second, возвращает true, если шашка стала дамкой
{
    int first_index = finding_index(first); // нахождение индекса первой клетки
    int second_index = finding_index(second); // нахождение индекса второй клетки
    information.field[second_index].king = information.field[first_index].king; // перемещение шашки из первой клетки во вторую
    information.field[second_index].checker_color = information.field[first_index].checker_color;
    information.field[first_index].king = false;
    information.field[first_index].checker_color = 0;
    if ((second[1] == '1' && color == 1 || second[1] == '8' && color == 2) && information.field[second_index].king == false) // если не дамка попала на последний / первый ряд
    {
        information.field[second_index].king = true;
        return true; // проверка используется для рассчета хода компьютера
    }
    else
    {
        return false;
    }
}

void Game_over(game_information& information, int color) // завершение игры, с победой цвета color, 0 - ничья
{
    information.game_over = true; // информация в поле, что игра окончена
    if (color == 1)
    {
        printf("Выиграли белые\n");
    }
    else
    {
        if (color == 2)
        {
            printf("Выиграли черные\n");
        }
        else
        {
            printf("Ничья\n");
        }
    }
}

bool Eating(game_information& information, char first[], char second[], char food[], int hunter_color) // перемещение из клетки first[] в second[] с съедением шашки из клетки food[], возвращает true если в результате шашка стала дамкой
{
    bool result;
    result = Moving(information, first, second, hunter_color); // стала ли шашка дамкой
    int index = finding_index(food); // определяем индекс съеденной шашки
    information.field[index].king = false; // убираем съеденную шашку
    information.field[index].checker_color = 0;
    first[0] = second[0]; // записывает текущее положение шашки (нужно для проверки возможности продолжить ход)
    first[1] = second[1];
    return result;
}

void PrintField(game_information& information, can_turn turns[], char current_position[], int cursor_position) // вывод поля игры, передается информаци о поле игры, 
{
    int counter = 0; // счетчик для отображения клеток, в которые можно совершить ход
    char position[3]; // позиция текущей клетки
    for (int i = 7; i >= 0; i--) // поле заполняется сверху, всего 8 клеток по вертикали
    {
        if (i == 7) // вывод верхней границы
        {
            printf("  -----------------------------------------");
            printf("     Список команд:\n");
        }
        for (int j = 0; j < 8; j++) // заполнение первой строчки, всего 8 клеток
        {
            if (j == 0) // начальный отступ от левой границы консоли
            {
                printf("  ");
            }
            if (i % 2 == j % 2) // если клетка доступна для шашек
            {
                if (j == 0) // вывод боковых границ
                {
                    printf("|");
                }
                if (cursor_position == (i + 1) * 10 + j + 1) // если клетка выделена
                {
                    printf("====");
                }
                else
                {
                    printf("    ");
                }
                if (j == 7)
                {
                    printf("|");
                }
            }
            else
            {
                if (cursor_position == (i + 1) * 10 + j + 1)
                {
                    printf("|====|");
                }
                else
                {
                    printf("|0000|"); // если клетка не участвует в игре
                }
            }
        }
        if (i == 7) // вывод элементов правой панели с командами если строка первая
        {
            if (cursor_position != 89)
            {
                printf("     Сохранить игру");
            }
            else
            {
                printf("     [Сохранить игру]");
            }
        }
        if (i == 6) // вовод последней строки панели с командами для 2 строки
        {
            if (cursor_position != 49)
            {
                printf("     Выход в главное меню");
            }
            else
            {
                printf("     [Выход в главное меню]");
            }
        }
        printf("\n"); // переход ко 2-ой строчке
        for (int j = 0; j < 8; j++) // заполнение второй строчки, всего 8 клеток
        {
            if (j == 0) // если клетка первая выводим номер строки
            {
                printf("%d ", i + 1);
            }
            if (i % 2 == j % 2) // если клетка участвует в игре
            {
                if (j == 0) // выводим левую границу для первой клетки
                {
                    printf("|");
                }
                if (current_position[0] == 'a' + j && current_position[1] == '1' + i && information.field[i * 4 + j / 2].king == true) // если клетка в этой позиции выделена и является дамкой
                {
                    printf("[");
                }
                else
                {
                    if (cursor_position == (i + 1) * 10 + j + 1)
                    {
                        printf("=");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
                if (information.field[i * 4 + j / 2].king == true) // если в этой клетке расположена дамка
                {
                    printf("$");
                }
                else
                {
                    if (current_position[0] == 'a' + j && current_position[1] == '1' + i) // если шашка в этой позиции выделена
                    {
                        printf("[");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
                if (information.field[i * 4 + j / 2].checker_color == 1) // в клетке белая шашка
                {
                    printf("1");
                }
                else
                {
                    if (information.field[i * 4 + j / 2].checker_color == 2) // черная
                    {
                        printf("2");
                    }
                    else // в клетке нет шашки
                    {
                        position[0] = 'a' + j;
                        position[1] = '0' + i + 1;
                        if (Be_in_turns(turns, position, counter)) // если в эту клетку можно сходить выделенной шашкой, counter - счетчик сколько клеток из списка ходов выделено (в списке они идут слева направо и сверху вниз)
                        {
                            printf("X");
                            counter++;
                        }
                        else
                        {
                            printf(" ");
                        }
                    }
                }
                if (current_position[0] == 'a' + j && current_position[1] == '1' + i) // если клетка выделена
                {
                    printf("]");
                }
                else
                {
                    if (cursor_position == (i + 1) * 10 + j + 1)
                    {
                        printf("=");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
                if (j == 7) // вывод правой границы поля
                {
                    printf("|");
                }
            }
            else
            {
                if (cursor_position == (i + 1) * 10 + j + 1)
                {
                    printf("|=00=|");
                }
                else
                {
                    printf("|0000|");
                }
            }
        }
        if (i == 7)
        {
            if (cursor_position != 79)
            {
                printf("     Загрузить игру");
            }
            else
            {
                printf("     [Загрузить игру]");
            }
        }
        printf("\n");
        for (int j = 0; j < 8; j++)  // третья строка
        {
            if (j == 0)
            {
                printf("  ");
            }
            if (i % 2 == j % 2)
            {
                if (j == 0)
                {
                    printf("|");
                }
                if (cursor_position == (i + 1) * 10 + j + 1) // если клетка выделена
                {
                    printf("====");
                }
                else
                {
                    printf("    ");
                }
                if (j == 7)
                {
                    printf("|");
                }
            }
            else
            {
                if (cursor_position == (i + 1) * 10 + j + 1)
                {
                    printf("|====|");
                }
                else
                {
                    printf("|0000|"); // если клетка не участвует в игре
                }
            }
        }
        if (i == 7)
        {
            if (cursor_position != 69)
            {
                printf("     Предложить ничью");
            }
            else
            {
                printf("     [Предложить ничью]");
            }
        }
        printf("\n");
        printf("  -----------------------------------------"); // вывод нижней границы клеток
        if (i == 7)
        {
            if (cursor_position != 59)
            {
                printf("     Сдаться");
            }
            else
            {
                printf("     [Сдаться]");
            }
        }
        printf("\n");
    }
    printf("     a    b    c    d    e    f    g    h  \n"); // печать буквенных обозначений клеток под полем игры
}

void CanMove(game_information& information, can_turn turns[], char first[], int color, bool eating) // проверяет, есть ли у шашки ходы, получает массив ходов, позицию шашки, цвет и возможность съедения
{
    bool eating_now_1 = false, eating_now_2 = false; // возможность есть в направлении вправо, влево при движении вверх или вниз
    int count = 0; // количество найденных ходов
    int index = finding_index(first); // индекс шашки
    char first_position[3]; // первая рассматриваемая конечная позиция
    char second_position[3]; // вторая
    int first_index; // индекс первой позиции
    int second_index; // второй
    if (information.field[index].king) // если дамка
    {
        first_position[0] = first[0] - 1; // идем влево вверх и вправо вверх
        first_position[1] = first[1] + 1;
        second_position[0] = first[0] + 1;
        second_position[1] = first[1] + 1;
        first_index = finding_index(first_position); // определяем индексы
        second_index = finding_index(second_position);
        while ((first_position[0] >= 'a' && first_position[1] <= '8' && information.field[first_index].checker_color != color) || (second_position[0] <= 'h' && second_position[1] <= '8' && information.field[second_index].checker_color != color)) // пока не вышли за поле игры и клетки не содержат шашки того же цвета
        {
            if (second_position[0] <= 'h' && second_position[1] <= '8' && information.field[second_index].checker_color != color) // если вторая позиция не вышла за поле игры и не содержит шашку того же цвета
            {
                if (information.field[second_index].checker_color == 0 && eating == eating_now_2) // пока клетка пуста и съедение совпадает с текущим
                {
                    for (int i = count; i > 0; i--) // записываем найденную позицию в массиве ходов
                    {
                        turns[i].number = turns[i - 1].number;
                        turns[i].letter = turns[i - 1].letter;
                    }
                    turns[0].number = information.field[second_index].number; // записываем в нулевой элемент найденное значение
                    turns[0].letter = information.field[second_index].letter;
                    count++; // увеличиваем счетчик
                }
                else
                {
                    if (eating == true && eating_now_2 == true) // в клетке шашка противника, которую нельзя съесть т.к. на этом ходе съедение было раньше
                    {
                        break;
                    }
                    if (information.field[second_index].checker_color == color % 2 + 1 && eating_now_2 == false) // обнаружена шашка противника и съедения еще не было
                    {
                        eating_now_2 = true;
                    }
                }
                second_position[0] = second_position[0] + 1; // увеличение позиции
                second_position[1] = second_position[1] + 1;
                second_index = finding_index(second_position); // пересчет индекса
            }
            if (first_position[0] >= 'a' && first_position[1] <= '8' && information.field[first_index].checker_color != color) // аналогично как для второй позиции
            {
                if (information.field[first_index].checker_color == 0 && eating == eating_now_1)
                {
                    for (int i = count; i > 0; i--)
                    {
                        turns[i].number = turns[i - 1].number;
                        turns[i].letter = turns[i - 1].letter;
                    }
                    turns[0].number = information.field[first_index].number;
                    turns[0].letter = information.field[first_index].letter;
                    count++;
                }
                else
                {
                    if (eating == true && eating_now_1 == true)
                    {
                        break;
                    }
                    if (information.field[first_index].checker_color == color % 2 + 1 && eating_now_1 == false)
                    {
                        eating_now_1 = true;
                    }
                }
                first_position[0] = first_position[0] - 1;
                first_position[1] = first_position[1] + 1;
                first_index = finding_index(first_position);
            }
        }
        first_position[0] = first[0] - 1; // идем вниз, аналогично для направления вверх
        first_position[1] = first[1] - 1;
        second_position[0] = first[0] + 1;
        second_position[1] = first[1] - 1;
        first_index = finding_index(first_position);
        second_index = finding_index(second_position);
        eating_now_2 = false;
        eating_now_1 = false;
        while ((first_position[0] >= 'a' && first_position[1] > '0' && information.field[first_index].checker_color != color) || (second_position[0] <= 'h' && second_position[1] > '0' && information.field[second_index].checker_color != color))
        {
            if (first_position[0] >= 'a' && first_position[1] <= '8' && information.field[first_index].checker_color != color) // сначала проверяем первую позицию, построение поля идет слева направо
            {
                if (information.field[first_index].checker_color == 0 && eating == eating_now_1)
                {
                    turns[count].number = information.field[first_index].number;
                    turns[count].letter = information.field[first_index].letter;
                    count++;
                }
                else
                {
                    if (eating == true && eating_now_1 == true)
                    {
                        break;
                    }
                    if (information.field[first_index].checker_color == color % 2 + 1 && eating_now_1 == false)
                    {
                        eating_now_1 = true;
                    }
                }
                first_position[0] = first_position[0] - 1;
                first_position[1] = first_position[1] - 1;
                first_index = finding_index(first_position);
            }
            if (second_position[0] <= 'h' && second_position[1] > '0' && information.field[second_index].checker_color != color)
            {
                if (information.field[second_index].checker_color == 0 && eating == eating_now_2) // вторая позиция записывается в конец массива
                {
                    turns[count].number = information.field[second_index].number;
                    turns[count].letter = information.field[second_index].letter;
                    count++;
                }
                else
                {
                    if (eating == true && eating_now_2 == true)
                    {
                        break;
                    }
                    if (information.field[second_index].checker_color == color % 2 + 1 && eating_now_2 == false)
                    {
                        eating_now_2 = true;
                    }
                }
                second_position[0] = second_position[0] + 1;
                second_position[1] = second_position[1] - 1;
                second_index = finding_index(second_position);
            }
        }
    }
    else
    {
        char eat_position[3]; // позиция съедаемой шашки
        first_position[0] = first[0] - 1; // проверяем клетки вверх влево и вправо
        first_position[1] = first[1] + 1;
        first_index = finding_index(first_position);
        second_position[0] = first[0] + 1;
        second_position[1] = first[1] + 1;
        second_index = finding_index(second_position);
        if (first[0] > 'a' && first[1] < '8' && information.field[first_index].checker_color == 0 && color == 2 && eating == false) // если первая позиция не за пределами поля, свободна и нет съедения
        {
            turns[count].letter = first_position[0]; // записываем ход
            turns[count].number = first_position[1] - '0';
            count++;
        }
        else
        {
            if (first_position[0] > 'a' && first_position[1] < '8' && eating) // если съедение
            {
                eat_position[0] = first_position[0] - 1;
                eat_position[1] = first_position[1] + 1;
                if (information.field[first_index].checker_color == color % 2 + 1 && information.field[finding_index(eat_position)].checker_color == 0) // определяем, возможно ли съедение, условие на то, что за шашкой противника есть пустая клетка
                {
                    turns[count].letter = eat_position[0]; // записываем ход
                    turns[count].number = eat_position[1] - '0';
                    count++;
                }
            }
        }
        if (first[0] < 'h' && first[1] < '8' && information.field[second_index].checker_color == 0 && color == 2 && eating == false) // аналогично как для первого
        {
            turns[count].letter = second_position[0];
            turns[count].number = second_position[1] - '0';
            count++;
        }
        else
        {
            if (second_position[0] < 'h' && second_position[1] < '8' && eating)
            { 
                eat_position[0] = second_position[0] + 1;
                eat_position[1] = second_position[1] + 1;
                if (information.field[second_index].checker_color == color % 2 + 1 && information.field[finding_index(eat_position)].checker_color == 0) // записываем ход
                {
                    turns[count].letter = eat_position[0]; 
                    turns[count].number = eat_position[1] - '0';
                    count++;
                }
            }
        }
        first_position[0] = first[0] - 1; // аналогично рассматриваем нижние клетки
        first_position[1] = first[1] - 1;
        second_position[0] = first[0] + 1;
        second_position[1] = first[1] - 1;
        first_index = finding_index(first_position);
        second_index = finding_index(second_position);
        if (first[0] > 'a' && first[1] > '1' && information.field[first_index].checker_color == 0 && color == 1 && eating == false)
        {
            turns[count].letter = first_position[0];
            turns[count].number = first_position[1] - '0';
            count++;
        }
        else
        {
            if (first_position[0] > 'a' && first_position[1] > '1' && eating)
            {
                eat_position[0] = first_position[0] - 1;
                eat_position[1] = first_position[1] - 1;
                if (information.field[first_index].checker_color == color % 2 + 1 && information.field[finding_index(eat_position)].checker_color == 0)
                {
                    turns[count].letter = eat_position[0];
                    turns[count].number = eat_position[1] - '0';
                    count++;
                }
            }
        }
        if (first[0] < 'h' && first[1] > '1' && information.field[second_index].checker_color == 0 && color == 1 && eating == false)
        {
            turns[count].letter = second_position[0];
            turns[count].number = second_position[1] - '0';
            count++;
        }
        else
        {
            if (second_position[0] < 'h' && second_position[1] > '1' && eating)
            {
                eat_position[0] = second_position[0] + 1;
                eat_position[1] = second_position[1] - 1;
                if (information.field[second_index].checker_color == color % 2 + 1 && information.field[finding_index(eat_position)].checker_color == 0)
                {
                    turns[count].letter = eat_position[0];
                    turns[count].number = eat_position[1] - '0';
                    count++;
                }
            }
        }
    }
} 

bool CanEating(game_information& information, char first[], int color) // проверяет, может ли шашка съесть шашку противника (одну или несколько), передается позиция шашки и ее цвет
{
    int first_index = finding_index(first); // индекс для шашки, которая ходит
    char position[3]; // позиция проверяемой клетки
    int index; // индекс проверяемой клетки
    char next_position[3]; // позиция следующих за проверяемой клеток
    position[0] = first[0] - 1; // идем влево и ввверх от исходной клетки
    position[1] = first[1] + 1;
    index = finding_index(position);
    if (information.field[first_index].king == 1) // если шашка - дамка
    {
        while (position[0] > 'a' && position[1] < '8' && information.field[index].checker_color != color) // пока не кончилось поле или не встречена собственная шашка
        {
            if (information.field[index].checker_color == color % 2 + 1) // если в клетке - шашка противника
            {
                next_position[0] = position[0] - 1;
                next_position[1] = position[1] + 1;
                if (information.field[finding_index(next_position)].checker_color == 0) // если следующая за ней клетка пуста - возвращаем true
                {
                    return true;
                }
                else
                {
                    break;
                }
            }
            position[0] = position[0] - 1; // рассматриваем следующую клетку
            position[1] = position[1] + 1;
            index = finding_index(position);
        }
        position[0] = first[0] + 1; // далее аналогично
        position[1] = first[1] + 1;
        index = finding_index(position);
        while (position[0] < 'h' && position[1] < '8' && information.field[index].checker_color != color)
        {
            if (information.field[index].checker_color == color % 2 + 1)
            {
                next_position[0] = position[0] + 1;
                next_position[1] = position[1] + 1;
                if (information.field[finding_index(next_position)].checker_color == 0)
                {
                    return true;
                }
                else
                {
                    break;
                }
            }
            position[0] = position[0] + 1;
            position[1] = position[1] + 1;
            index = finding_index(position);
        }
        position[0] = first[0] - 1;
        position[1] = first[1] - 1;
        index = finding_index(position);
        while (position[0] > 'a' && position[1] > '1' && information.field[index].checker_color != color)
        {
            if (information.field[index].checker_color == color % 2 + 1)
            {
                next_position[0] = position[0] - 1;
                next_position[1] = position[1] - 1;
                if (information.field[finding_index(next_position)].checker_color == 0)
                {
                    return true;
                }
                else
                {
                    break;
                }
            }
            position[0] = position[0] - 1;
            position[1] = position[1] - 1;
            index = finding_index(position);
        }
        position[0] = first[0] + 1;
        position[1] = first[1] - 1;
        index = finding_index(position);
        while (position[0] < 'h' && position[1] > '1' && information.field[index].checker_color != color)
        {
            if (information.field[index].checker_color == color % 2 + 1)
            {
                next_position[0] = position[0] + 1;
                next_position[1] = position[1] - 1;
                if (information.field[finding_index(next_position)].checker_color == 0)
                {
                    return true;
                }
                else
                {
                    break;
                }
            }
            position[0] = position[0] + 1;
            position[1] = position[1] - 1;
            index = finding_index(position);
        }
    }
    else // если не дамка
    {
        if (position[0] > 'a' && position[1] < '8' && information.field[index].checker_color == color % 2 + 1) // если в клетке чужая шашка и съедение возможно (клетка не с краю поля)
        {
            next_position[0] = position[0] - 1;
            next_position[1] = position[1] + 1;
            if (information.field[finding_index(next_position)].checker_color == 0) // если следующая клетка пуста
            {
                return true;
            }
        }
        position[0] = first[0] + 1; // рассматриваем следующую клетки
        position[1] = first[1] + 1;
        index = finding_index(position); // далее аналогично
        if (position[0] < 'h' && position[1] < '8' && information.field[index].checker_color == color % 2 + 1)
        {
            next_position[0] = position[0] + 1;
            next_position[1] = position[1] + 1;
            if (information.field[finding_index(next_position)].checker_color == 0)
            {
                return true;
            }
        }
        position[0] = first[0] - 1;
        position[1] = first[1] - 1;
        index = finding_index(position);
        if (position[0] > 'a' && position[1] > '1' && information.field[index].checker_color == color % 2 + 1)
        {
            next_position[0] = position[0] - 1;
            next_position[1] = position[1] - 1;
            if (information.field[finding_index(next_position)].checker_color == 0)
            {
                return true;
            }
        }
        position[0] = first[0] + 1;
        position[1] = first[1] - 1;
        index = finding_index(position);
        if (position[0] < 'h' && position[1] > '1' && information.field[index].checker_color == color % 2 + 1)
        {
            next_position[0] = position[0] + 1;
            next_position[1] = position[1] - 1;
            if (information.field[finding_index(next_position)].checker_color == 0)
            {
                return true;
            }
        }
    }
    return false;
}

bool CanEat(game_information& information, can_turn hunters[], int color) // перебирает все шашки цвета color и определяет могут ли они съесть шашку противника, передается список для заполнения его позициями шашек, могущих съесть шашку противника
{
    char position[3]; // позиция проверяемой шашки
    int count = 0; // количество найденных шашек
    for (int i = 0; i < 32; i++) // перебор всех клеток
    {
        if (information.field[i].checker_color == color) // если цвет шашки в клетке нужный
        {
            position[0] = information.field[i].letter;
            position[1] = '0' + information.field[i].number;
            if (CanEating(information, position, color)) // если шашка может есть записываем ее в массив
            {
                hunters[count].letter = information.field[i].letter;
                hunters[count].number = information.field[i].number;
                count++;
            }
        }
    }
    if (hunters[0].number == '0') // если не записано ни одной шашки
    {
        return false;
    }
    else
    {
        return true;
    }
}

void Clear_can_turn_list(can_turn turns[], int size) // очищает массив ходов размера size
{
    for (int i = 0; i < size; i++)
    {
        turns[i].letter = '0';
        turns[i].number = '0';
    }
}

void Checking_food_position(game_information& information, char first[], char second[], char food[], int color) // определяет позицию шашки, которая будет съедена при ходе из first в second
{
    char position[3]; 
    if (first[0] < second[0] && first[1] < second[1]) // если ход влево вниз
    {
        for (int i = 1; second[1] - i > first[1]; i++) // проверяем все клетки между first и second
        {
            position[0] = second[0] - i;
            position[1] = second[1] - i;
            if (information.field[finding_index(position)].checker_color == color % 2 + 1) // если в проверяемой клетке шашка противника
            {
                food[0] = position[0]; // записывается позиция найденной шашки
                food[1] = position[1];
                return;
            }
        }
    }
    if (first[0] > second[0] && first[1] < second[1]) // если ход вправо вниз
    {
        for (int i = 1; second[1] - i > first[1]; i++) 
        {
            position[0] = second[0] + i;
            position[1] = second[1] - i;
            if (information.field[finding_index(position)].checker_color == color % 2 + 1)
            {
                food[0] = position[0];
                food[1] = position[1];
                return;
            }
        }
    }
    if (first[0] > second[0] && first[1] > second[1]) // если ход вправо вверх
    {
        for (int i = 1; second[1] + i < first[1]; i++)
        {
            position[0] = second[0] + i;
            position[1] = second[1] + i;
            if (information.field[finding_index(position)].checker_color == color % 2 + 1)
            {
                food[0] = position[0];
                food[1] = position[1];
                return;
            }
        }
    }
    if (first[0] < second[0] && first[1] > second[1]) // если ход влево вверх
    {
        for (int i = 1; second[1] + i < first[1]; i++)
        {
            position[0] = second[0] - i;
            position[1] = second[1] + i;
            if (information.field[finding_index(position)].checker_color == color % 2 + 1)
            {
                food[0] = position[0];
                food[1] = position[1];
                return;
            }
        }
    }
}

bool Haveturn(game_information& information, int color) // проверяет есть ли для игрока цвета color ходы (нет, если все шашки съедены / заблокированы)
{
    can_turn turns[15], hunters[12]; // массивы для найденных ходов
    Clear_can_turn_list(turns, 15); // очищаем массивы
    Clear_can_turn_list(hunters, 12);
    if (CanEat(information, hunters, color)) // проверяем, может ли какая-то шашка есть
    {
        return true;
    }
    char position[3]; // позиция текущей шашки
    for (int i = 0; i < 32; i++) // перебор всех клеток
    {
        if (information.field[i].checker_color == color) // если клетка содержит шашку нужного цвета
        {
            position[0] = information.field[i].letter;
            position[1] = '0' + information.field[i].number;
            CanMove(information, turns, position, color, false); // проверка, есть ли ходы для текущей шашки
            if (turns[0].letter != '0') // если для шашки есть ходы
            {
                return true;
            }
        }
    }
    return false;
}

int Hotseat_game(game_information& information) // игра человека с человеком
{
    char buffer[1000]; // буффер для считывания ввода
    int position = 0, code = 0, error = 0;// позиция выделенной клетки, код стрелки, код ошибки
    char first[3], second[3], food[3]; //позиции начального, конечного и положения съедаемой шашки
    bool currect_turn; // корректно ли совершен ход
    bool eat = false, eating = false, draw = false; // съедение в начале хода, возможность съесть еще одну шашку, предложение ничьей
    int index; // индекс, выбранной шашки
    bool end; // конец хода
    can_turn turns[15]; // массив позиций, куда можно сделать ход выбранной шашкой
    can_turn hunters[12]; // массив шашек, которые могут есть
    while (information.game_over == false) // пока игра не закончилась
    {
        end = false; // ход не закончен
        if (information.player == 1)
        {
            position = 88;
        }
        else
        {
            position = 11;
        }
        first[0] = '0';
        first[1] = '0';
        Clear_can_turn_list(turns, 15);
        printf("Ход %d игрока\n", information.player);
        if (draw) // если другой игрок предложил сдаться
        {
            draw = false; // предложение сдачи неактивно
            printf("Противник предлагает ничью. Вы согласны? [Y/N]\n");
            int position_three = 1, code_three = 0;
            do // выбор цвета шашек
            {
                system("cls");
                PrintField(information, turns, first, 0); // вывод поля, пока ничего не выделено
                printf("Противник предлагает ничью. Вы согласны?\n");
                if (position_three == 1)
                {
                    printf("[Да]\n");
                    printf("Нет\n");
                }
                else
                {
                    printf("Да\n");
                    printf("[Нет]\n");
                }
                code_three = _getch();
                if (code_three == 224)
                {
                    code_three = _getch();
                    if (code_three == 0x48)
                    {
                        position_three = 1;
                    }
                    if (code_three == 0x50)
                    {
                        position_three = 2;
                    }
                }
            } while (code_three != 13);
            if (position_three == 1) // если согласен
            {
                Game_over(information, 0); // ничья
            }
            continue;
        }
        while (end == false) // пока ход не завершен (выбрана неверная клетка)
        {
            Clear_can_turn_list(hunters, 12); // очистка массивов
            Clear_can_turn_list(turns, 15);
            eat = false; // изначально шашка есть не может
            end = true; // ход завершится, если введены корректные данные
            eating = CanEat(information, hunters, information.player); // проверка, может ли игрок съесть шашку
            system("cls");
            PrintField(information, turns, first, position);
            if (error == 1)
            {
                printf("Такой клетки не существует\n");
            }
            if (error == 2)
            {
                printf("Необходимо съесть шашку противника\n");
            }
            if (error == 3)
            {
                printf("Эта шашка не имеет ходов\n");
            }
            if (error == 4)
            {
                printf("В выбранной клетке нет вашей шашки\n");
            }
           /* if (error == 5)
            {
                printf("Игра загружена\n");
            }*/
            error = 0;
            code = _getch();
            if (code == 224)
            {
                code = _getch();
                if (code == 0x48)
                {
                    position += 10;
                }
                if (code == 0x50)
                {
                    position -= 10;
                }
                if (code == 0x4B)
                {
                    position -= 1;
                }
                if (code == 0x4D && position % 10 < 9)
                {
                    if (position % 10 == 8)
                    {
                        position = 89;
                    }
                    else
                    {
                        position += 1;
                    }
                }
            }
            if (position / 10 == 9)
            {
                position -= 10;
            }
            if (position / 10 == 0)
            {
                position += 10;
            }
            if (position % 10 == 0)
            {
                position++;
            }
            if (position % 10 == 9 && position / 10 < 4)
            {
                position == 49;
            }
            if (position % 10 == 9 && position / 10 > 8)
            {
                position == 89;
            }
            if (code == 13)
            {
                if (position == 69) // если команда - предложить ничью
                {
                    draw = true; // противнику будет предложено сдаться в его ход
                    end = false; // ход не закончен
                    continue;
                }
                if (position == 59) // если команда сдаться
                {
                    if (information.player == 1)
                    {
                        printf("Белые сдались\n");
                    }
                    else
                    {
                        printf("Чёрные сдались\n");
                    }
                    Game_over(information, information.player % 2 + 1); // конец игры
                    continue;
                }
                if (position == 89) // если команда сохранить
                {
                    Save(information, 1); // сохранение
                    end = false; // ход не закончен
                    //PrintField(information, turns, first); // вывод поля
                    continue;
                }
                if (position == 79) // команда загрузить
                {
                    int code = Load(information); // определение типа загруженной игры
                    if (code != 0) // игра загружена
                    {
                        //error = 5;
                        printf("Игра загружена\n");
                        return code; // возвращает тип загруженной игры
                    }
                    end = false; // ход не закончен
                    continue;
                }
                if (position == 49) // выход в главное меню
                {
                    return 0;
                }
                first[0] = position % 10 - 1 + 'a'; // запись позиции выбранной клетки
                first[1] = position / 10 + '0';
                index = finding_index(first); // нахождение позиции клетки
                if (!Check_index(information, first, index)) // если клетка выбрана неправильно (не существует или не активна)
                {
                    error = 1;
                    end = false;
                    continue;
                }
                if (eating) // если можно съесть шашку
                {
                    if (Be_in_turns(hunters, first) == false) // если выбранная шашка не может съесть
                    {
                        error = 2;
                        first[0] = '0';
                        first[1] = '0';
                        end = false; // ход не закончен
                        continue;
                    }
                }
                else
                {
                    CanMove(information, turns, first, information.player, false); // определяет куда можно сходить шашкой
                    if (turns[0].letter == '0') // если никуда
                    {
                        error = 3;
                        first[0] = '0';
                        first[1] = '0';
                        end = false;
                        continue;
                    }
                }
                if (information.field[index].checker_color == information.player) // в выбранной клетке есть шашка игрока
                {
                    do // совершение хода
                    {
                        Clear_can_turn_list(turns, 15); // очистка массива ходов
                        if (eat == false)
                        {
                            eat = eating;
                        }
                        CanMove(information, turns, first, information.player, eat); // нахождение позиций, куда можно сходить, выбранной шашкой
                        do
                        {
                            system("cls");
                            PrintField(information, turns, first, position); // вывод поля
                            if (error == 1)
                            {
                                printf("Сохранить игру во время хода нельзя!\n");
                            }
                            /*if (error == 2)
                            {
                                printf("Игра загружена\n");
                            }*/
                            if (error == 3)
                            {
                                printf("Такой клетки не существует\n");
                            }
                            if (error == 4)
                            {
                                printf("Ход невозможен\n");
                            }
                            if (error == 5)
                            {
                                printf("Продолжение хода %d игрока\n", information.player);
                            }
                            error = 0;
                            currect_turn = true; // ход считаем правильным
                            code = _getch();
                            if (code == 224)
                            {
                                code = _getch();
                                if (code == 0x48)
                                {
                                    position += 10;
                                }
                                if (code == 0x50)
                                {
                                    position -= 10;
                                }
                                if (code == 0x4B)
                                {
                                    position -= 1;
                                }
                                if (code == 0x4D && position % 10 < 9)
                                {
                                    if (position % 10 == 8)
                                    {
                                        position = 89;
                                    }
                                    else
                                    {
                                        position += 1;
                                    }
                                }
                            }
                            if (position / 10 == 9)
                            {
                                position -= 10;
                            }
                            if (position / 10 == 0)
                            {
                                position += 10;
                            }
                            if (position % 10 == 0)
                            {
                                position++;
                            }
                            if (position % 10 == 9 && position / 10 < 4)
                            {
                                position == 49;
                            }
                            if (position % 10 == 9 && position / 10 > 8)
                            {
                                position == 89;
                            }
                            if (code == 13)
                            {
                                if (position == 69) // команда аналогична такой же выше
                                {
                                    draw = true;
                                    currect_turn = false;
                                    continue;
                                }
                                if (position == 59) // команда аналогична такой же выше
                                {
                                    if (information.player == 1)
                                    {
                                        printf("Белые сдались\n");
                                    }
                                    else
                                    {
                                        printf("Чёрные сдались\n");
                                    }
                                    Game_over(information, information.player % 2 + 1);
                                    continue;
                                }
                                if (position == 89) // сохранить во время хода нельзя
                                {
                                    error = 1;
                                    currect_turn = false;
                                    continue;
                                }
                                if (position == 79) // команда аналогична такой же выше
                                {
                                    int code = Load(information);
                                    if (code != 0)
                                    {
                                        //error = 2;
                                        printf("Игра загружена\n");
                                        return code;
                                    }
                                    currect_turn = false;
                                    continue;
                                }
                                if (position == 49) // команда аналогична такой же выше
                                {
                                    return 0;
                                }
                                second[0] = position % 10 - 1 + 'a';
                                second[1] = position / 10 + '0';
                                if (!Check_index(information, second, finding_index(second))) // если выбрана неверная клетка
                                {
                                    currect_turn = false;
                                    error = 3;
                                }
                                else
                                {
                                    if (Be_in_turns(turns, second) == false) // если ход невозможен
                                    {
                                        currect_turn = false;
                                        error = 4;
                                    }
                                }
                            }
                            else
                            {
                                currect_turn = false;
                            }
                        } while (currect_turn == false); // пока не совершен верный ход
                        if (eat) // если происходит съедение
                        {
                            Checking_food_position(information, first, second, food, information.player); // определение позиции съедаемой шашки
                            Eating(information, first, second, food, information.player); // съедение шашки
                            eat = CanEating(information, first, information.player); // проверка, может ли выбранная шашка съесть еще шашки
                            end = true; // ход совершен
                            if (eat) // если после хода текущая шашка может съесть еще одну, то ход продолжается
                            {
                                error = 5;
                            }
                        }
                        else
                        {
                            Moving(information, first, second, information.player); // перемещение шашки
                            end = true;
                        }
                    } while (eat); // если после хода текущая шашка может съесть еще одну, то ход продолжается
                    if (Haveturn(information, information.player % 2 + 1) == false) // если ходов у противника нет
                    {
                        first[0] = '0';
                        first[1] = '0';
                        Clear_can_turn_list(turns, 15);
                        system("cls");
                        PrintField(information, turns, first, 0);
                        Game_over(information, information.player); // конец игры
                    }
                }
                else // в клетке чужая шашка или шашки нет
                {
                    first[0] = '0';
                    first[1] = '0';
                    error = 4;
                    end = false;
                }
            }
            else
            {
                end = false;
            }
        }
        information.player = information.player % 2 + 1; // смена игрока
    }
    system("pause");
    return 0; // игра завершена, ничего не загружалось
}

void Clear_bot_turn_list(bot_turn& turn) // очистка массива ходов компьютера
{
    for (int i = 0; i < 13; i++)
    {
        turn.first[i].letter = '0';
        turn.first[i].number = '0';
        turn.second[i].letter = '0';
        turn.second[i].number = '0';
    }
}

int Bot_turn_analisis(game_information& information, bot_turn& best_turn, int step, int color);

void Bot_eat(game_information& information, char position[], bot_turn& best_turn, bot_turn& turn, int count, int step, int score, int& best_score, int color) // обработка съедения ботом шашки, передаеются клетка, для которой считается, лучший и текущий ход и счет, количество записанных ходов, цвет шашки
{
    int current_score = score; // запись счета, полученного ранее
    can_turn turns[15]; // массив ходов
    game_information copy; // копия информации о поле игры
    char second_position[3], food[3], first_position[3]; 
    Clear_can_turn_list(turns, 15); // очистка массива ходов
    if (CanEating(information, position, color)) // если шашка из клетки position может съесть шашку
    {
        CanMove(information, turns, position, color, true); // определение, куда шашка может переместиться
        for (int i = 0; turns[i].letter != '0'; i++) // и проверка для всех этих позиций
        {
            score = current_score; // откат счета до текущего состояния
            copy = information;
            first_position[0] = position[0];
            first_position[1] = position[1];
            second_position[0] = turns[i].letter;
            second_position[1] = turns[i].number + '0';
            Checking_food_position(copy, first_position, second_position, food, color);
            if (information.field[finding_index(food)].king) // если съедена дамка
            {
                score += 9;
                if (step % 2 == 1) // здесь и далее применение значения из настроек для хода компьютера
                {
                    score += settings.king * 3;
                }
            }
            else
            {
                score += 3; // съедена простая шашка
                if (step % 2 == 1)
                {
                    score += settings.attack;
                }
            }
            if (copy.field[finding_index(first_position)].king == 1) // проверка позиций для дамки
            {
                if (first_position[0] - 'a' == first_position[1] - '1') // дамка на главной диагонали
                {
                    if (second_position[0] - 'a' != second_position[1] - '1') // дамка покинула главную диагональ
                    {
                        score -= 2;
                        if (step % 2 == 1)
                        {
                            score -= settings.position;
                        }
                    }
                }
                else
                {
                    if (second_position[0] - 'a' == second_position[1] - '1') // дамка встала на главную диагональ
                    {
                        score += 2;
                        if (step % 2 == 1)
                        {
                            score += settings.position;
                        }
                    }
                }
            }
            else // проверка позиций для не дамки
            {
                if (color == 1 && finding_index(first_position) >= 28 && finding_index(first_position) < 32) // белая шашка покинула строку 8
                {
                    score -= 1;
                    if (step % 2 == 1)
                    {
                        score -= settings.position;
                    }
                }
                if (color == 2 && finding_index(first_position) >= 0 && finding_index(first_position) < 4) // черная шашка покинула строку 1
                {
                    score -= 1;
                    if (step % 2 == 1)
                    {
                        score -= settings.position;
                    }
                }
                if (color == 1 && finding_index(first_position) < 28) // белая шашка вернулась на строку 8
                {
                    score += 1;
                    if (step % 2 == 1)
                    {
                        score += settings.position;
                    }
                }
                if (color == 2 && finding_index(first_position) >= 0 && finding_index(first_position) > 4) // черная шашка вернулась на строку 1
                {
                    score += 1;
                    if (step % 2 == 1)
                    {
                        score += settings.position;
                    }
                }
                if (color == 1 && finding_index(second_position) == 7) // белая шашка заняла позицию h2
                {
                    score += 2;
                    if (step % 2 == 1)
                    {
                        score += settings.position;
                    }
                }
                if (color == 2 && finding_index(second_position) == 24) // черная шашка заняла позицию a7
                {
                    score += 2;
                    if (step % 2 == 1)
                    {
                        score += settings.position;
                    }
                }
            }
            if (step == 1) // если текущая глубина проверки = 1
            {
                turn.first[count].letter = position[0]; // записываем ход
                turn.first[count].number = position[1] - '0';
                turn.second[count].letter = turns[i].letter;
                turn.second[count].number = turns[i].number;
                count++; // увеличиваем счетчик записанных ходов (съедений подряд)
            }
            if (Eating(copy, first_position, second_position, food, color)) // съедение и проверка стала ли шашка дамкой
            {
                score += 9;
                if (step % 2 == 1)
                {
                    score += settings.king;
                }
            }
            Bot_eat(copy, first_position, best_turn, turn, count, step, score, best_score, color); // проверка возможно ли продолжение хода
            if (step == 1) // откат на прошлый шаг
            {
                count--; // уменьшение счетчика
                turn.first[count].letter = '0'; // удаление хода
                turn.first[count].number = '0';
                turn.second[count].letter = '0';
                turn.second[count].number = '0';
            }
        }
    }
    else
    {
        copy = information;
        if (Haveturn(copy, color % 2 + 1) == false) // все шашки противника заблокированы или съедены
        {
            score += 30;
            best_score = score;
            if (step == 1) // если шаг 1 - запись ходов в лучшие
            {
                Clear_bot_turn_list(best_turn); // очистка массива лучших ходов
                best_turn = turn; // запись текущего хода в лучший
                best_turn.eat = true;
            }
        }
        if (step < settings.turns * 2) // пока текущая глубина проверки не больше максимальной
        {
            copy.player = copy.player % 2 + 1; // смена игрока
            score -= Bot_turn_analisis(copy, best_turn, step + 1, color % 2 + 1); // нахождение лучшего хода противника
        }
        if (score > best_score) // если текущий счет больше лучшего
        {
            if (step == 1) // если шаг 1 - запись ходов в лучшие
            {
                Clear_bot_turn_list(best_turn); // очистка массива лучших ходов
                best_turn = turn; // запись текущего хода в лучший
                best_turn.eat = true;
            }
            best_score = score; // перезапись лучшего счета
        }
        if (score == best_score && rand() % 2 == 1) // если лучший ход совпадает с текущим, с вероятностью 0,5 перепишем лучший ход
        {
            if (step == 1) // если шаг 1 - запись ходов в лучшие
            {
                Clear_bot_turn_list(best_turn); // очистка массива лучших ходов
                best_turn = turn; // запись текущего хода в лучший
                best_turn.eat = true;
            }
            best_score = score; // перезапись лучшего счета
        }
    }
}

int Bot_turn_analisis(game_information& information, bot_turn& best_turn, int step, int color) // расчет лучшего хода для компьютера, передается информация о поле, лучший ход, текущая глубина, цвет текущего игрока
{
    bot_turn turn; // текущий ход
    Clear_bot_turn_list(turn); // очистить текущий ход
    int score, best_score = -1000000; // лучший и текщий счет, изначально отрицательны
    can_turn hunters[12], turns[15]; // клетки, шашки в которых едят и клетки в которые может сходить выбранная шашка
    Clear_can_turn_list(hunters, 12); // очистка массивов
    Clear_can_turn_list(turns, 15);
    char position[3], second_position[3], first_position[3]; // текущая позиция
    game_information copy = information; // копия информации об игре
    if (CanEat(copy, hunters, color)) // если можно съесть шашку
    {
        if (step == 1) // если шаг первый (для глубины n шагов n * 2, по n для каждого игрока)
        {
            best_turn.eat = true; // пометка в информации о лучшем ходе со съедением шашки
        }
        for (int i = 0; hunters[i].letter != '0'; i++) // перебор всех шашек, которые могут есть
        {
            copy = information;
            position[0] = hunters[i].letter;
            position[1] = hunters[i].number + '0';
            Bot_eat(copy, position, best_turn, turn, 0, step, 0, best_score, copy.player); // обработка съедения
        }
    }
    else
    {
        if (step == 1) // если шаг первый (для глубины n шагов n * 2, по n для каждого игрока)
        {
            best_turn.eat = false; // пометка в информации о лучшем ходе без съедения шашки
        }
        for (int i = 0; i < 32; i++) // перебор всех шашек
        {
            if (information.field[i].checker_color == color) // если цвет шашки совпадает с текущим цветом
            {
                position[0] = information.field[i].letter;
                position[1] = '0' + information.field[i].number;
                Clear_can_turn_list(turns, 15); // очищаем массив 
                CanMove(information, turns, position, color, false); // находим возможные ходы
                for (int j = 0; turns[j].letter != '0'; j++) // совершение всех возможных ходов
                {
                    copy = information;
                    score = 0; // текущий счет 0
                    first_position[0] = position[0]; // нахождении позиций в начале и конце хода
                    first_position[1] = position[1];
                    second_position[0] = turns[j].letter;
                    second_position[1] = '0' + turns[j].number;
                    if (copy.field[finding_index(first_position)].king == 1) // увеличение счета от позиций см. выше
                    {
                        if (first_position[0] - 'a' == first_position[1] - '1')
                        {
                            if (second_position[0] - 'a' != second_position[1] - '1')
                            {
                                score -= 2;
                                if (step % 2 == 1)
                                {
                                    score -= settings.position;
                                }
                            }
                        }
                        if (first_position[0] - 'a' != first_position[1] - '1')
                        {
                            if (second_position[0] - 'a' == second_position[1] - '1')
                            {
                                score += 2;
                                if (step % 2 == 1)
                                {
                                    score += settings.position;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (color == 1 && finding_index(first_position) >= 28 && finding_index(first_position) < 32)
                        {
                            score -= 1;
                            if (step % 2 == 1)
                            {
                                score -= settings.position;
                            }
                        }
                        if (color == 2 && finding_index(first_position) >= 0 && finding_index(first_position) < 4)
                        {
                            score -= 1;
                            if (step % 2 == 1)
                            {
                                score -= settings.position;
                            }
                        }
                        if (color == 1 && finding_index(second_position) == 7)
                        {
                            score += 2;
                            if (step % 2 == 1)
                            {
                                score += settings.position;
                            }
                        }
                        if (color == 2 && finding_index(second_position) == 24)
                        {
                            score += 2;
                            if (step % 2 == 1)
                            {
                                score += settings.position;
                            }
                        }
                    }
                    if (step == 1) // запись хода
                    {
                        turn.first[0].letter = position[0];
                        turn.first[0].number = position[1] - '0';
                        turn.second[0].letter = second_position[0];
                        turn.second[0].number = second_position[1] - '0';
                    }
                    if (Moving(copy, first_position, second_position, color)) // если шашка стала дамкой
                    {
                        score += 9;
                        if (step % 2 == 1)
                        {
                            score += settings.king * 3;
                        }
                    }
                    if (Haveturn(copy, color % 2 + 1) == false) // все шашки противника заблокированы или кончились
                    {
                        score += 30;
                        if (step == 1)
                        {
                            best_score = score;
                            best_turn.first[0].letter = turn.first[0].letter;
                            best_turn.first[0].number = turn.first[0].number;
                            best_turn.second[0].letter = turn.second[0].letter;
                            best_turn.second[0].number = turn.second[0].number;
                        }
                        return score;
                    }
                    if (step < settings.turns * 2) // если шаг меньше глубины * 2
                    {
                        copy.player = copy.player % 2 + 1; // смена игрока
                        score -= Bot_turn_analisis(copy, best_turn, step + 1, color % 2 + 1); // рассчет лучшего хода для второго игрока
                    }
                    if (score > best_score) // определение лучшего хода / счета (подробно см. выше)
                    {
                        if (step == 1)
                        {
                            best_score = score;
                            best_turn.first[0].letter = turn.first[0].letter;
                            best_turn.first[0].number = turn.first[0].number;
                            best_turn.second[0].letter = turn.second[0].letter;
                            best_turn.second[0].number = turn.second[0].number;
                        }
                        else
                        {
                            best_score = score;
                        }
                    }
                    if (score == best_score && rand() % 2 == 1)
                    {
                        if (step == 1)
                        {
                            best_score = score;
                            best_turn.first[0].letter = turn.first[0].letter;
                            best_turn.first[0].number = turn.first[0].number;
                            best_turn.second[0].letter = turn.second[0].letter;
                            best_turn.second[0].number = turn.second[0].number;
                        }
                        else
                        {
                            best_score = score;
                        }
                    }
                }
            }
        }
    }
    if (best_score == -1000000) // если счет не менялся
    {
        best_score = 0;
    }
    return best_score /** ((settings.turns + 1) / 2)*/; // возвращает лучший найденный счет
}

bool bot_draw(game_information& information) // определяет принимать ли компьютеру ничью 
{
    int my = 0, enemy = 0; // количество шашек текущего игрока и противника
    bool king_1 = false, king_2 = false; // наличие дамки
    for (int i = 0; i < 32; i++) // перебирает все клетки
    {
        if (information.field[i].checker_color == information.player % 2 + 1) // считает шашки для обоих игроков, просто шашка = 1, дамка - 3
        {
            my += 1;
            if (information.field[i].king == true)
            {
                king_1 = true;
                my += 2;
            }
        }
        if (information.field[i].checker_color == information.player)
        {
            enemy += 1;

            if (information.field[i].king == true)
            {
                king_2 = true;
                enemy += 2;
            }
        }
    }
    if (enemy / my >= 2 || (enemy == my && enemy == 3 && king_2 == true && king_1 == true)) // если у компьютера в два раза меньше шашек или у обоих игроков по 1 дамке
    {
        return true;
    }
    else
    {
        return false;
    }
}

int Game_with_bot(game_information& information, bool loading) // игра против компьютера, передается информация о игре и было ли загружено сохранение
{
    srand(time(NULL)); // запуск таймера, используется в анализе хода для компьютера
    char buffer[1000]; // буффер для ввода
    int position = 0, code = 0, error = 0;// позиция выделенной клетки, код стрелки, код ошибки
    char first[3], second[3], food[3]; //позиции начального, конечного и положения съедаемой шашки
    bot_turn best_turn; // лучший ход для компьютера
    bool currect_turn; // корректно ли совершен ход
    bool eat = false, eating = false; // съедение в начале хода, возможность съесть еще одну шашку
    int index; // индекс, выбранной шашки
    bool end = false; // конец хода
    can_turn turns[15]; // массив позиций, куда можно сделать ход выбранной шашкой
    can_turn hunters[12]; // массив шашек, которые могут есть
    while (information.game_over == false) // пока игра не закончилась
    {
        Clear_bot_turn_list(best_turn); // очистка лучшего хода
        if (information.player == 2 && !loading) // если компьютер играет за белых и ход не первый после загрузки игры (если первый то компьютер в сохранении уже походил)
        {
            information.player = information.player % 2 + 1; // смена игрока для хода компьютера
            Bot_turn_analisis(information, best_turn, 1, information.player); // поиск лучшего хода
            if (best_turn.eat) // если лучший ход - съедение
            {
                for (int i = 0; best_turn.first[i].letter != '0'; i++) // последовательное совершение хода
                {
                    first[0] = best_turn.first[i].letter; // запись начальной и конечной позиций
                    first[1] = best_turn.first[i].number + '0';
                    second[0] = best_turn.second[i].letter;
                    second[1] = best_turn.second[i].number + '0';
                    Checking_food_position(information, first, second, food, information.player);
                    Eating(information, first, second, food, information.player); // съедение шашки
                }
            }
            else
            {
                first[0] = best_turn.first[0].letter; // запись начальной и конечной позиций
                first[1] = best_turn.first[0].number + '0';
                second[0] = best_turn.second[0].letter;
                second[1] = best_turn.second[0].number + '0';
                Moving(information, first, second, 1); // совершение хода
                first[0] = second[0]; // для выделения конечного положения шашки
                first[1] = second[1];
            }
            if (Haveturn(information, information.player % 2 + 1) == false) // если игрок проиграл
            {
                Clear_can_turn_list(turns, 15);
                system("cls");
                PrintField(information, turns, first, 0);
                Game_over(information, information.player); // конец игры
                continue;
            }
            information.player = information.player % 2 + 1; // смена игрока для хода игрока
        }
        if (loading)
        {
            loading = false; // действия, необходимые для загруженной игры выполнены
        }
        Clear_can_turn_list(hunters, 12); // обработка хода игрока аналогично для игры с человеком
        Clear_can_turn_list(turns, 15);
        end = false;
        if(information.player == 1)
        {
            position = 88;
        }
        else
        {
            position = 11;
        }
        while (end == false)
        {
            system("cls");
            PrintField(information, turns, first, position);
            if (error == 1)
            {
                printf("Такой клетки не существует\n");
            }
            if (error == 2)
            {
                printf("Необходимо съесть шашку противника\n");
            }
            if (error == 3)
            {
                printf("Эта шашка не имеет ходов\n");
            }
            if (error == 4)
            {
                printf("В выбранной клетке нет вашей шашки\n");
            }
            if (error == 5)
            {
                printf("Компьютер отказался\n");
            }
            error = 0;
            Clear_can_turn_list(hunters, 12);
            Clear_can_turn_list(turns, 15);
            eat = false;
            end = true;
            eating = CanEat(information, hunters, information.player);
            code = _getch();
            if (code == 224)
            {
                code = _getch();
                if (code == 0x48)
                {
                    position += 10;
                }
                if (code == 0x50)
                {
                    position -= 10;
                }
                if (code == 0x4B)
                {
                    position -= 1;
                }
                if (code == 0x4D && position % 10 < 9)
                {
                    if (position % 10 == 8)
                    {
                        position = 89;
                    }
                    else
                    {
                        position += 1;
                    }
                }
            }
            if (position / 10 == 9)
            {
                position -= 10;
            }
            if (position / 10 == 0)
            {
                position += 10;
            }
            if (position % 10 == 0)
            {
                position++;
            }
            if (position % 10 == 9 && position / 10 < 4)
            {
                position == 49;
            }
            if (position % 10 == 9 && position / 10 > 8)
            {
                position == 89;
            }
            if (code == 13)
            {
                if (position == 69)
                {
                    if (bot_draw(information)) // компьютер анализирует предложение ничьей
                    {
                        printf("Компьютер согласился на ничью\n");
                        Game_over(information, 0);
                    }
                    else
                    {
                        error = 5;
                        end = false;
                    }
                    continue;
                }
                if (position == 59)
                {
                    printf("Вы сдались\n");
                    Game_over(information, information.player % 2 + 1);
                    continue;
                }
                if (position == 89)
                {
                    Save(information, 2);
                    end = false;
                    //PrintField(information, turns, first);
                    continue;
                }
                if (position == 79)
                {
                    int code = Load(information);
                    if (code != 0)
                    {
                        printf("Игра загружена\n");
                        return code;
                    }
                    end = false;
                    continue;
                }
                if (position == 49)
                {
                    return 0;
                }
                first[0] = position % 10 - 1 + 'a'; // запись позиции выбранной клетки
                first[1] = position / 10 + '0';
                index = finding_index(first);
                if (!Check_index(information, first, index))
                {
                    error = 1;
                    end = false;
                    first[0] = '0';
                    first[1] = '0';
                    continue;
                }
                if (eating)
                {
                    if (Be_in_turns(hunters, first) == false)
                    {
                        end = false;
                        error = 2;
                        first[0] = '0';
                        first[1] = '0';
                        continue;
                    }
                }
                else
                {
                    CanMove(information, turns, first, information.player, false);
                    if (turns[0].letter == '0')
                    {
                        end = false;
                        error = 3;
                        first[0] = '0';
                        first[1] = '0';
                        continue;
                    }
                }
                if (information.field[index].checker_color == information.player)
                {
                    do
                    {
                        Clear_can_turn_list(turns, 15);
                        if (eat == false)
                        {
                            eat = CanEating(information, first, information.player);
                        }
                        CanMove(information, turns, first, information.player, eat);
                        do
                        {
                            system("cls");
                            PrintField(information, turns, first, position);
                            if (error == 1)
                            {
                                printf("Сохранить игру во время хода нельзя!\n");
                            }
                            if (error == 3)
                            {
                                printf("Такой клетки не существует\n");
                            }
                            if (error == 4)
                            {
                                printf("Ход невозможен\n");
                            }
                            if (error == 5)
                            {
                                printf("Продолжение хода %d игрока\n", information.player);
                            }
                            error = 0;
                            currect_turn = true;
                            code = _getch();
                            if (code == 224)
                            {
                                code = _getch();
                                if (code == 0x48)
                                {
                                    position += 10;
                                }
                                if (code == 0x50)
                                {
                                    position -= 10;
                                }
                                if (code == 0x4B)
                                {
                                    position -= 1;
                                }
                                if (code == 0x4D && position % 10 < 9)
                                {
                                    if (position % 10 == 8)
                                    {
                                        position = 89;
                                    }
                                    else
                                    {
                                        position += 1;
                                    }
                                }
                            }
                            if (position / 10 == 9)
                            {
                                position -= 10;
                            }
                            if (position / 10 == 0)
                            {
                                position += 10;
                            }
                            if (position % 10 == 0)
                            {
                                position++;
                            }
                            if (position % 10 == 9 && position / 10 < 4)
                            {
                                position == 49;
                            }
                            if (position % 10 == 9 && position / 10 > 8)
                            {
                                position == 89;
                            }
                            if (code == 13)
                            {
                                if (position == 69)
                                {
                                    if (bot_draw(information))
                                    {
                                        printf("Компьютер согласился на ничью\n");
                                        Game_over(information, 0);
                                    }
                                    else
                                    {
                                        currect_turn = false;
                                    }
                                    continue;
                                }
                                if (position == 59)
                                {
                                    printf("Вы сдались\n");
                                    Game_over(information, information.player % 2 + 1);
                                    continue;
                                }
                                if (position == 89)
                                {
                                    error = 1;
                                    currect_turn = false;
                                    continue;
                                }
                                if (position == 79)
                                {
                                    int code = Load(information);
                                    if (code != 0)
                                    {
                                        printf("Игра загружена\n");
                                        return code;
                                    }
                                    currect_turn = false;
                                    continue;
                                }
                                if (position == 49)
                                {
                                    return 0;
                                }
                                second[0] = position % 10 - 1 + 'a';
                                second[1] = position / 10 + '0';
                                if (second[0] < 'a' || second[0] > 'h' || second[1] < '1' || second[1] > '8' && !Check_index(information, second, finding_index(second)))
                                {
                                    currect_turn = false;
                                    error = 3;
                                }
                                else
                                {
                                    if (Be_in_turns(turns, second) == false)
                                    {
                                        currect_turn = false;
                                        error = 4;
                                    }
                                }
                            }
                            else
                            {
                                currect_turn = false;
                            }
                        } while (currect_turn == false);
                        if (eat)
                        {
                            Checking_food_position(information, first, second, food, information.player);
                            Eating(information, first, second, food, information.player);
                            eat = CanEating(information, first, information.player);
                            end = true;
                            if (eat)
                            {
                                error = 5;
                            }
                        }
                        else
                        {
                            Moving(information, first, second, information.player);
                            end = true;
                        }
                    } while (eat);
                }
                else
                {
                    first[0] = '0';
                    first[1] = '0';
                    end = false;
                    error = 4;
                }
            }
            else
            {
                end = false;
                continue;
            }
            if (Haveturn(information, information.player % 2 + 1) == false)
            {
                first[0] = '0';
                first[1] = '0';
                Clear_can_turn_list(turns, 15);
                system("cls");
                PrintField(information, turns, first, 0);
                Game_over(information, information.player);
            }
            if (information.player == 1 && information.game_over == false) // если компьютер играет за черных, аналогично для белых, см выше
            {
                information.player = information.player % 2 + 1;
                Bot_turn_analisis(information, best_turn, 1, information.player);
                if (best_turn.eat)
                {
                    for (int i = 0; best_turn.first[i].letter != '0'; i++)
                    {
                        first[0] = best_turn.first[i].letter;
                        first[1] = best_turn.first[i].number + '0';
                        second[0] = best_turn.second[i].letter;
                        second[1] = best_turn.second[i].number + '0';
                        Checking_food_position(information, first, second, food, information.player);
                        Eating(information, first, second, food, information.player);
                    }
                }
                else
                {
                    first[0] = best_turn.first[0].letter;
                    first[1] = best_turn.first[0].number + '0';
                    second[0] = best_turn.second[0].letter;
                    second[1] = best_turn.second[0].number + '0';
                    Moving(information, first, second, 2);
                    first[0] = second[0];
                    first[1] = second[1];
                }
                if (Haveturn(information, information.player % 2 + 1) == false)
                {
                    Clear_can_turn_list(turns, 15);
                    system("cls");
                    PrintField(information, turns, first, 0);
                    Game_over(information, information.player);
                }
                information.player = information.player % 2 + 1;
            }
        }
    }
    system("pause");
    return 0;
}

void main() // главное меню игры
{
    int position = 1, position_two = 1; // позиция выделенной строки
    int code = 0, code_two = 0; // код нажатой стрелки
    bool loading; // выполнена ли загрузка
    setlocale(LC_ALL, "Rus");
    char buffer[100]; // буффер для ввода
    int n; // проверка на возвращение в главное меню или продолжение игры
    game_information information; // информация об игре
    Getting_settings(); // получение настроек для игры компьютера
    printf("Шашки\n\n");
    do // отображение главного меню
    {
        printf("Главное меню\n\n");
        if (position < 1) // если позиция выделенной строки вышла за границы
        {
            position = 1;
        }
        if (position > 6)
        {
            position = 6;
        }
        if (position == 1) // вывод меню, если текущая строка выделена
        {
            printf("[Новая игра]\n");
        }
        else
        {
            printf("Новая игра\n");
        }
        if (position == 2)
        {
            printf("[Загрузить]\n");
        }
        else
        {
            printf("Загрузить\n");
        }
        if (position == 3)
        {
            printf("[Настройки]\n");
        }
        else
        {
            printf("Настройки\n");
        }
        if (position == 4)
        {
            printf("[Правила]\n");
        }
        else
        {
            printf("Правила\n");
        }
        if (position == 5)
        {
            printf("[Справка]\n");
        }
        else
        {
            printf("Справка\n");
        }
        if (position == 6)
        {
            printf("[Выход]\n");
        }
        else
        {
            printf("Выход\n");
        }
        code = _getch();
        if (code == 224)
        {
            code = _getch(); // получение команды
            if (code == 0x48)
            {
                position -= 1;
            }
            if (code == 0x50)
            {
                position += 1;
            }
        }
        if (code == 13)
        {
            if (position == 1) // начало новой игры
            {
                loading = false; // изначально игра не загружена
                n = -1;
                Clear(information); // очистка информации об игре
                do // пока идет игра
                {
                    system("cls");
                    if (position_two < 1) // если позиция выделенной строки вышла за границы
                    {
                        position_two = 1;
                    }
                    if (position_two > 3)
                    {
                        position_two = 3;
                    }
                    if (position_two == 1)
                    {
                        printf("[Игра с другим игроком]\n");
                    }
                    else
                    {
                        printf("Игра с другим игроком\n");
                    }
                    if (position_two == 2)
                    {
                        printf("[Игра против компьютера]\n");
                    }
                    else
                    {
                        printf("Игра против компьютера\n");
                    }
                    if (position_two == 3)
                    {
                        printf("[Назад]\n");
                    }
                    else
                    {
                        printf("Назад\n");
                    }
                    if (!loading) // если игра не загружена нужно выбрать тип игры
                    {
                        code_two = _getch();
                        if (code_two == 224)
                        {
                            code_two = _getch();
                            if (code_two == 0x48)
                            {
                                position_two -= 1;
                            }
                            if (code_two == 0x50)
                            {
                                position_two += 1;
                            }
                        }
                    }
                    if (position_two == 2 && code_two == 13) // игра против компьютера
                    {
                        int position_three = 1, code_three = 0;
                        if (n == -1) // проверка для загрузки, если загружена не выполнять
                        {
                            do // выбор цвета шашек
                            {
                                system("cls");
                                printf("Выберите цвет ваших шашек:\n");
                                if (position_three == 1)
                                {
                                    printf("[Белые]\n");
                                    printf("Черные\n");
                                }
                                else
                                {
                                    printf("Белые\n");
                                    printf("[Черные]\n");
                                }
                                code_three = _getch();
                                if (code_three == 224)
                                {
                                    code_three = _getch();
                                    if (code_three == 0x48)
                                    {
                                        position_three = 1;
                                    }
                                    if (code_three == 0x50)
                                    {
                                        position_three = 2;
                                    }
                                }
                            } while (code_three != 13);
                            if (position_three == 2) // если цвет черные - игрок второй (по умолчанию первый)
                            {
                                information.player = 2;
                            }
                            n = 2;
                        }
                    }
                    if (n == 2) // игра с компьютером
                    {
                        n = Game_with_bot(information, loading); // осуществление игры, получение информации о отсутствии загруженной игры (0) или типе загруженной игры
                        loading = true;
                    }
                    if (position_two == 1 && code_two == 13) // игра с человеком
                    {
                        n = 1;
                    }
                    if (n == 1)
                    {
                        n = Hotseat_game(information);
                        loading = true;
                    }
                    if (position_two == 3 && code_two == 13) // возвращение в главное меню
                    {
                        n = 0;
                    }
                } while (n != 0);
            }
            if (position == 2) // загрузка игры
            {
                n = Load(information); // получение типа загруженной игры
                do
                {
                    if (n == 1) // осуществление игры
                    {
                        n = Hotseat_game(information);
                    }
                    if (n == 2)
                    {
                        n = Game_with_bot(information, true);
                    }
                } while (n != 0);
            }
            if (position == 3)
            {
                Change_settings(); // изменение настроек
            }
            if (position == 4)
            {
                char name[5];
                strcpy_s(name, "Rule");
                Open_file(name); // вывод правил
            }
            if (position == 5)
            {
                char name[5];
                strcpy_s(name, "Help"); // вывод справки
                Open_file(name);
            }
            printf("\n");
        }
        system("cls");
    } while (position != 6 || code != 13); // пока игра не закончена
}