#include <iostream>
#include <stdint.h>
#include <vector>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include <string>

#define RIGHT 0
#define DOWN 256
#define LEFT 512
#define UP 768

using namespace std;

        // Pawn move representation
        // +--------+--------+--------+--------+--------+--------+--------+--------+
        // |    0   |    0   |   verticalMov   |vertPos | horPos |  horizontalMov  |
        // +--------+--------+--------+--------+--------+--------+--------+--------+

        // Wall move representation
        // +--------+--------+--------+--------+--------+--------+--------+--------+
        // |    1   |  isHor |             i            |             j            |
        // +--------+--------+--------+--------+--------+--------+--------+--------+

class Board
{
    private:

    uint8_t whitePawn = 4;
    uint8_t blackPawn = 132;

    uint8_t whiteWalls = 10;
    uint8_t blackWalls = 10;

    bool wallsOnBoard[128] = {false};
    bool takenWallPlaces[128] = {false};
    bool walledOffCells[904] = {false};

    // === Not used yet ===
    vector<uint8_t> neighbouringWalls[128] = {{}};
    uint8_t wallsTouchingSide[128] = {0};

    vector<uint8_t> whitePath = {20, 36, 52, 68, 84, 100, 116, 132};
    vector<uint8_t> blackPath = {116, 100, 84, 68, 52, 36, 20, 4};

    char winner = 0;

    bool dfs(bool player){
        uint8_t startCell = player ? whitePawn : blackPawn;
        uint8_t todo[81];
        size_t todoSize = 0;
        bool seen[137] = {false};

        todo[todoSize] = startCell;
        todoSize++;
        seen[startCell] = true;
        uint8_t curCell;
        uint8_t neighbourCell;
        bool foundRoute = false;

        while (todoSize > 0){
            curCell = todo[todoSize - 1];
            todoSize--;
            foundRoute = (player && curCell >= 128) || (!player && curCell <= 8);

            if (foundRoute){
                break;
            }

            // This is ugly, but really good for performance...
            if((curCell & 0xf) != 8 && !walledOffCells[curCell + RIGHT]){
                neighbourCell = curCell + 1;
                if (!seen[neighbourCell]){
                    todo[todoSize] = neighbourCell;
                    todoSize++;
                    seen[neighbourCell] = true;
                }
            }

            if((curCell & 0xf) != 0 && !walledOffCells[curCell + LEFT]){
                neighbourCell = curCell - 1;
                if (!seen[neighbourCell]){
                    todo[todoSize] = neighbourCell;
                    todoSize++;
                    seen[neighbourCell] = true;
                }
            }

            if(((curCell & 0xf0) >> 4) != 8 && !walledOffCells[curCell + UP]){
                neighbourCell = curCell + 16;
                if (!seen[neighbourCell]){
                    todo[todoSize] = neighbourCell;
                    todoSize++;
                    seen[neighbourCell] = true;
                }
            }

            if(((curCell & 0xf0) >> 4) != 0 && !walledOffCells[curCell + DOWN]){
                neighbourCell = curCell - 16;
                if (!seen[neighbourCell]){
                    todo[todoSize] = neighbourCell;
                    todoSize++;
                    seen[neighbourCell] = true;
                }
            }
        }

        return foundRoute;
    }


    inline void updateWallsOnBoard(uint8_t wallPlacement){
        wallsOnBoard[wallPlacement] = true;
    }


    inline void updateTakenWallPlaces(uint8_t wallPlacement){
        takenWallPlaces[wallPlacement] = true;
        bool isHorizontal = wallPlacement & 0x40;

        if (isHorizontal){
            takenWallPlaces[wallPlacement - 0x40] = true;
            if((wallPlacement & 7) < 7){
                takenWallPlaces[wallPlacement + 1] = true;
            }
            if((wallPlacement & 7) > 0){
                takenWallPlaces[wallPlacement - 1] = true;
            }
        }

        if (!isHorizontal){
            takenWallPlaces[wallPlacement + 0x40] = true;
            if((wallPlacement & 56) >> 3 < 7){
                takenWallPlaces[wallPlacement + 8] = true;
            }
            if((wallPlacement & 56) >> 3 > 0){
                takenWallPlaces[wallPlacement - 8] = true;
            }
        }
    }


    inline void updateWalledOffCells(uint8_t wallPlacement){
        bool isHorizontal = wallPlacement & 0x40;
        uint8_t i = (wallPlacement & 56) >> 3;
        uint8_t j = wallPlacement & 7;

        if(isHorizontal){
            walledOffCells[16 * i + j + UP] = true;
            walledOffCells[16 * (i + 1) + j + DOWN] = true;
            walledOffCells[16 * i + j + 1 + UP] = true;
            walledOffCells[16 * (i + 1) + (j + 1) + DOWN] = true;
        }

        if(!isHorizontal){
            walledOffCells[16 * i + j + RIGHT] = true;
            walledOffCells[16 * i + j + 1 + LEFT] = true;
            walledOffCells[16 * (i + 1) + j + RIGHT] = true;
            walledOffCells[16 * (i + 1) + (j + 1) + LEFT] = true;
        }
    }


    inline void executeWallPlacement(uint8_t wallPlacement){
        updateWallsOnBoard(wallPlacement);
        updateTakenWallPlaces(wallPlacement);
        updateWalledOffCells(wallPlacement);
    }


    inline void executePawnMove(uint8_t move, uint8_t player){
        if(player){
            (move & 8) ? whitePawn += (move & 48) : whitePawn -= (move & 48);
            (move & 4) ? whitePawn += (move & 3) : whitePawn -= (move & 3);

            if(whitePawn > 127){
                winner = 'w';
            }
        }

        if(!player){
            (move & 8) ? blackPawn += (move & 48) : blackPawn -= (move & 48);
            (move & 4) ? blackPawn += (move & 3) : blackPawn -= (move & 3);

            if(blackPawn < 9){
                winner = 'b';
            }
        }
    }


    inline void getNeighbours(uint8_t cell, uint8_t* neighbours, size_t& neighbourCount){
        if((cell & 0xf) != 8 && !walledOffCells[cell + RIGHT]){
            neighbours[neighbourCount] = cell + 1;
            neighbourCount++;
        }

        if((cell & 0xf) != 0 && !walledOffCells[cell + LEFT]){
            neighbours[neighbourCount] = cell - 1;
            neighbourCount++;
        }

        if(((cell & 0xf0) >> 4) != 8 && !walledOffCells[cell + UP]){
            neighbours[neighbourCount] = cell + 16;
            neighbourCount++;
        }

        if(((cell & 0xf0) >> 4) != 0 && !walledOffCells[cell + DOWN]){
            neighbours[neighbourCount] = cell - 16;
            neighbourCount++;
        }
    }


    inline void generatePossiblePawnMoves(bool player, uint8_t* possibleMoves, size_t& moveCount){
        uint8_t iPlayer = ((player ? whitePawn : blackPawn) & 0xf0) >> 4;
        uint8_t jPlayer = (player ? whitePawn : blackPawn) & 0x0f; 
        uint8_t playerPawn = player ? whitePawn : blackPawn;
        uint8_t opponentPawn = player ? blackPawn : whitePawn;

        uint8_t neighbours[4];
        size_t neighbourCount = 0;
        getNeighbours(playerPawn, neighbours, neighbourCount);

        for(int ind=0; ind<neighbourCount; ind++){
            uint8_t neighbour = neighbours[ind];
            if (neighbour == 255){
                break;
            }
            uint8_t i = (neighbour & 0xf0) >> 4;
            uint8_t j = neighbour & 0x0f;

            if (neighbour != opponentPawn){
                uint8_t move = (i == iPlayer ? 0 : (i > iPlayer ? 24 : 16)) + (j == jPlayer ? 0 : (j > jPlayer ? 5 : 1));   // simple pawn move
                possibleMoves[moveCount] = move;
                moveCount++;
                continue;
            }

            if (i > iPlayer) {
                if(i < 8 && !walledOffCells[neighbour + UP]){
                    // hop up
                    possibleMoves[moveCount] = 40;
                    moveCount++;
                    continue;
                }
                    
                if (j > 0 && !walledOffCells[neighbour + LEFT]){
                    // hop up left
                    possibleMoves[moveCount] = 25;
                    moveCount++;
                }
                if (j < 8 && !walledOffCells[neighbour + RIGHT]){
                    // hop up right
                    possibleMoves[moveCount] = 29;
                    moveCount++;
                }
                continue;
            }

            if (i < iPlayer) {
                if(i > 0 && !walledOffCells[neighbour + DOWN]){
                    // hop down
                    possibleMoves[moveCount] = 32;
                    moveCount++;
                    continue;
                }
                    
                if (j > 0 && !walledOffCells[neighbour + LEFT]){
                    // hop down left
                    possibleMoves[moveCount] = 17;
                    moveCount++;
                }
                if (j < 8 && !walledOffCells[neighbour + RIGHT]){
                    // hop down right
                    possibleMoves[moveCount] = 21;
                    moveCount++;
                }
                continue;
            }

            if (j > jPlayer) {
                if(j < 8 && !walledOffCells[neighbour + RIGHT]){
                    // hop right
                    possibleMoves[moveCount] = 6;
                    moveCount++;
                    continue;
                }
                    
                if (i > 0 && !walledOffCells[neighbour + DOWN]){
                    // hop right down
                    possibleMoves[moveCount] = 21;
                    moveCount++;
                }
                if (i < 8 && !walledOffCells[neighbour + UP]){
                    // hop right up
                    possibleMoves[moveCount] = 29;
                    moveCount++;
                }
                continue;
            }

            if (j < jPlayer) {
                if(j > 0 && !walledOffCells[neighbour + LEFT]){
                    // hop left
                    possibleMoves[moveCount] = 2;
                    moveCount++;
                    continue;
                }
                
                if (i > 0 && !walledOffCells[neighbour + DOWN]){
                    // hop left down
                    possibleMoves[moveCount] = 17;
                    moveCount++;
                }
                if (i < 8 && !walledOffCells[neighbour + UP]){
                    // hop left up
                    possibleMoves[moveCount] = 25;
                    moveCount++;
                }
                continue;
            }
        }

    }


    inline bool isSafeWallPlacement(uint8_t wallPlacement){
        // TODO implement
        return false;
    }


    inline void generatePossibleWallPlacements(bool player, uint8_t* possibleMoves, size_t& moveCount, bool checkValidity=true){
        // check whether wall placements can be made
        if ((player && !whiteWalls) || (!player && !blackWalls)){
            return;
        }

        // generates wall placements
        for(uint8_t wallPlacement = 0; wallPlacement < 0x80; wallPlacement++){
            if (takenWallPlaces[wallPlacement]){
                continue;
            }

            if (isSafeWallPlacement(wallPlacement) || !checkValidity){
                possibleMoves[moveCount] = 128 + wallPlacement;
                moveCount++;
                continue;
            }

            if (isValidWallPlacement(wallPlacement)){
                possibleMoves[moveCount] = 128 + wallPlacement;
                moveCount++;
                continue;
            }
        }

    }


    inline bool checkMoveValidity(uint8_t move, bool player){
        size_t moveCount = 0;
        uint8_t possibleMoves[256];
        generatePossibleMoves(player, possibleMoves, moveCount);

        for (int i = 0; i < moveCount; i++){
            if (move == possibleMoves[i]){
                return true;
            }
        }
        return false;
    }


    public:
    

    inline bool isValidWallPlacement(uint8_t wallPlacement){
        bool isHorizontal = wallPlacement & 0x40;
        uint8_t i = (wallPlacement & 56) >> 3;
        uint8_t j = wallPlacement & 7;

        // add blockades
        if(isHorizontal){
            walledOffCells[16 * i + j + UP] = true;
            walledOffCells[16 * (i + 1) + j + DOWN] = true;
            walledOffCells[16 * i + j + 1 + UP] = true;
            walledOffCells[16 * (i + 1) + (j + 1) + DOWN] = true;
        }

        if(!isHorizontal){
            walledOffCells[16 * i + j + RIGHT] = true;
            walledOffCells[16 * i + j + 1 + LEFT] = true;
            walledOffCells[16 * (i + 1) + j + RIGHT] = true;
            walledOffCells[16 * (i + 1) + (j + 1) + LEFT] = true;
        }

        bool validity = dfs(true) && dfs(false);

        // remove blockades
        if(isHorizontal){
            walledOffCells[16 * i + j + UP] = false;
            walledOffCells[16 * (i + 1) + j + DOWN] = false;
            walledOffCells[16 * i + j + 1 + UP] = false;
            walledOffCells[16 * (i + 1) + (j + 1) + DOWN] = false;
        }

        if(!isHorizontal){
            walledOffCells[16 * i + j + RIGHT] = false;
            walledOffCells[16 * i + j + 1 + LEFT] = false;
            walledOffCells[16 * (i + 1) + j + RIGHT] = false;
            walledOffCells[16 * (i + 1) + (j + 1) + LEFT] = false;
        }
        return validity;
    }


    /**
     * Execute a valid move
     * 
     * @param move Valid move   ASSUMPTION: move is valid
     * @param player true: white, false: black
     * @return void
     */
    void executeMove(uint8_t move, bool player){
        if (move >> 7){
            uint8_t wallPlacement = move & 0b01111111;
            player ? whiteWalls-- : blackWalls--;
            executeWallPlacement(wallPlacement);
        }
        else{
            executePawnMove(move, player);
        }
        
        int iWhite = (whitePawn & 0xf0) >> 4;
        int jWhite = whitePawn & 0x0f;
        int iBlack = (blackPawn & 0xf0) >> 4;
        int jBlack = blackPawn & 0x0f;
        if (iWhite > 8 || iBlack > 8 || jWhite > 8 || jBlack > 8){
            cout << "Pawn Out Of Bound\n";
        }
    }
    

    /**
     * Generate possible move for given player
     * 
     * @param player true: white, false: black
     * @return possible moves in a vector
     */
    void generatePossibleMoves(bool player, uint8_t* possibleMoves, size_t& moveCount){        
        generatePossiblePawnMoves(player, possibleMoves, moveCount);
        generatePossibleWallPlacements(player, possibleMoves, moveCount);
    }


    void generatePossibleMovesUnchecked(bool player, uint8_t* possibleMoves, size_t& moveCount){        
        generatePossiblePawnMoves(player, possibleMoves, moveCount);
        generatePossibleWallPlacements(player, possibleMoves, moveCount, false);
    }


    /**
     * Evaluate position
     * 
     * @return evaluation value
     */
    float evaluate(){
        if(winner == 'w'){
            return 1000.0f;
        }

        if(winner == 'b'){
            return -1000.0f;
        }

        float whitePathLength = (float) whitePath.size();
        float blackPathLength = (float) blackPath.size();

        float whiteWallCount = (float) whiteWalls;
        float blackWallCount = (float) blackWalls;

        float numberOfWallsAheadWhite = 0.0f;
        int iWhite = (whitePawn & 0xf0) >> 4;
        for(int i = iWhite; i < 8; i++){
            for(int j = 0; j < 8; j++){
                if(wallsOnBoard[8 * i + j] || wallsOnBoard[64 + 8 * i + j]){
                    numberOfWallsAheadWhite++;
                }
            }
        }
        float numberOfWallsAheadBlack = 0.0f;
        int iBlack = (whitePawn & 0xf0) >> 4;
        for(int i = 0; i < iBlack; i++){
            for(int j = 0; j < 8; j++){
                if(wallsOnBoard[8 * i + j] || wallsOnBoard[64 + 8 * i + j]){
                    numberOfWallsAheadBlack++;
                }
            }
        }
        
        uint8_t whiteNeighbours[4];
        size_t whiteNeighbourCount = 0;
        getNeighbours(whitePawn, whiteNeighbours, whiteNeighbourCount);
        
        uint8_t blackNeighbours[4];
        size_t blackNeighbourCount = 0;
        getNeighbours(blackPawn, blackNeighbours, blackNeighbourCount);
        
        float numberOfWhiteNeighbours = (float) whiteNeighbourCount;
        float numberOfBlackNeighbours = (float) blackNeighbourCount;

        float heuristics[8] = {whitePathLength, blackPathLength, whiteWallCount, blackWallCount, numberOfWallsAheadWhite, numberOfWallsAheadBlack, numberOfWhiteNeighbours, numberOfBlackNeighbours};
        float weights[8] = {-1.0f, 1.0f, 0.5f, -0.5f, -0.2f, 0.2f, 0.1f, -0.1f};

        float value = 0.0f;
        for(int i = 0; i < 8; i++){
            value += weights[i] * heuristics[i];
        }

        return value;
    }


    /**
     * Place a wall if move is valid
     * 
     * @param i vertical coordinate
     * @param j horizontal coordinate
     * @param horizontal true: wall direction is horizontal, false: wall direction vertical
     * @param player true: white, false: black
     * @return true: wall placement is valid and executed, false: invalid wall placement - move not executed
     */
    bool placeWall(uint8_t i, uint8_t j, bool horizontal, bool player){
        uint8_t move = 128 + (horizontal ? 64 : 0) + (i << 3) + j;
        if (checkMoveValidity(move, player)){
            executeMove(move, player);
            return true;
        }
        return false;
    }
    

    /**
     * Move pawn if move is valid
     * 
     * @param verticalMovement vertical movement
     * @param horizontalMovement horizontal movement
     * @param player true: white, false: black
     * @return true: pawn movement is valid and executed, false: invalid pawn movement - move not executed
     */
    bool movePawn(int verticalMovement, int horizontalMovement, bool player){
        uint8_t move = (((uint8_t) abs(verticalMovement)) << 4) + ((uint8_t) abs(horizontalMovement)) + (verticalMovement > 0 ? 8 : 0) + (horizontalMovement > 0 ? 4 : 0);
        if (checkMoveValidity(move, player)){
            executeMove(move, player);
            return true;
        }
        return false;
    }


    /**
     * Translate move into human readable form
     * 
     * @param move move encoded in a byte
     * @return move described in a string in the forms: Pawn(vert,hor) Wall(i-j) Wall(i|j)
     */
    static string translateMove(uint8_t move){
        string moveString = "";
        if(move >> 7){
            moveString.append("Wall(");
            moveString.append(to_string((move & 56) >> 3));
            (move & 64) ? moveString.append("-") : moveString.append("|");
            moveString.append(to_string(move & 7));
            moveString.append(")");

            return moveString;
        }

        moveString.append("Pawn(");
        int i = ((move & 48) >> 4) * (move & 8 ? 1 : -1);
        int j = (move & 3) * (move & 4 ? 1 : -1);
        moveString.append(to_string(i));
        moveString.append(",");
        moveString.append(to_string(j));
        moveString.append(")");

        return moveString;
    }


    Board& operator=(const Board& other) {
    if (this != &other) {
        this->whitePawn = other.whitePawn;
        this->blackPawn = other.blackPawn;

        for (int i = 0; i < 128; i++) {
            this->wallsOnBoard[i] = other.wallsOnBoard[i];
            this->takenWallPlaces[i] = other.takenWallPlaces[i];
        }

        for (int i = 0; i < 904; i++) {
            this->walledOffCells[i] = other.walledOffCells[i];
        }

        this->whiteWalls = other.whiteWalls;
        this->blackWalls = other.blackWalls;

        this->winner = other.winner;
    }
    return *this;
}


    char getWinner(){
        return this->winner;
    }


    bool whiteCloser(bool player){
        if(this->whitePath.size() > this->blackPath.size()){
            return false;
        }

        if(this->whitePath.size() < this->blackPath.size()){
            return true;
        }

        return player;
    }


    bool operator==(const Board& other){
        bool whitePawn = this->whitePawn == other.whitePawn;
        bool blackPawn = this->blackPawn == other.blackPawn;
        bool whiteWalls = this->whiteWalls == other.whiteWalls;
        bool blackWalls =  this->blackWalls == other.blackWalls;
        bool wallsOnBoard = equal(begin(this->wallsOnBoard), end(this->wallsOnBoard), begin(other.wallsOnBoard));
        bool takenWallPlaces = equal(begin(this->takenWallPlaces), end(this->takenWallPlaces), begin(other.takenWallPlaces));
        bool walledOffCells = equal(begin(this->walledOffCells), end(this->walledOffCells), begin(other.walledOffCells));
        bool whitePath = this->whitePath.size() == other.whitePath.size();
        bool blackPath = this->blackPath.size() == other.blackPath.size();
        bool winner = this->winner == other.winner;
        return whitePawn && blackPawn && whiteWalls && blackWalls && wallsOnBoard && takenWallPlaces && walledOffCells && whitePath && blackPath && winner;
    }


    Board(pair<int, int> whitePawn, pair<int, int> blackPawn, vector<pair<bool, pair<int, int>>> walls, int whiteWalls, int blackWalls){
        this->whitePawn = 16 * whitePawn.first + whitePawn.second;
        this->blackPawn = 16 * blackPawn.first + blackPawn.second;

        this->wallsOnBoard[128] = {false};
        this->takenWallPlaces[128] = {false};
        this->walledOffCells[904] = {false};
        
        for(int i = 0; i < walls.size(); i++){
            uint8_t move = 128 + (walls[i].first ? 64 : 0) + 8 * walls[i].second.first + walls[i].second.second;
            executeMove(move, true);
        }

        this->whiteWalls = whiteWalls;
        this->blackWalls = blackWalls;

        char winner = 0;
        if (whitePawn.first == 8){
            winner = 'w';
        }
        if (blackPawn.first == 0){
            winner = 'b';
        }

        this->winner = winner;
    }
    

    Board(Board& other){
        this->whitePawn = other.whitePawn;
        this->blackPawn = other.blackPawn;

        this->wallsOnBoard[128] = {false};
        this->takenWallPlaces[128] = {false};
        this->walledOffCells[904] = {false};
        
        for(int i = 0; i < 128; i++){
            this->wallsOnBoard[i] = other.wallsOnBoard[i];
            this->takenWallPlaces[i] = other.takenWallPlaces[i];
        }

        for(int i = 0; i < 904; i++){
            this->walledOffCells[i] = other.walledOffCells[i];
        }

        this->whiteWalls = whiteWalls;
        this->blackWalls = blackWalls;

        this->winner = other.winner;
    }


    Board(){
        this->whitePawn = 4;
        this->blackPawn = 132;

        this->wallsOnBoard[128] = {false};
        this->takenWallPlaces[128] = {false};
        this->walledOffCells[904] = {false};

        this->whitePath = {20, 36, 52, 68, 84, 100, 116, 132};
        this->blackPath = {116, 100, 84, 68, 52, 36, 20, 4};

        this->whiteWalls = 10;
        this->blackWalls = 10;

        char winner = 0;
    }
};

