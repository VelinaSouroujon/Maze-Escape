#include <iostream>
#include <fstream>
#include <string>

const size_t NAME_MAX_LENGTH = 50;

struct Game
{
    bool keyFound;
    int coinsCollected;
    int totalCoins;
    char** map;
};

struct Player
{
    char name[NAME_MAX_LENGTH];
    int level;
    int lives;
    int coins;
    char** highestLevelGame;
};

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

char** readMap(const char* mapPath, size_t rowCount, size_t colCount)
{
    if (mapPath == nullptr)
    {
        return nullptr;
    }

    std::ifstream map(mapPath);
    if (!map.is_open())
    {
        return nullptr;
    }

    const char DEFAULT_SYMBOL = ' ';
    char** matrix = initDefaultMatrix(rowCount, colCount + 1, DEFAULT_SYMBOL);

    int rowIdx = 0;

    while (rowIdx < rowCount
        && map.getline(matrix[rowIdx], colCount + 1))
    {
        rowIdx++;
    }

    map.close();

    return matrix;
}

void printMatrix(char** matrix, size_t rows, size_t cols)
{
    if (matrix == nullptr)
    {
        return;
    }

    for (size_t i = 0; i < rows; i++)
    {
        for (size_t j = 0; j < cols; j++)
        {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
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
