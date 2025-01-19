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

const char UP = 'w';
const char DOWN = 's';
const char LEFT = 'a';
const char RIGHT = 'd';

const int MIN_LEVEL = 1;
const int MAX_LEVEL = 3;
const int DEFAULT_LIVES = 3;
const int NAME_MIN_LENGTH = 2;
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
    Game savedGamesPerLevel[MAX_LEVEL] = {};
};

void clearConsole()
{
    std::cout << "\033[;H"; // Moves cursor to the top left
    std::cout << "\033[J"; // Clears the console
}

bool isInRange(int value, int from, int to)
{
    return value >= from
        && value <= to;
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

bool isValidIdx(int idx, size_t arrLen)
{
    return idx >= 0 && idx < arrLen;
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

    while(idx >= 0)
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

        deleteMatrix(savedGame.map.matrix, savedGame.map.rowsCount);
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