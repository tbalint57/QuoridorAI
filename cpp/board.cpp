#include <iostream>
#include <stdint.h>
#include <vector>
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

    friend struct BoardHasher;

    char winner = 0;

    bool dfs(bool player){
        uint8_t startCell = player ? whitePawn : blackPawn;
        // write in that this is fatser than vector
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


    int bfs(bool player){
        uint8_t startCell = player ? whitePawn : blackPawn;
        uint8_t todo[81] = {};
        int todoFront = 0;
        int todoBack = 0;

        bool seen[137] = {false};
        int depth[137] = {0};

        seen[startCell] = true;
        todo[todoBack] = startCell;
        todoBack++;

        uint8_t curCell;
        uint8_t neighbourCell;
        bool foundRoute = false;

        while (todoBack - todoFront > 0){
            curCell = todo[todoFront];
            foundRoute = (player && curCell >= 128) || (!player && curCell <= 8);

            if (foundRoute){
                break;
            }

            // This is ugly, copy-paste is king...
            if((curCell & 0xf) != 8 && !walledOffCells[curCell + RIGHT]){
                neighbourCell = curCell + 1;
                if (!seen[neighbourCell]){
                    todo[todoBack] = neighbourCell;
                    todoBack++;
                    seen[neighbourCell] = true;
                    depth[neighbourCell] = depth[curCell] + 1;
                }
            }

            if((curCell & 0xf) != 0 && !walledOffCells[curCell + LEFT]){
                neighbourCell = curCell - 1;
                if (!seen[neighbourCell]){
                    todo[todoBack] = neighbourCell;
                    todoBack++;
                    seen[neighbourCell] = true;
                    depth[neighbourCell] = depth[curCell] + 1;
                }
            }

            if(((curCell & 0xf0) >> 4) != 8 && !walledOffCells[curCell + UP]){
                neighbourCell = curCell + 16;
                if (!seen[neighbourCell]){
                    todo[todoBack] = neighbourCell;
                    todoBack++;
                    seen[neighbourCell] = true;
                    depth[neighbourCell] = depth[curCell] + 1;
                }
            }

            if(((curCell & 0xf0) >> 4) != 0 && !walledOffCells[curCell + DOWN]){
                neighbourCell = curCell - 16;
                if (!seen[neighbourCell]){
                    todo[todoBack] = neighbourCell;
                    todoBack++;
                    seen[neighbourCell] = true;
                    depth[neighbourCell] = depth[curCell] + 1;
                }
            }
            todoFront++;
        }
        return depth[curCell];
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


    inline void addWallIfPlacementValid(uint8_t wallPlacement, uint8_t* possibleMoves, size_t& moveCount, bool checkValidity=true){
        if (takenWallPlaces[wallPlacement]){
            return;
        }

        if (isSafeWallPlacement(wallPlacement) || !checkValidity){
            possibleMoves[moveCount] = 128 + wallPlacement;
            moveCount++;
            return;
        }

        if (isValidWallPlacement(wallPlacement)){
            possibleMoves[moveCount] = 128 + wallPlacement;
            moveCount++;
            return;
        }
    }


    inline void generatePossibleWallPlacements(bool player, uint8_t* possibleMoves, size_t& moveCount, bool checkValidity=true){
        // check whether wall placements can be made
        if ((player && !whiteWalls) || (!player && !blackWalls)){
            return;
        }

        // generates wall placements
        for(uint8_t wallPlacement = 0; wallPlacement < 0x80; wallPlacement++){
            addWallIfPlacementValid(wallPlacement, possibleMoves, moveCount, checkValidity);
        }

    }


    void generateProbableWallPlacements(bool player, uint8_t* possibleMoves, size_t& moveCount, bool checkValidity=true){
        // check whether wall placements can be made
        if ((player && !whiteWalls) || (!player && !blackWalls)){
            return;
        }

        // we consider the following wall placements probable:
        // 1. Right next to opponent pawn
        // 2. Next to existing wall
        // 3. Behind player's pawn in opponent pawn's column

        uint8_t opponentPawn = player ? blackPawn : whitePawn;
        uint8_t iOpponentPawn = opponentPawn >> 4;
        uint8_t jOpponentPawn = opponentPawn & 0x0f;

        uint8_t playerPawn = player ? whitePawn : blackPawn;
        uint8_t iPlayerPawn = playerPawn >> 4;
        uint8_t jPlayerPawn = playerPawn & 0x0f;
        
        // 1.
        uint8_t wallPlacementsNextToOpponentPawn[8] = {
            (uint8_t)((iOpponentPawn * 8) + jOpponentPawn),
            (uint8_t)(iOpponentPawn * 8 + jOpponentPawn + 64),
            (uint8_t)(iOpponentPawn * 8 + (jOpponentPawn - 1)),
            (uint8_t)(iOpponentPawn * 8 + (jOpponentPawn - 1) + 64),
            (uint8_t)((iOpponentPawn - 1) * 8 + jOpponentPawn),
            (uint8_t)((iOpponentPawn - 1) * 8 + jOpponentPawn + 64),
            (uint8_t)((iOpponentPawn - 1) * 8 + (jOpponentPawn - 1)),
            (uint8_t)((iOpponentPawn - 1) * 8 + (jOpponentPawn - 1) + 64)
        };

        for(int i = 0; i < 8; i++){
            uint8_t wallPlacement = wallPlacementsNextToOpponentPawn[i];

            if (wallPlacement > 127){
                continue;
            }

            addWallIfPlacementValid(wallPlacement, possibleMoves, moveCount, checkValidity);
        }

        // 2.
        for(uint8_t wallPlacement = 0; wallPlacement < 0x80; wallPlacement++){
            if (hasNeighbouringWallPlacement(wallPlacement)){
                addWallIfPlacementValid(wallPlacement, possibleMoves, moveCount, checkValidity);
            }
        }

        // 3.
        int iBottom = player ? 0 : iPlayerPawn;
        int iTop = player ? iPlayerPawn : 8;
        for(int i = 0; i < iPlayerPawn; i++){
            uint8_t wallPlacement = 8 * i + jOpponentPawn + 64;
            addWallIfPlacementValid(wallPlacement, possibleMoves, moveCount, checkValidity);

            wallPlacement = 8 * i + jOpponentPawn + 64 - 1;
            addWallIfPlacementValid(wallPlacement, possibleMoves, moveCount, checkValidity);
        }


    }


    inline bool hasNeighbouringWallPlacement(uint8_t wallPlacement){
        bool isHorizontal = wallPlacement & 0b01000000;
        uint8_t neighbouringWallPlacements[8];

        if (isHorizontal) {
            neighbouringWallPlacements[0] = wallPlacement + 2;
            neighbouringWallPlacements[1] = wallPlacement - 2;
            neighbouringWallPlacements[2] = wallPlacement - 64 + 7;
            neighbouringWallPlacements[3] = wallPlacement - 64 + 8;
            neighbouringWallPlacements[4] = wallPlacement - 64 + 9;
            neighbouringWallPlacements[5] = wallPlacement - 64 - 7;
            neighbouringWallPlacements[6] = wallPlacement - 64 - 8;
            neighbouringWallPlacements[7] = wallPlacement - 64 - 9;
        } else {
            neighbouringWallPlacements[0] = wallPlacement + 16;
            neighbouringWallPlacements[1] = wallPlacement - 16;
            neighbouringWallPlacements[2] = wallPlacement + 64 - 7;
            neighbouringWallPlacements[3] = wallPlacement + 64 + 1;
            neighbouringWallPlacements[4] = wallPlacement + 64 + 9;
            neighbouringWallPlacements[5] = wallPlacement + 64 - 9;
            neighbouringWallPlacements[6] = wallPlacement + 64 - 1;
            neighbouringWallPlacements[7] = wallPlacement + 64 + 7;
        }

        for (int i = 0; i < 8; i++){
            uint8_t wallPlacement = neighbouringWallPlacements[i];

            if (wallPlacement < 128 && wallsOnBoard[wallPlacement]){
                return true;
            }
        }

        return false;
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


    inline float pawnHeuristic(uint8_t move, bool player){
        if(move < 128){
            return 100;
        }

        return 0;
    }


    inline float shortestPawnHeuristic(uint8_t move, bool player){
        uint8_t shortestPawnMove = generateMoveOnShortestPath(player);
        if(move == shortestPawnMove){
            return 100;
        }

        return 0;
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
    }
    

    /**
     * Generate possible move for given player
     * 
     * @param player true: white, false: black
     * @return possible moves
     */
    int generatePossibleMoves(bool player, uint8_t* possibleMoves, size_t& moveCount){        
        generatePossiblePawnMoves(player, possibleMoves, moveCount);
        int pawnMoves =  moveCount;
        generatePossibleWallPlacements(player, possibleMoves, moveCount);
        return pawnMoves;
    }


    /**
     * Generate possible move for given player, this may include invalid wall placements
     * 
     * @param player true: white, false: black
     * @return possible moves
     */
    int generatePossibleMovesUnchecked(bool player, uint8_t* possibleMoves, size_t& moveCount){        
        generatePossiblePawnMoves(player, possibleMoves, moveCount);
        int pawnMoves =  moveCount;
        generatePossibleWallPlacements(player, possibleMoves, moveCount, false);
        return pawnMoves;
    }


    /**
     * Generate probable move for given player, this may include invalid wall placements
     * 
     * @param player true: white, false: black
     * @return possible moves
     */
    int generateProbableMovesUnchecked(bool player, uint8_t* possibleMoves, size_t& moveCount){        
        generatePossiblePawnMoves(player, possibleMoves, moveCount);
        int pawnMoves =  moveCount;
        generateProbableWallPlacements(player, possibleMoves, moveCount, false);
        return pawnMoves;
    }


    uint8_t generateMoveOnShortestPath(bool player){
        uint8_t startCell = player ? whitePawn : blackPawn;
        uint8_t todo[81] = {};
        int todoFront = 0;
        int todoBack = 0;

        bool seen[137] = {false};
        int depth[137] = {0};
        uint8_t parent[137];

        seen[startCell] = true;
        parent[startCell] = startCell;
        todo[todoBack] = startCell;
        todoBack++;

        uint8_t curCell;
        uint8_t neighbourCell;
        bool foundRoute = false;

        while (todoBack - todoFront > 0){
            curCell = todo[todoFront];
            foundRoute = (player && curCell >= 128) || (!player && curCell <= 8);

            if (foundRoute){
                break;
            }

            // This is ugly, copy-paste is king...
            if((curCell & 0xf) != 8 && !walledOffCells[curCell + RIGHT]){
                neighbourCell = curCell + 1;
                if (!seen[neighbourCell]){
                    todo[todoBack] = neighbourCell;
                    todoBack++;
                    seen[neighbourCell] = true;
                    depth[neighbourCell] = depth[curCell] + 1;
                    parent[neighbourCell] = curCell;
                }

                if((neighbourCell > 127 && player) || (neighbourCell < 9 && !player)){
                    break;
                }
            }

            if((curCell & 0xf) != 0 && !walledOffCells[curCell + LEFT]){
                neighbourCell = curCell - 1;
                if (!seen[neighbourCell]){
                    todo[todoBack] = neighbourCell;
                    todoBack++;
                    seen[neighbourCell] = true;
                    depth[neighbourCell] = depth[curCell] + 1;
                    parent[neighbourCell] = curCell;
                }

                if((neighbourCell > 127 && player) || (neighbourCell < 9 && !player)){
                    break;
                }
            }

            if(((curCell & 0xf0) >> 4) != 8 && !walledOffCells[curCell + UP]){
                neighbourCell = curCell + 16;
                if (!seen[neighbourCell]){
                    todo[todoBack] = neighbourCell;
                    todoBack++;
                    seen[neighbourCell] = true;
                    depth[neighbourCell] = depth[curCell] + 1;
                    parent[neighbourCell] = curCell;
                }

                if((neighbourCell > 127 && player) || (neighbourCell < 9 && !player)){
                    break;
                }
            }

            if(((curCell & 0xf0) >> 4) != 0 && !walledOffCells[curCell + DOWN]){
                neighbourCell = curCell - 16;
                if (!seen[neighbourCell]){
                    todo[todoBack] = neighbourCell;
                    todoBack++;
                    seen[neighbourCell] = true;
                    depth[neighbourCell] = depth[curCell] + 1;
                    parent[neighbourCell] = curCell;
                }

                if((neighbourCell > 127 && player) || (neighbourCell < 9 && !player)){
                    break;
                }
            }
            todoFront++;
        }
        
        curCell = todo[todoBack - 1];
        uint8_t path[81];
        size_t pathSize = 0;

        while(parent[curCell] != curCell){
            path[pathSize] = curCell;
            pathSize++;
            curCell = parent[curCell];
        }

        uint8_t possibleMoves[5];
        size_t moveCount = 0;
        generatePossiblePawnMoves(player, possibleMoves, moveCount);

        uint8_t playerPawn = player ? whitePawn : blackPawn;
        uint8_t opponentPawn = player ? blackPawn : whitePawn;

        for(int i = 0; i < moveCount; i++){
            uint8_t move = possibleMoves[i];

            uint8_t nextCell = playerPawn;
            (move & 8) ? nextCell += (move & 48) : nextCell -= (move & 48);
            (move & 4) ? nextCell += (move & 3) : nextCell -= (move & 3);

            if(nextCell != opponentPawn && (nextCell == path[pathSize - 1] || nextCell == path[pathSize - 2])){
                return move;
            }
        }

        // It is possible that none of the hops are on the shortest path we found. In this case return a random hop.
        // Note: This is not always the best move, as it is somtimes not on any of the shortest paths.
        return possibleMoves[0];
    }


    /**
     * Evaluate position
     * 
     * @return evaluation value
     */
    inline float calculateHeuristicForMove(uint8_t move, bool player) {
        return shortestPawnHeuristic(move, player);
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
        int whitePathLength = bfs(true);
        int blackPathLength = bfs(false);
        if(whitePathLength > blackPathLength){
            return false;
        }

        if(whitePathLength < blackPathLength){
            return true;
        }

        return player;
    }


    void getSaveData(uint8_t* saveData){
        saveData[1] = whitePawn;
        saveData[2] = blackPawn;
        saveData[3] = whiteWalls;
        saveData[4] = blackWalls;

        uint8_t saveLength = 5;

        for(int wallPlacement = 0; wallPlacement < 128; wallPlacement++){
            if(wallsOnBoard[wallPlacement]){
                saveData[saveLength] = wallPlacement;
                saveLength++;
            }
        }

        saveData[0] = saveLength;
    }


    bool operator==(const Board& other) const {
        bool whitePawn = this->whitePawn == other.whitePawn;
        bool blackPawn = this->blackPawn == other.blackPawn;
        bool whiteWalls = this->whiteWalls == other.whiteWalls;
        bool blackWalls =  this->blackWalls == other.blackWalls;
        bool wallsOnBoard = equal(begin(this->wallsOnBoard), end(this->wallsOnBoard), begin(other.wallsOnBoard));
        bool takenWallPlaces = equal(begin(this->takenWallPlaces), end(this->takenWallPlaces), begin(other.takenWallPlaces));
        bool walledOffCells = equal(begin(this->walledOffCells), end(this->walledOffCells), begin(other.walledOffCells));
        bool winner = this->winner == other.winner;
        return whitePawn && blackPawn && whiteWalls && blackWalls && wallsOnBoard && takenWallPlaces && walledOffCells && winner;
    }


    void printState() {
        cout << "pawns: " << (int)whitePawn << " " << (int)blackPawn << "\n";
        cout << "walls: " << (int)whiteWalls << " " << (int)blackWalls << "\n";
        for(int i = 0; i < 128; i++){
            if (wallsOnBoard[i]){
                cout << i << " ";
            }
        }
        cout << "\n";
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
    

    Board(const Board& other){
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

        this->whiteWalls = other.whiteWalls;
        this->blackWalls = other.blackWalls;

        this->winner = other.winner;
    }


    Board(){
        this->whitePawn = 4;
        this->blackPawn = 132;

        this->wallsOnBoard[128] = {false};
        this->takenWallPlaces[128] = {false};
        this->walledOffCells[904] = {false};

        this->whiteWalls = 10;
        this->blackWalls = 10;

        char winner = 0;
    }


    Board(char whitePawn, char blackPawn, char whiteWalls, char blackWalls, char* placedWallsOnBoard, size_t numberOfWallsOnBoard){
        this->whitePawn = whitePawn;
        this->blackPawn = blackPawn;
        
        for(int i = 0; i < numberOfWallsOnBoard; i++){
            executeMove(128 + placedWallsOnBoard[i], true);
        }

        this->whiteWalls = whiteWalls;
        this->blackWalls = blackWalls;

        char winner = 0;
    }
};


#include <functional>


struct BoardHasher {
    size_t operator()(const Board& board) const {
        size_t hash = 0;

        // Combine all relevant pieces of board state into the hash
        hash ^= std::hash<uint8_t>{}(board.whitePawn) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= std::hash<uint8_t>{}(board.blackPawn) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= std::hash<uint8_t>{}(board.whiteWalls) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        hash ^= std::hash<uint8_t>{}(board.blackWalls) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

        for (int i = 0; i < 128; i++) {
            if (board.wallsOnBoard[i]) {
                hash ^= std::hash<int>{}(i) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
        }

        return hash;
    }
};
