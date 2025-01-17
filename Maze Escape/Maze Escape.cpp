#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>

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
    Game* savedGamesPerLevel[MAX_LEVEL] = {nullptr};
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

bool readMatrix(std::ifstream& inMap, Game& game)
{
    if (!inMap.is_open())
    {
        return false;
    }

    Map& map = game.map;

    for (size_t i = 0; i < map.rowsCount; i++)
    {
        for (size_t j = 0; j < map.colsCount; j++)
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

    return true;
}

bool readGame(Game& game, std::ifstream& inMap)
{
    if (!inMap.is_open())
    {
        return false;
    }

    Map& map = game.map;
    inMap >> map.rowsCount;
    inMap >> map.colsCount;
    inMap.ignore();

    map.matrix = initMatrix(map.rowsCount, map.colsCount);
    readMatrix(inMap, game);

    return true;
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

void printRulesToMove()
{
    std::cout << "Press the keys below to move: " << std::endl;
    std::cout << "W - Up" << std::endl;
    std::cout << "S - Down" << std::endl;
    std::cout << "A - Left" << std::endl;
    std::cout << "D - Right" << std::endl;
}

char toLower(char ch)
{
    if (ch >= 'A' && ch <= 'Z')
    {
        return ch + ('a' - 'A');
    }

    return ch;
}

bool isValidCoordinate(const MapCoordinate& coordinate, int rows, int cols)
{
    return coordinate.rowIdx < rows && coordinate.colIdx < cols;
}

bool changePosition(MapCoordinate& pCoordinate, char playerMove)
{
    playerMove = toLower(playerMove);

    switch (playerMove)
    {
    case 'w':
        pCoordinate.rowIdx--;
        break;

    case 's':
        pCoordinate.rowIdx++;
        break;

    case 'a':
        pCoordinate.colIdx--;
        break;

    case 'd':
        pCoordinate.colIdx++;
        break;

    default:
        return false;
    }

    return true;
}

MapCoordinate findNextPortal(const Map& map, const MapCoordinate& currPortal)
{
    MapCoordinate nextPortal = {};

    if (map.matrix == nullptr)
    {
        return nextPortal;
    }

    int startRow, startCol;

    if (currPortal.rowIdx == map.rowsCount - 1
        && currPortal.colIdx == map.colsCount - 1)
    {
        startRow = 0;
        startCol = 0;
    }
    else
    {
        startRow = currPortal.rowIdx;
        startCol = currPortal.colIdx + 1;
    }

    bool portalFound = false;
    for (int i = startRow; i < map.rowsCount; i++)
    {
        if (i != startRow)
        {
            startCol = 0;
        }

        for (int j = startCol; j < map.colsCount; j++)
        {
            if (map.matrix[i][j] == PORTAL)
            {
                portalFound = true;
                nextPortal.rowIdx = i;
                nextPortal.colIdx = j;
                break;
            }

            if (i == map.rowsCount - 1 && j == map.colsCount - 1)
            {
                i = 0;
                j = -1;
            }
        }

        if (portalFound)
        {
            break;
        }
    }

    return nextPortal;
}

void move(Player& player, Game& game, char playerMove)
{
    char** matrix = game.map.matrix;
    MapCoordinate& plCoordinate = game.map.playerPosition;
    MapCoordinate newPosition = plCoordinate;

    if (matrix == nullptr)
    {
        return;
    }

    if (!changePosition(newPosition, playerMove))
    {
        return;
    }

    if (!isValidCoordinate(newPosition, game.map.rowsCount, game.map.colsCount))
    {
        return;
    }

    switch (matrix[newPosition.rowIdx][newPosition.colIdx])
    {
    case WALL:
        player.lives--;
        break;

    case SPACE:
        plCoordinate = newPosition;
        break;

    case COIN:
        game.coinsCollected++;
        plCoordinate = newPosition;
        matrix[newPosition.rowIdx][newPosition.colIdx] = SPACE;
        break;

    case KEY:
        game.keyFound = true;
        plCoordinate = newPosition;
        matrix[newPosition.rowIdx][newPosition.colIdx] = SPACE;
        break;

    case PORTAL:
        plCoordinate = findNextPortal(game.map, newPosition);
        break;

    case TREASURE:
        plCoordinate = newPosition;

        if (game.keyFound)
        {
            game.treasureFound = true;
        }

        break;
    }
}


bool savePlayerInfo(std::ofstream& outFile, const Player& player)
{
    if (!outFile.is_open())
    {
        return false;
    }

    outFile << player.name << std::endl;
    outFile << player.level << std::endl;
    outFile << player.lives << std::endl;
    outFile << player.coins << std::endl;

    return true;
}

bool appendMapInfo(std::ofstream& outFile, const Map& map)
{
    if (!outFile.is_open())
    {
        return false;
    }

    outFile << map.rowsCount << std::endl;
    outFile << map.colsCount << std::endl;

    for (size_t i = 0; i < map.rowsCount; i++)
    {
        for (size_t j = 0; j < map.colsCount; j++)
        {
            if (i == map.playerPosition.rowIdx
                && j == map.playerPosition.colIdx)
            {
                outFile << PLAYER;
            }
            else
            {
                outFile << map.matrix[i][j];
            }
        }
        outFile << std::endl;
    }

    return true;
}

bool appendGameInfo(std::ofstream& outFile, const Game& game)
{
    if (game.map.matrix == nullptr)
    {
        return false;
    }

    if (!outFile.is_open())
    {
        return false;
    }

    outFile << game.keyFound << std::endl;
    outFile << game.coinsCollected << std::endl;
    outFile << game.level << std::endl;

    appendMapInfo(outFile, game.map);

    return true;
}

int getStrLen(const char* str)
{
    if (str == nullptr)
    {
        return -1;
    }

    int length = 0;

    while (*str != '\0')
    {
        length++;
        str++;
    }

    return length;
}

void strCopy(const char* source, char* dest, size_t destStartIdx)
{
    if (source == nullptr || dest == nullptr)
    {
        return;
    }

    size_t idx = 0;
    while (source[idx] != '\0')
    {
        dest[idx + destStartIdx] = source[idx];
        idx++;
    }

    dest[idx + destStartIdx] = '\0';
}

int getTotalLength(const char* const* strings, size_t len, int* lengthMap)
{
    if (strings == nullptr || lengthMap == nullptr)
    {
        return -1;
    }

    int totalLen = 0;

    for (size_t i = 0; i < len; i++)
    {
        int currLen = getStrLen(strings[i]);
        if (currLen == -1)
        {
            lengthMap[i] = 0;
            continue;
        }

        lengthMap[i] = currLen;
        totalLen += currLen;
    }

    return totalLen;
}

char* getFilePath(const char* const* folders, size_t len, const char* extension = "txt")
{
    if (folders == nullptr || extension == nullptr || len == 0)
    {
        return nullptr;
    }

    const char DELIMITER = '/';
    const char EXTENSION_DELIMITER = '.';
    int delimetersCount = len;

    int* lenMap = new int[len];
    int extensionLen = getStrLen(extension);

    int totalLen = getTotalLength(folders, len, lenMap) + extensionLen + delimetersCount;

    char* result = new char[totalLen + 1];
    result[totalLen] = '\0';

    size_t resultIdx = 0;
    bool isFirst = true;

    for (size_t i = 0; i < len; i++)
    {
        if (folders[i] == nullptr)
        {
            continue;
        }

        if (isFirst)
        {
            isFirst = false;
        }
        else
        {
            result[resultIdx] = DELIMITER;
            resultIdx++;
        }

        strCopy(folders[i], result, resultIdx);
        resultIdx += lenMap[i];
    }

    delete[] lenMap;

    if (extensionLen > 0)
    {
        result[resultIdx] = EXTENSION_DELIMITER;
        resultIdx++;
    }

    strCopy(extension, result, resultIdx);

    return result;
}

void swap(int& first, int& second)
{
    int temp = first;
    first = second;
    second = temp;
}

void initRandom()
{
    srand(time(0));
}

int getRandomNumber(int min, int max)
{
    if (min > max)
    {
        swap(min, max);
    }

    int random = min + rand() % (max - min + 1);
    return random;
}

void playGame(Game& game, Player& player)
{
    if (game.map.matrix == nullptr)
    {
        return;
    }

    char playerMove;

    while (true)
    {
        clearConsole();
        printGameInfo(game, player);
        printMatrix(game.map);

        if (game.treasureFound)
        {
            player.coins += game.coinsCollected;

            if ((game.level != MAX_LEVEL)
                && player.level == game.level)
            {
                player.level++;
            }

            std::cout << "Congratulations! You win!" << std::endl;
            break;
        }
        if (player.lives == 0)
        {
            player.lives = DEFAULT_LIVES;
            std::cout << "You lose! Better luck next game!" << std::endl;
            break;
        }

        printRulesToMove();

        std::cin >> playerMove;
        if (toLower(playerMove) == QUIT)
        {
            return;
        }

        move(player, game, playerMove);
    }
}

int main()
{
    return 0;
}