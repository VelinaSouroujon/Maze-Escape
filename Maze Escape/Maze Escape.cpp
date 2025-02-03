#define NOMINMAX

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <Windows.h>

const char SPACE = ' ';
const char WALL = '#';
const char COIN = 'C';
const char PORTAL = '%';
const char KEY = '&';
const char TREASURE = 'X';
const char PLAYER = '@';
const char ENEMY = 'E';

const char VISITED = 'V';

const char QUIT = 'q';

const char UP = 'w';
const char DOWN = 's';
const char LEFT = 'a';
const char RIGHT = 'd';

const int MIN_LEVEL = 1;
const int MAX_LEVEL = 3;
const int DEFAULT_LIVES = 3;
const int NAME_MIN_LENGTH = 2;
const int NAME_MAX_LENGTH = 51;
const int LIFE_PRICE = 50;

const int GREEN_COLOR = 2;
const int RED_COLOR = 4;
const int WHITE_COLOR = 7;

enum MoveResult
{
    NONE,
    INVALID_MOVE,
    WALL_HIT,
    COIN_COLLECTED,
    KEY_FOUND,
    TELEPORTATION,
    TREASURE_WITHOUT_KEY,
    TREASURE_WITH_KEY,
    ENEMY_ENCOUNTER
};

struct MapCoordinate
{
    size_t rowIdx;
    size_t colIdx;
};

struct VisitedCell
{
    char originalContent;
    int steps;
    int parentIndex;
    MapCoordinate coordinate;
};

struct Map
{
    int rowsCount;
    int colsCount;
    int portalsCount;
    char** matrix;
    MapCoordinate playerPosition;
    MapCoordinate enemyPosition;
    MapCoordinate* portals;
};

struct Game
{
    bool keyFound = false;
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
    Game savedGamesPerLevel[MAX_LEVEL] = {};
};

char toLower(char ch)
{
    if (ch >= 'A' && ch <= 'Z')
    {
        return ch + ('a' - 'A');
    }

    return ch;
}

int strCompare(const char* str1, const char* str2)
{
    if (str1 == nullptr || str2 == nullptr)
    {
        return -2;
    }

    while (true)
    {
        unsigned char ch1 = (unsigned char)*str1;
        unsigned char ch2 = (unsigned char)*str2;

        if (ch1 < ch2)
        {
            return -1;
        }
        if (ch1 > ch2)
        {
            return 1;
        }
        if (ch1 == '\0')
        {
            return 0;
        }

        str1++;
        str2++;
    }
}

bool isValidIdx(int idx, size_t arrLen)
{
    return idx >= 0 && idx < arrLen;
}

void clearConsole()
{
    std::cout << "\033[;H"; // Moves cursor to the top left
    std::cout << "\033[2J"; // Clears the entire screen
    std::cout << "\033[3J"; // Clears the scrollback buffer
}

void setConsoleColor(int colorNumber)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, colorNumber);
}

void printCharWithColorAndReset(char ch, int color)
{
    setConsoleColor(color);
    std::cout << ch;
    setConsoleColor(WHITE_COLOR);
}

bool isInRange(int value, int from, int to)
{
    return value >= from
        && value <= to;
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

bool fileExists(const char* name)
{
    if (name == nullptr)
    {
        return false;
    }

    std::ifstream file(name);

    if (file.is_open())
    {
        file.close();
        return true;
    }

    return false;
}

void strToLower(const char* inputStr, char* result)
{
    if (inputStr == nullptr || result == nullptr)
    {
        return;
    }

    int idx = 0;

    while (inputStr[idx] != '\0')
    {
        result[idx] = toLower(inputStr[idx]);
        idx++;
    }

    result[idx] = '\0';
}

char* getPlayerFilePath(const char* name)
{
    if (name == nullptr)
    {
        return nullptr;
    }

    char nameToLower[NAME_MAX_LENGTH];
    strToLower(name, nameToLower);

    const int foldersCount = 2;
    const char playerDirPath[] = "../Players";
    const char* folders[foldersCount] = { playerDirPath, nameToLower };
    char* filePath = getFilePath(folders, foldersCount);

    return filePath;
}

char* getPlayerNamesFilePath()
{
    const char plNamesDirPath[] = "../Names";
    const int foldersCount = 1;
    const char* pNamesDir[foldersCount] = { plNamesDirPath };
    char* filePath = getFilePath(pNamesDir, foldersCount);

    return filePath;
}

bool readPlayerInfo(std::ifstream& inFile, Player& player)
{
    if (!inFile.is_open())
    {
        return false;
    }

    inFile.getline(player.name, NAME_MAX_LENGTH);
    inFile >> player.level;
    inFile >> player.lives;
    inFile >> player.coins;

    return true;
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

void deleteMatrix(char**& matrix, size_t rows)
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

    matrix = nullptr;
}

void deleteMap(Map& map)
{
    deleteMatrix(map.matrix, map.rowsCount);
    delete[] map.portals;
}

bool readMatrix(std::ifstream& inMap, Game& game)
{
    if (!inMap.is_open())
    {
        return false;
    }

    Map& map = game.map;
    int portalIdx = 0;

    for (size_t row = 0; row < map.rowsCount; row++)
    {
        for (size_t col = 0; col < map.colsCount; col++)
        {
            char ch;
            do
            {
                inMap.get(ch);
            } while (ch == '\n');

            if (inMap.eof())
            {
                return false;
            }

            if (ch == PLAYER)
            {
                map.playerPosition = { row, col };
                map.matrix[row][col] = SPACE;
                continue;
            }
            if (ch == ENEMY)
            {
                map.enemyPosition = { row, col };
                map.matrix[row][col] = SPACE;
                continue;
            }

            map.matrix[row][col] = ch;

            if (ch == COIN)
            {
                game.totalCoins++;
            }
            else if (ch == PORTAL)
            {
                map.portals[portalIdx] = { row, col };
                portalIdx++;
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
    inMap >> map.portalsCount;
    inMap.ignore();

    map.matrix = initMatrix(map.rowsCount, map.colsCount);
    map.portals = new MapCoordinate[map.portalsCount];

    if (!readMatrix(inMap, game))
    {
        deleteMap(map);
        return false;
    }

    return true;
}

bool readSavedGames(std::ifstream& inFile, Player& player)
{
    if (!inFile.is_open())
    {
        return false;
    }

    while (inFile.peek() != EOF)
    {
        Game game = {};
        inFile >> game.keyFound;
        inFile >> game.coinsCollected;
        inFile >> game.level;
        game.totalCoins += game.coinsCollected;

        readGame(game, inFile);
        player.savedGamesPerLevel[game.level - 1] = game;
        inFile.ignore();
    }

    return true;
}

bool getPlayerByName(const char* name, Player& player)
{
    if (name == nullptr)
    {
        return false;
    }

    char* filePath = getPlayerFilePath(name);
    std::ifstream inFile(filePath);
    delete[] filePath;

    if (!inFile.is_open())
    {
        return false;
    }

    readPlayerInfo(inFile, player);
    inFile.ignore();
    readSavedGames(inFile, player);

    inFile.close();

    return true;
}

int getDigitsCount(int num)
{
    if (num == 0)
    {
        return 1;
    }

    int digitsCount = 0;

    while (num != 0)
    {
        num /= 10;
        digitsCount++;
    }

    return digitsCount;
}

char* intToString(size_t num)
{
    int digitsCount = getDigitsCount(num);
    char* strNum = new char[digitsCount + 1];
    strNum[digitsCount] = '\0';
    int idx = digitsCount - 1;

    while (idx >= 0)
    {
        strNum[idx] = (num % 10) + '0';
        num /= 10;
        idx--;
    }

    return strNum;
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

char* getMapFilePath(size_t level, size_t mapsCount)
{
    if (level > MAX_LEVEL)
    {
        return nullptr;
    }

    const char mapsDirPath[] = "../Maps";
    char* strLevel = intToString(level);

    int mapNumber = getRandomNumber(1, mapsCount);
    char* strMapNumber = intToString(mapNumber);

    const int foldersCount = 3;
    const char* mapsPathLevel[foldersCount] = { mapsDirPath, strLevel, strMapNumber };
    char* filePath = getFilePath(mapsPathLevel, foldersCount);

    delete[] strLevel;
    delete[] strMapNumber;

    return filePath;
}

bool isSamePosition(const MapCoordinate& firstPosition, const MapCoordinate& secondPosition)
{
    return firstPosition.rowIdx == secondPosition.rowIdx
        && firstPosition.colIdx == secondPosition.colIdx;
}

void printPlayerInfo(const Player& player)
{
    std::cout << player.name << ": ";

    std::cout << player.level << " level; ";
    std::cout << player.coins << " coins; ";
    std::cout << player.lives << " lives";

    std::cout << std::endl;
}

void swapPlayers(std::vector<Player>& players, size_t firstIdx, size_t secondIdx)
{
    if (firstIdx >= players.size() || secondIdx >= players.size())
    {
        return;
    }

    Player temp = players[firstIdx];
    players[firstIdx] = players[secondIdx];
    players[secondIdx] = temp;
}

int compareDesc(int first, int second)
{
    if (first > second)
    {
        return -1;
    }

    if (first < second)
    {
        return 1;
    }

    return 0;
}

int comparePlayers(const Player& firstPlayer, const Player& secondPlayer)
{
    int result = compareDesc(firstPlayer.level, secondPlayer.level);
    if (result != 0)
    {
        return result;
    }

    result = compareDesc(firstPlayer.coins, secondPlayer.coins);
    if (result != 0)
    {
        return result;
    }

    result = compareDesc(firstPlayer.lives, secondPlayer.lives);
    return result;
}

void sortPlayers(std::vector<Player>& players, int startIdx, int endIdx)
{
    size_t playersSize = players.size();
    if ((!isValidIdx(startIdx, playersSize)) || (!isValidIdx(endIdx, playersSize)))
    {
        return;
    }

    if (startIdx >= endIdx)
    {
        return;
    }

    int pivot = startIdx;
    int left = pivot + 1;
    int right = endIdx;

    while (left <= right)
    {
        if (comparePlayers(players[left], players[pivot]) > 0
            && comparePlayers(players[right], players[pivot]) < 0)
        {
            swapPlayers(players, left, right);
        }

        if (comparePlayers(players[left], players[pivot]) <= 0)
        {
            left++;
        }

        if (comparePlayers(players[right], players[pivot]) >= 0)
        {
            right--;
        }
    }

    swapPlayers(players, pivot, right);

    int firstSubvectorStart = startIdx;
    int firstSubvectorEnd = right - 1;

    int secondSubvectorStart = right + 1;
    int secondSubvectorEnd = endIdx;

    int firstSubvectorSize = firstSubvectorEnd - firstSubvectorStart + 1;
    int secondSubvectorSize = secondSubvectorEnd - secondSubvectorStart + 1;

    if (firstSubvectorSize <= secondSubvectorSize)
    {
        sortPlayers(players, firstSubvectorStart, firstSubvectorEnd);
        sortPlayers(players, secondSubvectorStart, secondSubvectorEnd);
    }
    else
    {
        sortPlayers(players, secondSubvectorStart, secondSubvectorEnd);
        sortPlayers(players, firstSubvectorStart, firstSubvectorEnd);
    }
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

void printMatrix(const Map& map, int playerColor, int enemyColor)
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
            MapCoordinate currPosition = { i, j };

            if (isSamePosition(currPosition, map.playerPosition))
            {
                printCharWithColorAndReset(PLAYER, playerColor);
            }
            else if (isSamePosition(currPosition, map.enemyPosition))
            {
                printCharWithColorAndReset(ENEMY, enemyColor);
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
    std::cout << "Press one of the keys below:" << std::endl;
    std::cout << "W - Up" << std::endl;
    std::cout << "S - Down" << std::endl;
    std::cout << "A - Left" << std::endl;
    std::cout << "D - Right" << std::endl;
    std::cout << "Q - Quit the level saving the progress" << std::endl;
}

int readNumber()
{
    int num;
    std::cin >> num;

    while (std::cin.fail())
    {
        std::cout << "Invalid input! Please enter a number." << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin >> num;
    }

    return num;
}

int getNumberInRange(int from, int to)
{
    int num = readNumber();

    while (!isInRange(num, from, to))
    {
        std::cout << "Invalid number! Please enter a number between " << from << " and " << to << std::endl;
        num = readNumber();
    }

    return num;
}

int getInputOption()
{
    std::cout << "Please choose one of the following options:" << std::endl;
    std::cout << "1) Log in" << std::endl;
    std::cout << "2) Sign up" << std::endl;

    int inputOption = getNumberInRange(1, 2);
    return inputOption;
}

bool inputYesNo(const char* question)
{
    if (question == nullptr)
    {
        return false;
    }

    std::cout << question;
    std::cout << " Enter one of the numbers below:" << std::endl;

    const int YES = 1;
    const int NO = 2;

    std::cout << YES << ") Yes" << std::endl;
    std::cout << NO << ") No" << std::endl;

    int optionNumber = getNumberInRange(YES, NO);
    return optionNumber == YES;
}

bool isValidCoordinate(const MapCoordinate& coordinate, int rows, int cols)
{
    return coordinate.rowIdx < rows && coordinate.colIdx < cols;
}

bool isValidEnemyMove(const MapCoordinate& newPosition, const Map& map)
{
    return isValidCoordinate(newPosition, map.rowsCount, map.colsCount)
        && (map.matrix[newPosition.rowIdx][newPosition.colIdx] != WALL)
        && (map.matrix[newPosition.rowIdx][newPosition.colIdx] != VISITED);
}

bool changePosition(MapCoordinate& pCoordinate, char playerMove)
{
    playerMove = toLower(playerMove);

    switch (playerMove)
    {
    case UP:
        pCoordinate.rowIdx--;
        break;

    case DOWN:
        pCoordinate.rowIdx++;
        break;

    case LEFT:
        pCoordinate.colIdx--;
        break;

    case RIGHT:
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

    for (size_t i = 0; i < map.portalsCount; i++)
    {
        if (isSamePosition(currPortal, map.portals[i]))
        {
            if (i == map.portalsCount - 1)
            {
                nextPortal = map.portals[0];
            }
            else
            {
                nextPortal = map.portals[i + 1];
            }

            break;
        }
    }

    return nextPortal;
}

void restoreMatrix(Map& map, const std::vector<VisitedCell>& visitedCells)
{
    for (size_t i = 0; i < visitedCells.size(); i++)
    {
        char originalContent = visitedCells[i].originalContent;
        size_t row = visitedCells[i].coordinate.rowIdx;
        size_t col = visitedCells[i].coordinate.colIdx;

        map.matrix[row][col] = originalContent;
    }
}

void markVisited(Map& map, const MapCoordinate& position, int parentIndex, int steps, std::vector<VisitedCell>& visitedCells)
{
    size_t row = position.rowIdx;
    size_t col = position.colIdx;

    char originalSymbol = map.matrix[row][col];
    map.matrix[row][col] = VISITED;
    visitedCells.push_back({ originalSymbol, steps, parentIndex, position });
}

MapCoordinate restorePath(const std::vector<VisitedCell>& visitedCells, const Map& map, size_t enemyStepsPerMove)
{
    VisitedCell currCell = visitedCells[visitedCells.size() - 1];
    int totalSteps = currCell.steps;
    int stepsBack = totalSteps - enemyStepsPerMove;
    if (stepsBack <= 0)
    {
        return map.playerPosition;
    }

    for (size_t i = 0; i < stepsBack; i++)
    {
        VisitedCell parent = visitedCells[currCell.parentIndex];
        currCell = parent;
    }

    return currCell.coordinate;
}

MapCoordinate findShortestPath(Map& map, std::vector<VisitedCell>& visitedCells, size_t enemyStepsPerMove)
{
    MapCoordinate enemyNewPosition = {};

    if (map.matrix == nullptr)
    {
        return enemyNewPosition;
    }

    std::vector<MapCoordinate> queue;
    queue.push_back(map.enemyPosition);

    int parentIndex = -1;
    const int initialSteps = 0;
    markVisited(map, map.enemyPosition, parentIndex, initialSteps, visitedCells);

    bool pathFound = false;

    while (queue.size() > 0)
    {
        parentIndex++;

        MapCoordinate currPosition = queue[0];
        queue.erase(queue.begin());

        const int directionsRows = 4;
        const int directionsCols = 2;
        int directions[directionsRows][directionsCols] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
        for (size_t i = 0; i < directionsRows; i++)
        {
            size_t newRow = currPosition.rowIdx + directions[i][0];
            size_t newCol = currPosition.colIdx + directions[i][1];
            MapCoordinate newPosition = { newRow, newCol };

            if (!isValidEnemyMove(newPosition, map))
            {
                continue;
            }

            int steps = visitedCells[parentIndex].steps + 1;
            markVisited(map, newPosition, parentIndex, steps, visitedCells);

            if (newRow == map.playerPosition.rowIdx
                && newCol == map.playerPosition.colIdx)
            {
                pathFound = true;
                break;
            }

            queue.push_back(newPosition);
        }

        if (pathFound)
        {
            break;
        }
    }

    restoreMatrix(map, visitedCells);
    enemyNewPosition = restorePath(visitedCells, map, enemyStepsPerMove);
    visitedCells.clear();
    
    return enemyNewPosition;
}

MoveResult move(Player& player, Game& game, char playerMove)
{
    char** matrix = game.map.matrix;
    MapCoordinate& plCoordinate = game.map.playerPosition;
    MapCoordinate newPosition = plCoordinate;

    if (matrix == nullptr)
    {
        return INVALID_MOVE;
    }

    if (!changePosition(newPosition, playerMove))
    {
        return INVALID_MOVE;
    }

    if (!isValidCoordinate(newPosition, game.map.rowsCount, game.map.colsCount))
    {
        return INVALID_MOVE;
    }
    if (isSamePosition(newPosition, game.map.enemyPosition))
    {
        player.lives = 0;
        return ENEMY_ENCOUNTER;
    }

    switch (matrix[newPosition.rowIdx][newPosition.colIdx])
    {
    case WALL:
        player.lives--;
        return WALL_HIT;

    case SPACE:
        plCoordinate = newPosition;
        return NONE;

    case COIN:
        game.coinsCollected++;
        plCoordinate = newPosition;
        matrix[newPosition.rowIdx][newPosition.colIdx] = SPACE;
        return COIN_COLLECTED;

    case KEY:
        game.keyFound = true;
        plCoordinate = newPosition;
        matrix[newPosition.rowIdx][newPosition.colIdx] = SPACE;
        return KEY_FOUND;

    case PORTAL:
        plCoordinate = findNextPortal(game.map, newPosition);
        return TELEPORTATION;

    case TREASURE:
        plCoordinate = newPosition;

        if (game.keyFound)
        {
            return TREASURE_WITH_KEY;
        }

        return TREASURE_WITHOUT_KEY;

    default:
        return INVALID_MOVE;
    }
}

bool winCondition(MoveResult moveRes)
{
    return moveRes == TREASURE_WITH_KEY;
}

bool lossCondition(const Player& player)
{
    return player.lives == 0;
}

void winUpdate(const Game& game, Player& player)
{
    if (game.map.matrix == nullptr)
    {
        return;
    }

    player.coins += game.coinsCollected;

    if ((game.level != MAX_LEVEL)
        && player.level == game.level)
    {
        player.level++;
    }
}

void lossUpdate(Player& player)
{
    player.lives = 1;
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
    outFile << map.portalsCount << std::endl;

    for (size_t i = 0; i < map.rowsCount; i++)
    {
        for (size_t j = 0; j < map.colsCount; j++)
        {
            MapCoordinate currPosition = { i, j };

            if (isSamePosition(currPosition, map.playerPosition))
            {
                outFile << PLAYER;
            }
            else if (isSamePosition(currPosition, map.enemyPosition))
            {
                outFile << ENEMY;
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

bool appendPlayerNameToFile(const char* name)
{
    if (name == nullptr)
    {
        return false;
    }

    char nameToLower[NAME_MAX_LENGTH];
    strToLower(name, nameToLower);
    char* plFilePath = getPlayerFilePath(nameToLower);

    if (fileExists(plFilePath))
    {
        delete[] plFilePath;
        return false;
    }

    delete[] plFilePath;

    char* filePath = getPlayerNamesFilePath();

    std::ofstream outFile(filePath, std::ios::app);
    delete[] filePath;

    if (!outFile.is_open())
    {
        return false;
    }

    outFile << nameToLower << std::endl;
    outFile.close();

    return true;
}

void enterUsername(Player& player)
{
    std::cout << "Please enter username:" << std::endl;
    std::cin.getline(player.name, NAME_MAX_LENGTH);

    while (getStrLen(player.name) < NAME_MIN_LENGTH)
    {
        std::cout << "Your name must have at least " << NAME_MIN_LENGTH << " symbols. Please, try again!" << std::endl;
        std::cin.getline(player.name, NAME_MAX_LENGTH);
    }
}

bool savePlayerGames(std::ofstream& outFile, const Player& player)
{
    if (!outFile.is_open())
    {
        return false;
    }

    for (size_t i = 0; i < player.level; i++)
    {
        const Game& savedGame = player.savedGamesPerLevel[i];

        if (savedGame.map.matrix == nullptr)
        {
            continue;
        }

        appendGameInfo(outFile, savedGame);
    }

    return true;
}

bool savePlayerProgress(const Player& player)
{
    if (player.name == nullptr)
    {
        return false;
    }

    char* filePath = getPlayerFilePath(player.name);
    std::ofstream outFile(filePath);
    delete[] filePath;

    if (!outFile.is_open())
    {
        return false;
    }

    savePlayerInfo(outFile, player);
    savePlayerGames(outFile, player);

    outFile.close();

    return true;
}

int getGameLevel(const Player& player)
{
    int maxLevel = player.level;

    if (maxLevel == MIN_LEVEL)
    {
        return maxLevel;
    }

    std::cout << "Please enter the level you want to play. It must be between " << MIN_LEVEL << " and " << maxLevel << std::endl;
    return getNumberInRange(MIN_LEVEL, maxLevel);
}

std::vector<Player> getAllPlayers(const Player& player)
{
    std::vector<Player> allPlayers;
    char* namesFilePath = getPlayerNamesFilePath();

    std::ifstream finPlayerNames(namesFilePath);
    delete[] namesFilePath;

    if (!finPlayerNames.is_open())
    {
        return allPlayers;
    }

    char playerNameToLower[NAME_MAX_LENGTH];
    strToLower(player.name, playerNameToLower);
    char name[NAME_MAX_LENGTH];

    while (finPlayerNames.getline(name, NAME_MAX_LENGTH))
    {
        if (strCompare(playerNameToLower, name) == 0)
        {
            allPlayers.push_back(player);
            continue;
        }

        char* playerFilePath = getPlayerFilePath(name);
        std::ifstream finPlayer(playerFilePath);
        delete[] playerFilePath;

        Player currPlayer = {};
        if (!readPlayerInfo(finPlayer, currPlayer))
        {
            finPlayerNames.close();
            return allPlayers;
        }

        finPlayer.close();
        allPlayers.push_back(currPlayer);
    }

    finPlayerNames.close();
    return allPlayers;
}

int enemyMovesPerPlayerMove(const Game& game)
{
    if (game.level == MAX_LEVEL)
    {
        return 2;
    }

    return 1;
}

Game setUpGame(Player& player)
{
    int level = getGameLevel(player);
    Game& savedGame = player.savedGamesPerLevel[level - 1];

    if (savedGame.map.matrix != nullptr)
    {
        bool continuePrevGame = inputYesNo("Would you like to continue from where you left off?");

        if (continuePrevGame)
        {
            return savedGame;
        }

        deleteMap(savedGame.map);
    }

    Game game = {};
    game.level = level;

    const int MAPS_COUNT = 2;

    char* filePath = getMapFilePath(game.level, MAPS_COUNT);
    std::ifstream mapFile(filePath);
    delete[] filePath;

    if (!mapFile.is_open())
    {
        return game;
    }

    readGame(game, mapFile);
    mapFile.close();
    return game;
}

void printMoveResult(MoveResult moveRes)
{
    switch (moveRes)
    {
    case ENEMY_ENCOUNTER:
        std::cout << "You were captured by enemy!" << std::endl;
        break;

    case WALL_HIT:
        std::cout << "Ouch! You hit a wall!" << std::endl;
        break;

    case COIN_COLLECTED:
        std::cout << "You collected a coin!" << std::endl;
        break;

    case KEY_FOUND:
        std::cout << "You found the key! Now find the treasure!" << std::endl;
        break;

    case TELEPORTATION:
        std::cout << "Whoosh! You teleported successfully!" << std::endl;
        break;

    case TREASURE_WITHOUT_KEY:
        std::cout << "You need a key to open the treasure!" << std::endl;
        break;

    case TREASURE_WITH_KEY:
        std::cout << "Congratulations! You win!" << std::endl;
        break;
    }
}

void lossUpdateAndPrint(Player& player, MoveResult moveRes)
{
    lossUpdate(player);
    printMoveResult(moveRes);
    std::cout << "You lose! Better luck next game!" << std::endl;
}

void playGame(Game& game, Player& player)
{
    if (game.map.matrix == nullptr)
    {
        return;
    }

    clearConsole();
    char playerMove;
    MoveResult moveRes = NONE;

    int capacity = (game.map.rowsCount * game.map.colsCount) / 2;
    std::vector<VisitedCell> visitedCells;
    visitedCells.reserve(capacity);

    int enemyMoves = enemyMovesPerPlayerMove(game);

    while (true)
    {
        printGameInfo(game, player);
        printMatrix(game.map, GREEN_COLOR, RED_COLOR);
        printMoveResult(moveRes);
        printRulesToMove();

        std::cin >> playerMove;
        clearConsole();

        if (toLower(playerMove) == QUIT)
        {
            player.savedGamesPerLevel[game.level - 1] = game;
            return;
        }

        moveRes = move(player, game, playerMove);
        if (winCondition(moveRes))
        {
            winUpdate(game, player);
            printMoveResult(moveRes);
            break;
        }
        if (lossCondition(player))
        {
            lossUpdateAndPrint(player, moveRes);
            break;
        }
        if (moveRes == INVALID_MOVE)
        {
            continue;
        }

        game.map.enemyPosition = findShortestPath(game.map, visitedCells, enemyMoves);

        if (isSamePosition(game.map.playerPosition, game.map.enemyPosition))
        {
            lossUpdateAndPrint(player, ENEMY_ENCOUNTER);
            break;
        }
    }

    deleteMap(game.map);
    player.savedGamesPerLevel[game.level - 1].map.matrix = nullptr;
}

void deleteSavedGames(Player& player)
{
    for (size_t i = 0; i < player.level; i++)
    {
        Game& savedGame = player.savedGamesPerLevel[i];

        if (savedGame.map.matrix == nullptr)
        {
            continue;
        }

        deleteMap(savedGame.map);
    }
}

void logIn(Player& player)
{
    while (!getPlayerByName(player.name, player))
    {
        std::cout << "Name does not exist!" << std::endl;
        enterUsername(player);
    }
}

void signUp(Player& player)
{
    while (!appendPlayerNameToFile(player.name))
    {
        std::cout << "Name already exists!" << std::endl;
        enterUsername(player);
    }

    savePlayerProgress(player);
}

void showLeaderboard(const Player& player)
{
    clearConsole();
    size_t playerRank = 1;

    std::vector<Player> allPlayers = getAllPlayers(player);
    size_t playersCount = allPlayers.size();
    sortPlayers(allPlayers, 0, playersCount - 1);

    for (size_t i = 0; i < playersCount; i++)
    {
        size_t currRank = i + 1;
        if (strCompare(player.name, allPlayers[i].name) == 0)
        {
            playerRank = currRank;
        }

        std::cout << currRank << ". ";
        printPlayerInfo(allPlayers[i]);
    }

    std::cout << "You are number " << playerRank << " in the leaderboard" << std::endl;
}

void buyLives(Player& player)
{
    clearConsole();

    const int returnToMenu = 0;
    const int maxLivesToBuy = 100;

    int initialLives = player.lives;
    int inputNum;

    while (true)
    {
        std::cout << "One life costs " << LIFE_PRICE << " coins" << std::endl;
        std::cout << "You have " << player.coins << " coins." << std::endl;
        std::cout << "Enter the number of lives you want to buy or " << returnToMenu << " to return to menu:" << std::endl;

        inputNum = getNumberInRange(returnToMenu, maxLivesToBuy);
        clearConsole();

        if (inputNum == returnToMenu)
        {
            break;
        }

        int cost = inputNum * LIFE_PRICE;

        if (player.coins < cost)
        {
            std::cout << "Not enough coins! You need " << cost << " coins to buy " << inputNum << " lives!" << std::endl;
            continue;
        }

        std::cout << "You are about to buy " << inputNum << " lives for " << cost << " coins" << std::endl;
        bool acceptToBuy = inputYesNo("Are you sure you want to make this purchase?");
        clearConsole();

        if (acceptToBuy)
        {
            player.lives += inputNum;
            player.coins -= cost;
            break;
        }
    }

    if (player.lives > initialLives)
    {
        std::cout << "You successfully bought " << player.lives - initialLives << " lives" << std::endl;
    }
}

void displayPlayerInfo(const Player& player)
{
    clearConsole();

    std::cout << "Name: " << player.name << std::endl;
    std::cout << "Level: " << player.level << std::endl;
    std::cout << "Lives: " << player.lives << std::endl;
    std::cout << "Coins: " << player.coins << std::endl;
}

void exit(Player& player)
{
    savePlayerProgress(player);
    deleteSavedGames(player);
}

int displayMenuOptions()
{
    int optionsCount = 0;

    std::cout << "Please enter one of the numbers below to choose an option:" << std::endl;
    std::cout << ++optionsCount << ") " << "Play a game" << std::endl;
    std::cout << ++optionsCount << ") " << "Buy lives" << std::endl;
    std::cout << ++optionsCount << ") " << "View info" << std::endl;
    std::cout << ++optionsCount << ") " << "View leaderboard" << std::endl;
    std::cout << ++optionsCount << ") " << "Sign out" << std::endl;
    std::cout << ++optionsCount << ") " << "Exit" << std::endl;

    return optionsCount;
}

void pressKeyToContinue()
{
    std::cout << "Press any key to return to menu" << std::endl;

    std::cin.get();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    clearConsole();
}

Player enterApp()
{
    Player player = {};
    int optionNum = getInputOption();

    std::cin.ignore();
    enterUsername(player);

    if (optionNum == 1)
    {
        logIn(player);
    }
    else if (optionNum == 2)
    {
        signUp(player);
    }

    clearConsole();
    std::cout << "Welcome " << player.name << std::endl;

    return player;
}

void signOut(Player& player)
{
    exit(player);
    clearConsole();
    std::cout << "You successfully signed out." << std::endl;
    player = enterApp();
}

bool selectMenuOption(Player& player)
{
    int optionsCount = displayMenuOptions();
    int selectedOption = getNumberInRange(1, optionsCount);

    switch (selectedOption)
    {
    case 1:
    {
        Game game = setUpGame(player);
        playGame(game, player);
    }
    break;

    case 2:
        buyLives(player);
        break;

    case 3:
        displayPlayerInfo(player);
        pressKeyToContinue();
        break;

    case 4:
        showLeaderboard(player);
        pressKeyToContinue();
        break;

    case 5:
        signOut(player);
        break;

    case 6:
        exit(player);
        return false;
    }

    return true;
}

void run()
{
    initRandom();
    Player player = enterApp();
    while (selectMenuOption(player));
}

int main()
{
    run();

    return 0;
}