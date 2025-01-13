#include <iostream>
#include <fstream>
#include <string>

const char SPACE = ' ';
const char WALL = '#';
const char COIN = 'C';
const char PORTAL = '%';
const char KEY = '&';
const char TREASURE = 'X';
const char PLAYER = '@';

const int MAX_LEVEL = 3;
const int DEFAULT_LIVES = 3;
const int NAME_MAX_LENGTH = 51;

struct MapCoordinate
{
    size_t rowIdx;
    size_t colIdx;
};

struct Map
{
    int rowsCount;
    int colsCount;
    char** matrix;
    MapCoordinate playerPosition;
};

struct Game
{
    bool keyFound = false;
    bool treasureFound = false;
    int coinsCollected = 0;
    int totalCoins;
    int level;
    Map map;
};

struct Player
{
    char name[NAME_MAX_LENGTH];
    int level = 1;
    int lives = DEFAULT_LIVES;
    int coins = 0;
    Game lastGame;
};

void clearConsole()
{
    std::cout << "\033[;H"; // Moves cursor to the top left
    std::cout << "\033[J"; // Clears the console
}

char** initDefaultMatrix(size_t rowCount, size_t colCount, char defaultSymbol)
{
    char** matrix = new char* [rowCount];

    for (size_t i = 0; i < rowCount; i++)
    {
        matrix[i] = new char[colCount];

        for (size_t j = 0; j < colCount; j++)
        {
            matrix[i][j] = defaultSymbol;
        }
    }

    return matrix;
}

char** initMatrix(size_t rows, size_t cols)
{
    char** matrix = new char* [rows];

    for (size_t i = 0; i < rows; i++)
    {
        matrix[i] = new char[cols];
    }

    return matrix;
}

Game readGame(std::ifstream& inMap, size_t rowCount, size_t colCount, size_t level)
{
    Game game = {};
    if (level > MAX_LEVEL)
    {
        return game;
    }

    if (!inMap.is_open())
    {
        return game;
    }

    game.level = level;
    Map map = {};
    map.rowsCount = rowCount;
    map.colsCount = colCount;
    map.matrix = initMatrix(rowCount, colCount);

    for (size_t i = 0; i < rowCount; i++)
    {
        for (size_t j = 0; j < colCount; j++)
        {
            char ch;
            do
            {
                inMap.get(ch);
            } while (ch == '\n');

            if (ch == PLAYER)
            {
                map.playerPosition = { i, j };
                map.matrix[i][j] = SPACE;
            }
            else
            {
                map.matrix[i][j] = ch;
            }
            if (ch == COIN)
            {
                game.totalCoins++;
            }
        }
    }

    game.map = map;
    return game;
}

void printMatrix(const Map& map)
{
    if (map.matrix == nullptr)
    {
        return;
    }

    std::cout << std::endl;

    for (size_t i = 0; i < map.rowsCount; i++)
    {
        for (size_t j = 0; j < map.colsCount; j++)
        {
            if (i == map.playerPosition.rowIdx
                && j == map.playerPosition.colIdx)
            {
                std::cout << PLAYER;
            }
            else
            {
                std::cout << map.matrix[i][j];
            }
            std::cout << "  ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void deleteMatrix(char** matrix, size_t rows)
{
    if (matrix == nullptr)
    {
        return;
    }

    for (size_t i = 0; i < rows; i++)
    {
        delete[] matrix[i];
    }

    delete[] matrix;
}

void printGameInfo(const Game& game, const Player& player)
{
    std::cout << "Level: " << game.level << std::endl;
    std::cout << "Lives: " << player.lives << std::endl;
    std::cout << "Coins: " << game.coinsCollected << "/" << game.totalCoins << std::endl;
    std::cout << "Key: ";

    if (game.keyFound)
    {
        std::cout << "Found";
    }
    else
    {
        std::cout << "Not found";
    }

    std::cout << std::endl;
}

int main()
{
    const char mapPath[] = "../Maps/Level 1/Map 1.txt";
    const int rows = 10;
    const int cols = 15;

    char** pMatrix = readMap(mapPath, rows, cols);
    printMatrix(pMatrix, rows, cols);
    deleteMatrix(pMatrix, rows);

    return 0;
}
