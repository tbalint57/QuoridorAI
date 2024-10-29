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

    vector<uint8_t> whitePath = {20, 36, 52, 68, 84, 100, 116, 132};
    vector<uint8_t> blackPath = {116, 100, 84, 68, 52, 36, 20, 4};

    char winner = 0;
    

    void addBlockades(uint8_t wallPlacement){
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
    

    void removeBlockades(uint8_t wallPlacement){
        bool isHorizontal = wallPlacement & 0x40;
        uint8_t i = (wallPlacement & 56) >> 3;
        uint8_t j = wallPlacement & 7;

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
    } 


    bool bfs(bool player, bool changeRoute){
        uint8_t startCell = player ? whitePawn : blackPawn;
        queue<uint8_t> todo = {};
        uint8_t parent[137] = {0};
        bool seen[137] = {false};

        todo.push(startCell);
        parent[startCell] = startCell;
        seen[startCell] = true;
        uint8_t curCell;
        bool foundRoute = false;

        while (todo.size() > 0){
            curCell = todo.front();
            foundRoute = (player && curCell >= 128) || (!player && curCell <= 8);

            if (foundRoute){
                break;
            }

            todo.pop();
            vector<uint8_t> neighbours = getNeighbours(curCell);

            for (uint8_t neighbour : neighbours){
                if(neighbour <= 136 && !seen[neighbour]){
                    todo.push(neighbour);
                    parent[neighbour] = curCell;
                    seen[neighbour] = true;
                }
            }
        }
        if (!changeRoute){
            return foundRoute;
        }

        // finish logic
        vector<uint8_t> newPath = {};
        while (curCell != startCell){
            newPath.push_back(curCell);
            curCell = parent[curCell];
        }
        reverse(newPath.begin(), newPath.end());

        player ? whitePath = newPath : blackPath = newPath;
        return foundRoute;
    }


    bool checkMoveValidity(uint8_t move, bool player){
        vector<uint8_t> possibleMoves = generatePossibleMoves(player);

        for (uint8_t possibleMove : possibleMoves){
            if (move == possibleMove){
                return true;
            }
        }
        return false;
    }
    

    vector<uint8_t> getNeighbours(uint8_t cell){
        vector<uint8_t> neighbours = {};

        if((cell & 0xf) != 8 && !walledOffCells[cell + RIGHT]){
            neighbours.push_back(cell + 1);
        }

        if((cell & 0xf) != 0 && !walledOffCells[cell + LEFT]){
            neighbours.push_back(cell - 1);
        }

        if(((cell & 0xf0) >> 4) != 8 && !walledOffCells[cell + UP]){
            neighbours.push_back(cell + 16);
        }

        if(((cell & 0xf0) >> 4) != 0 && !walledOffCells[cell + DOWN]){
            neighbours.push_back(cell - 16);
        }

        return neighbours;
    }


    void giveWallPlaces(uint8_t wallPlacement){
        takenWallPlaces[wallPlacement] = false;
        bool isHorizontal = wallPlacement & 0x40;

        if (isHorizontal){
            if(!wallsOnBoard[wallPlacement - 0x40 + 8] && !wallsOnBoard[wallPlacement - 0x40 - 8]){
                takenWallPlaces[wallPlacement - 0x40] = false;
            }
            if((wallPlacement & 7) < 7 && !wallsOnBoard[wallPlacement + 1 + 1]&& !wallsOnBoard[wallPlacement + 1 - 0x40]){
                takenWallPlaces[wallPlacement + 1] = false;
            }
            if((wallPlacement & 7&& !wallsOnBoard[wallPlacement - 1 - 1]&& !wallsOnBoard[wallPlacement - 1 - 0x40]) > 0){
                takenWallPlaces[wallPlacement - 1] = false;
            }
        }

        if (!isHorizontal){
            if(!wallsOnBoard[wallPlacement + 0x40 + 1] && !wallsOnBoard[wallPlacement + 0x40 - 1]){
                takenWallPlaces[wallPlacement + 0x40] = false;
            }
            if((wallPlacement & 56) >> 3 < 7 && !wallsOnBoard[wallPlacement + 8 + 8] && !wallsOnBoard[wallPlacement + 8 + 0x40]){
                takenWallPlaces[wallPlacement + 8] = false;
            }
            if((wallPlacement & 56) >> 3 > 0 && !wallsOnBoard[wallPlacement - 8 - 8] && !wallsOnBoard[wallPlacement - 8 + 0x40]){
                takenWallPlaces[wallPlacement - 8] = false;
            }
        }
    }
    

    void takeWallPlaces(uint8_t wallPlacement){
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
            if((wallPlacement & 56) >> 3 < 7){  // fixed logic (added >> 3)
                takenWallPlaces[wallPlacement + 8] = true;
            }
            if((wallPlacement & 56) >> 3 > 0){  // fixed logic (added >> 3)
                takenWallPlaces[wallPlacement - 8] = true;
            }
        }
    }


    public:

    /**
     * Execute a valid move
     * 
     * @param move Valid move   ASSUMPTION: move is valid
     * @param player true: white, false: black
     * @return void
     */
    void executeMove(uint8_t move, bool player){
        if (move >> 7){
            uint8_t wallPlacement = move & 0x7f;

            player ? whiteWalls-- : blackWalls--;

            wallsOnBoard[wallPlacement] = true;
            takeWallPlaces(wallPlacement);
            addBlockades(wallPlacement);
            bfs(player, true);
            bfs(!player, true);
            return;
        }

        if (player) {
            (move & 8) ? whitePawn += (move & 48) : whitePawn -= (move & 48);
            (move & 4) ? whitePawn += (move & 3) : whitePawn -= (move & 3);

            if (whitePawn == whitePath[0]){
                whitePath.erase(whitePath.begin());
                return;
            }

            if (whitePawn == whitePath[1]){
                whitePath.erase(whitePath.begin(), whitePath.begin() + 1);
                return;
            }

            bfs(player, true);
            return;
        }

        if (!player) {
            (move & 8) ? blackPawn += (move & 48) : blackPawn -= (move & 48);
            (move & 4) ? blackPawn += (move & 3) : blackPawn -= (move & 3);

            if (blackPawn == blackPath[0]){
                blackPath.erase(blackPath.begin());
                return;
            }

            if (blackPawn == blackPath[1]){
                blackPath.erase(blackPath.begin(), blackPath.begin() + 1);
                return;
            }

            bfs(player, true);
            return;
        }
    }
    

    /**
     * Undo last move
     * 
     * @param move Last move   ASSUMPTION: move was the last move
     * @param player true: white, false: black
     * @return void
     */
    void undoMove(uint8_t move, bool player){
        if (move >> 7){
            uint8_t wallPlacement = move & 0x7f;

            player ? whiteWalls++ : blackWalls++;

            wallsOnBoard[wallPlacement] = false;
            giveWallPlaces(wallPlacement);
            removeBlockades(wallPlacement);
            bfs(player, true);
            bfs(!player, true);
            return;
        }

        if (player) {
            (move & 8) ? whitePawn -= (move & 48) : whitePawn += (move & 48);
            (move & 4) ? whitePawn -= (move & 3) : whitePawn += (move & 3);
            bfs(player, true);
            return;
        }

        if (!player) {
            (move & 8) ? blackPawn -= (move & 48) : blackPawn += (move & 48);
            (move & 4) ? blackPawn -= (move & 3) : blackPawn += (move & 3);
            bfs(player, true);
            return;
        }
    }


    /**
     * Generate possible move for given player
     * 
     * @param player true: white, false: black
     * @return possible moves in a vector
     */
    vector<uint8_t> generatePossibleMoves(bool player){
        vector<uint8_t> possibleMoves = {};
        
        uint8_t iPlayer = ((player ? whitePawn : blackPawn) & 0xf0) >> 4;
        uint8_t jPlayer = (player ? whitePawn : blackPawn) & 0x0f; 
        uint8_t playerPawn = player ? whitePawn : blackPawn;
        uint8_t opponentPawn = player ? blackPawn : whitePawn;
        vector<uint8_t> neighbours = getNeighbours(playerPawn);

        // generate pawn moves
        for(uint8_t neighbour : neighbours){
            uint8_t i = (neighbour & 0xf0) >> 4;
            uint8_t j = neighbour & 0x0f;

            if (neighbour != opponentPawn){
                uint8_t move = (i == iPlayer ? 0 : (i > iPlayer ? 24 : 16)) + (j == jPlayer ? 0 : (j > jPlayer ? 5 : 1));   // simple pawn move
                possibleMoves.push_back(move);
                continue;
            }

            if (i > iPlayer) {
                if(i < 8 && !walledOffCells[neighbour + UP]){
                    // hop up
                    possibleMoves.push_back(40);
                    continue;
                }
                    
                if (j > 0 && !walledOffCells[neighbour + LEFT]){
                    // hop up left
                    possibleMoves.push_back(25);
                }
                if (j < 8 && !walledOffCells[neighbour + RIGHT]){
                    // hop up right
                    possibleMoves.push_back(29);
                }
                continue;
            }

            if (i < iPlayer) {
                if(i > 0 && !walledOffCells[neighbour + DOWN]){
                    // hop down
                    possibleMoves.push_back(32);
                    continue;
                }
                    
                if (j > 0 && !walledOffCells[neighbour + LEFT]){
                    // hop down left
                    possibleMoves.push_back(17);
                }
                if (j < 8 && !walledOffCells[neighbour + RIGHT]){
                    // hop down right
                    possibleMoves.push_back(21);
                }
                continue;
            }

            if (j > jPlayer) {
                if(j < 8 && !walledOffCells[neighbour + RIGHT]){
                    // hop right
                    possibleMoves.push_back(6);
                    continue;
                }
                    
                if (i > 0 && !walledOffCells[neighbour + DOWN]){
                    // hop right down
                    possibleMoves.push_back(21);
                }
                if (i < 8 && !walledOffCells[neighbour + UP]){
                    // hop right up
                    possibleMoves.push_back(29);
                }
                continue;
            }

            if (j < jPlayer) {
                if(j > 0 && !walledOffCells[neighbour + LEFT]){
                    // hop left
                    possibleMoves.push_back(2);
                    continue;
                }
                
                if (i > 0 && !walledOffCells[neighbour + DOWN]){
                    // hop left down
                    possibleMoves.push_back(17);
                }
                if (i < 8 && !walledOffCells[neighbour + UP]){
                    // hop left up
                    possibleMoves.push_back(25);
                }
                continue;
            }
        }

        // check whether wall placements can be made
        if ((player && !whiteWalls) || (!player && !blackWalls)){
            return possibleMoves;
        }

        // generates wall placements
        for(uint8_t wallPlacement = 0; wallPlacement < 0x80; wallPlacement++){
            if (takenWallPlaces[wallPlacement]){
                continue;
            }

            addBlockades(wallPlacement);
            if(bfs(true, false) && bfs(false, false)){
                possibleMoves.push_back(128 + wallPlacement);
            }
            removeBlockades(wallPlacement);
        }

        return possibleMoves;
    }


    /**
     * Evaluate position
     * 
     * @return evaluation value
     */
    float evaluate(){
        // Implement when necessary
        return -1;
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
};