import random
import MinesweeperGame
import copy
# Globals

ALPHABET = "abcdefghijklmnopqrstuvwxyz"
COVERED = " "
FLAG = "F"
class RegionWithMines:
    def __init__(self, tiles, numMines):
        '''A region known to have at least one mine
        '''
        self.tiles = tiles
        self.numMines = numMines


class MinesweeperSolverFirstOrder:
    '''class will reason through a game of minesweeper as far as it can, and report states in which it has to guess. each run generates
    a game log. It needs to be provided with the board dimensions (always square), and number of mines
    '''

    def __init__(self, boardSize, numMines, board):
        '''init for a minesweeper board, with board dimension and number of mines provided
        '''
        self.boardSize = boardSize
        self.numMines = numMines
        self.board = board

    def val(self, x, y, z) :
        if(x < 0 or y < 0 or x >= self.boardSize or y >= self.boardSize) :
            return False
        return self.board[x][y] == z
    
    def isNonZeroNumber(self, x, y) :
        if(self.val(x, y, FLAG) or self.val(x, y, COVERED) or self.val(x, y, '0') or x < 0 or y < 0 or x >= self.boardSize or y >= self.boardSize) :
            return False
        return True
    
    def adjacentTiles(self, x, y) :
        adjacentTiles = []
        if (x-1 >= 0 and y-1 >= 0) :
            adjacentTiles.append([x-1, y-1])
        if (x-1 >= 0) :
            adjacentTiles.append([x-1, y])
        if (x-1 >= 0 and y+1 <= self.boardSize) :
            adjacentTiles.append([x-1, y+1])
        if (y-1 >= 0) :
            adjacentTiles.append([x, y-1])
        if (y+1 <= self.boardSize) :
            adjacentTiles.append([x, y+1])
        if (x+1 <= self.boardSize and y-1 >= 0) :
            adjacentTiles.append([x+1, y-1])
        if (x+1 <= self.boardSize) :
            adjacentTiles.append([x+1, y])
        if (x+1 <= self.boardSize and y+1 <= self.boardSize) :
            adjacentTiles.append([x+1, y+1])
        return adjacentTiles
    
    def adjacentCovered(self, x, y) :
        adjacentCovered = []
        if self.val(x-1, y-1, COVERED) :
            adjacentCovered.append([x-1, y-1])
        if self.val(x-1, y, COVERED) :
            adjacentCovered.append([x-1, y])
        if self.val(x-1, y+1, COVERED) :
            adjacentCovered.append([x-1, y+1])
        if self.val(x, y-1, COVERED) :
            adjacentCovered.append([x, y-1])
        if self.val(x, y+1, COVERED) :
            adjacentCovered.append([x, y+1])
        if self.val(x+1, y-1, COVERED) :
            adjacentCovered.append([x+1, y-1])
        if self.val(x+1, y, COVERED) :
            adjacentCovered.append([x+1, y])
        if self.val(x+1, y+1, COVERED) :
            adjacentCovered.append([x+1, y+1])
        return adjacentCovered
    
    def adjacentFlagged(self, x, y) :
        adjacentFlagged = []
        if self.val(x-1, y-1, FLAG) :
            adjacentFlagged.append([x-1, y-1])
        if self.val(x-1, y, FLAG) :
            adjacentFlagged.append([x-1, y])
        if self.val(x-1, y+1, FLAG) :
            adjacentFlagged.append([x-1, y+1])
        if self.val(x, y-1, FLAG) :
            adjacentFlagged.append([x, y-1])
        if self.val(x, y+1, FLAG) :
            adjacentFlagged.append([x, y+1])
        if self.val(x+1, y-1, FLAG) :
            adjacentFlagged.append([x+1, y-1])
        if self.val(x+1, y, FLAG) :
            adjacentFlagged.append([x+1, y])
        if self.val(x+1, y+1, FLAG) :
            adjacentFlagged.append([x+1, y+1])
        return adjacentFlagged

    def isNumberAndTouchingAllMines(self, x, y) :
        if(not self.isNonZeroNumber(x, y)) :
            return False
        adjacentFlagged = self.adjacentFlagged(x,y)
        return self.val(x,y, str(len(adjacentFlagged)))
    
    def mineNumberDifferential(self, x, y) :
        if(not self.isNonZeroNumber(x, y)) :
            return -1
        adjacentFlagged = self.adjacentFlagged(x,y)
        return int(self.board[x][y]) - len(adjacentFlagged)

    def adjacentSafe(self, x, y) :
        if(self.isNumberAndTouchingAllMines(x, y) and len(self.adjacentCovered(x, y)) > 0) :
            return self.adjacentCovered(x, y)
        return []
        
    def cantBeMinesIfIsMine(self, x, y) :
        result = []
        if(not self.val(x,y, COVERED)):
            return []
        for tile in self.adjacentTiles(x, y) :
            tX = tile[0]
            tY = tile[1]
            if(self.mineNumberDifferential(tX, tY) == 1) :
                for tile in self.adjacentCovered(tX, tY) :
                    if(tile[0] != x or tile[1] != y) :
                        result.append([tile[0], tile[1]])
        return result
    
    def makesSomeTileUnsatisfiable(self, x, y) :
        preventsFromBeingMinesIfIsMine = self.cantBeMinesIfIsMine(x, y)
        if(len(preventsFromBeingMinesIfIsMine) == 0) :
            return False
        for i in range(x - 3, x + 4) :
            for j in range(y - 3, y + 4) :
                if(self.isNonZeroNumber(i, j)) :
                    adjacentCoveredOfImpacted = self.adjacentCovered(i, j)
                    if(len(adjacentCoveredOfImpacted) > 0) :
                        for tile in preventsFromBeingMinesIfIsMine :
                            if tile in adjacentCoveredOfImpacted:
                                adjacentCoveredOfImpacted.remove(tile)
                        if(len(adjacentCoveredOfImpacted) < self.mineNumberDifferential(i, j)) :
                            return True
        return False
    
    def getDisjointRegionAndMinesPresentMap(self) :
        result = []
        for x in range(self.boardSize) :
            for y in range(self.boardSize) :
                if(self.isNonZeroNumber(x, y)) :
                    adjacentCovered = self.adjacentCovered(x,y)
                    if len(adjacentCovered) :
                        if len(result) :
                            sharedTile = False
                            for region in result :
                                for covered in adjacentCovered :
                                    if covered in region.tiles:
                                        sharedTile = True
                                        break
                            if not sharedTile :
                                result.append(RegionWithMines(adjacentCovered, self.mineNumberDifferential(x, y)))
                        else :
                            result.append(RegionWithMines(adjacentCovered, self.mineNumberDifferential(x, y)))
        return result
    
    def isSafeDueToMineLimit(self, x, y, mineLimit, numFlags) :
        regionsWithMines = self.getDisjointRegionAndMinesPresentMap()
        numMinesToSatisfyKnownTiles = sum(region.numMines for region in regionsWithMines)
        allTilesInRegionsWithMines = []
        for region in regionsWithMines :
            allTilesInRegionsWithMines = allTilesInRegionsWithMines + region.tiles
        if([x,y] not in allTilesInRegionsWithMines and numFlags + numMinesToSatisfyKnownTiles == mineLimit) :
            return True
        return False

    def getTileToGuess(self) :
        regionsWithMines = self.getDisjointRegionAndMinesPresentMap()
        allTilesInRegionsWithMines = []
        for region in regionsWithMines :
            allTilesInRegionsWithMines = allTilesInRegionsWithMines + region.tiles
        tilesInUnknownRegions = []
        for x in range(self.boardSize) :
            for y in range(self.boardSize) :
                if self.val(x, y, COVERED) and [x,y] not in allTilesInRegionsWithMines :
                    tilesInUnknownRegions.append([x, y])
        if(len(tilesInUnknownRegions) > 0) :
            val = tilesInUnknownRegions[random.randrange(len(tilesInUnknownRegions))]
            return ALPHABET[val[1]] + str(val[0] + 1)
        # no tile present outside of regions known to containmines, pick a random tile
        val = allTilesInRegionsWithMines[random.randrange(len(allTilesInRegionsWithMines))]
        return ALPHABET[val[1]] + str(val[0] + 1)
    
    def RowString(self, row) :
        result = ""
        for c in row:
            result = result + "[" + c + "]"
        return result
    
    def FlagConfirmedMines(self, safeMoves) :
        for i in range(self.boardSize) :
                for j in range(self.boardSize) :
                    if(self.isNonZeroNumber(i, j)) :
                        adjCov = len(self.adjacentCovered(i, j))
                        adjFla = len(self.adjacentFlagged(i, j))
                        if(adjCov + adjFla == int(self.board[i][j])) :
                            for val in self.adjacentCovered(i, j) :
                                safeMoves.add(ALPHABET[val[1]] + str(val[0] + 1) + 'f')
        return safeMoves
    
    def ApplyAndClearMoves(self, safeMoves, gameData) :
        for move in safeMoves :
            MinesweeperGame.makeMoveForSolver(gameData, move)
            self.board = gameData.gameCurrentGrid
        safeMoves.clear()


    def LogBoard(self, gamelog) :
        for row in self.board:
            gamelog.write(self.RowString(row) + "\n")

    def LogGivenBoard(self, gamelog, board) :
        for row in board:
            gamelog.write(self.RowString(row) + "\n")

    def LogRevealed(self, gamelog, gameData) :
        for row in gameData.gameGrid:
            gamelog.write(self.RowString(row) + "\n")

    def PlayAndSolve(self,  gamelog, wins, parityBoards, boardSize = 12, numMines = 20) :

        #initialize board and make first move, first move is always safe
        safeMoves  = set()
        gameData = MinesweeperGame.GameData()
        self.boardSize = boardSize
        self.numMines = numMines
        MinesweeperGame.startGameForSolver(gameData, self.boardSize, self.numMines)
        MinesweeperGame.makeMoveForSolver(gameData, ALPHABET[int(self.boardSize/2) - 1] + str(int(self.boardSize/2)))
        self.board = gameData.gameCurrentGrid

        

        #main solver loop
        gamelog.write("Starting board after first move: \n")
        self.LogBoard(gamelog)
        round = 0
        while(gameData.gameState == 0) :
            gamelog.write("Round: " + str(round) + "\n")
            # save so that if we don't change the board this iteration, we report that we have reached a state where we cannot deduce a safe move
            startingBoard = copy.deepcopy(self.board)

            #place all possible flags
            safeMoves = self.FlagConfirmedMines(safeMoves)
            if(len(safeMoves) > 0) :
                gamelog.write("Mines flagged from a single number, and remaining safe moves or flags:\n")
                for move in safeMoves :
                    gamelog.write(move + "\n")
            if(len(gameData.gameFlagsList) == numMines) :
                gamelog.write("all mines flagged, flip over remaining covered spaces")
                for val in gameData.CoveredCells() :
                    safeMoves.add(ALPHABET[val[1]] + str(val[0] + 1))

            if(len(gameData.gameFlagsList) + len(gameData.CoveredCells()) == numMines) :
               gamelog.write("Remaining covered spaces equal to number of mines, flag all remaining spaces")
               for val in gameData.CoveredCells() :
                    safeMoves.add(ALPHABET[val[1]] + str(val[0] + 1) + "f")
            if(len(safeMoves) > 0) :
                self.ApplyAndClearMoves(safeMoves, gameData)
                gamelog.write("board after flagging mines:\n")
                self.LogBoard(gamelog)

            #make all confirmed safe moves after flags are placed
            for i in range(self.boardSize) :
                for j in range(self.boardSize) :
                    possibleMoves = self.adjacentSafe(i, j)
                    if(len(possibleMoves) > 0) :
                        for val in possibleMoves :
                            move = ALPHABET[val[1]] + str(val[0] + 1)
                            safeMoves.add(ALPHABET[val[1]] + str(val[0] + 1))
            if(len(safeMoves) > 0) :
                gamelog.write("Moves that can be taken after flagging mines:\n")
                safeMoves = safeMoves
                for move in safeMoves :
                    gamelog.write(move + "\n")
                self.ApplyAndClearMoves(safeMoves, gameData)
                gamelog.write("board after making safe moves:\n")
                self.LogBoard(gamelog)

            if(startingBoard == self.board) :
                #Check covered tiles to see if they can't be mines if we could not make any other plays
                for i in range(self.boardSize) :
                    for j in range(self.boardSize) :
                        if(self.val(i, j, COVERED)) :
                            if(self.makesSomeTileUnsatisfiable(i, j)) :
                                safeMoves.add(ALPHABET[j] + str(i + 1))
                if(len(safeMoves) > 0) :
                    gamelog.write("Tiles that cannot be mines due to making some number tile unsatisfiable:\n")
                    for move in safeMoves :
                        gamelog.write(move + "\n")
                    self.ApplyAndClearMoves(safeMoves, gameData)
                    gamelog.write("board after revealing thoise tiles:\n")
                    self.LogBoard(gamelog)
                
            if(startingBoard == self.board) :
                #Check number of flags and make moves based on satisfying remaining tiles with remaining flags available
                for i in range(self.boardSize) :
                    for j in range(self.boardSize) :
                        if(self.val(i, j, COVERED)) :
                            if(self.isSafeDueToMineLimit(i, j, gameData.gameNumMines, len(gameData.gameFlagsList))) :
                                safeMoves.add(ALPHABET[j] + str(i + 1))
                if(len(safeMoves) > 0) :
                    gamelog.write("Tiles that cannot be mines due to remaining available flags being needed in other board segments:\n")
                    for move in safeMoves :
                        gamelog.write(move + "\n")
                    self.ApplyAndClearMoves(safeMoves, gameData)
                    gamelog.write("board after revealing thoise tiles:\n")
                    self.LogBoard(gamelog)
            
            if(startingBoard == self.board and gameData.gameState != 1) :
                gamelog.write("Board in parity, safe move cannot be derived. Guessing a tile:\n")
                boardInParity = copy.deepcopy(self.board)
                parityBoards.append(boardInParity)
                safeMoves.add(self.getTileToGuess())
                for move in safeMoves :
                    gamelog.write(move + "\n")
                self.ApplyAndClearMoves(safeMoves, gameData)
            round += 1
        if(gameData.gameState == 2) :
            gamelog.write("Game was Lost, actual board:\n")
            self.LogRevealed(gamelog, gameData)
        if(gameData.gameState == 1) :
            gamelog.write("Game Won\n")
            wins = wins + 1
        return wins

solver = MinesweeperSolverFirstOrder(0,0,0)

gamelog = open("gamelog16-40.txt", "w")
gameStats = open("gameStats16-40.txt", "w")
parityBoards = []
wins = 0
games = 50
for i in range(games) :
 wins = solver.PlayAndSolve(gamelog, wins, parityBoards, 16,40)
gamelog.close()

gameStats.write("Won " + str (wins/games * 100) + " percent of games\n")
gameStats.write("Board where a safe tile had to be derived and agent had to guess\n")
for board in parityBoards :
    solver.LogGivenBoard(gameStats, board)
    gameStats.write("----------------------------------------\n")
gameStats.close

gamelog = open("gamelog12-20.txt", "w")
gameStats = open("gameStats12-20.txt", "w")
parityBoards = []
wins = 0
games = 50
for i in range(games) :
 wins = solver.PlayAndSolve(gamelog, wins, parityBoards)
gamelog.close()

gameStats.write("Won " + str (wins/games * 100) + " percent of games\n")
gameStats.write("Board where a safe tile had to be derived and agent had to guess\n")
for board in parityBoards :
    solver.LogGivenBoard(gameStats, board)
    gameStats.write("----------------------------------------\n")
gameStats.close