'''
randothellogame module

sets up a RandOthello game closely following the book's framework for games

RandOthelloState is a class that will handle our state representation, then we've 
got stand-alone functions for player, actions, result and terminal_test

Differing from the book's framework, is that utility is *not* a stand-alone 
function, as each player might have their own separate way of calculating utility


'''
import random
import copy
import time

WHITE = 1
BLACK = -1
EMPTY = 0
BLOCKED = -2
SIZE = 8
SKIP = "SKIP"

class OthelloPlayerTemplate:
    '''Template class for an Othello Player

    An othello player *must* implement the following methods:

    get_color(self) - correctly returns the agent's color

    make_move(self, state) - given the state, returns an action that is the agent's move
    '''
    def __init__(self, mycolor):
        self.color = mycolor

    def get_color(self):
        return self.color

    def make_move(self, state):
        '''Given the state, returns a legal action for the agent to take in the state
        '''
        return None

class HumanPlayer(OthelloPlayerTemplate):
    def __init__(self, mycolor):
        self.color = mycolor

    def get_color(self):
        return self.color

    def make_move(self, state):
        curr_move = None
        legals = actions(state)
        while curr_move == None:
            display(state)
            if self.color == 1:
                print("White ", end='')
            else:
                print("Black ", end='')
            print(" to play.")
            print("Legal moves are " + str(legals))
            move = input("Enter your move as a r,c pair:")
            if move == "":
                return legals[0]

            if move == SKIP and SKIP in legals:
                return move

            try:
                movetup = int(move.split(',')[0]), int(move.split(',')[1])
            except:
                movetup = None
            if movetup in legals:
                curr_move = movetup
            else:
                print("That doesn't look like a legal action to me")
        return curr_move
    
class RandomPlayer(OthelloPlayerTemplate):
    def __init__(self, mycolor):
        self.color = mycolor

    def get_color(self):
        return self.color

    def make_move(self, state):
        legals = actions(state)
        move = random.randrange(0, len(legals))
        return legals[move]
    

class MinimaxPlayer(OthelloPlayerTemplate):
    def __init__(self, mycolor, depthLimit):
        self.color = mycolor
        self.limit = depthLimit

    def get_color(self):
        return self.color

    def make_move(self, state):
        move = self.minimax(state)
        return move
    
    def minimax(self, state):
        '''Minimax search, assumes player is always to-move
        '''
        val, move = self.max_value(state, 1)
        return move
    
    def max_value(self, state, depth):
        if(terminal_test(state) or depth >= self.limit) :
            return utility(state, self), EMPTY
        value = -9999999
        move = EMPTY
        for a in  actions(state) :
            newstate = result(state, a)
            val2, move2 = self.min_value(newstate, depth + 1)
            if(val2 > value) :
                value = val2
                move = a
        return value,move
        
    def min_value(self, state, depth):
        if(terminal_test(state) or depth >= self.limit) :
            return utility(state, self), EMPTY
        value = 9999999
        move = EMPTY
        for a in  actions(state) :
            newstate = result(state, a)
            val2, move2 = self.max_value(newstate, depth + 1)
            if(val2 < value) :
                value = val2
                move = a
        return value,move

class AlphabetaPlayer(OthelloPlayerTemplate):
    def __init__(self, mycolor, depthLimit):
        self.color = mycolor
        self.limit = depthLimit

    def get_color(self):
        return self.color

    def make_move(self, state):
        move = self.alpha_beta_search(state)
        return move
    
    def alpha_beta_search(self, state):
        '''alpha-beta search, assumes player is always to-move
        '''
        val, move = self.max_value_ab(state, 1, -9999999, 9999999)
        return move
    
    def max_value_ab(self, state, depth, alpha, beta):
        if(terminal_test(state) or depth >= self.limit) :
            return utility(state, self), EMPTY
        value = -9999999
        move = EMPTY
        for a in  actions(state) :
            newstate = result(state, a)
            val2, move2 = self.min_value_ab(newstate, depth + 1, alpha, beta)
            if(val2 > value) :
                value = val2
                move = a
                alpha = max(value, alpha)
            if(value >= beta) :
                return value, move
        return value,move
        
    def min_value_ab(self, state, depth, alpha, beta):
        if(terminal_test(state) or depth >= self.limit) :
            return utility(state, self), EMPTY
        value = 9999999
        move = EMPTY
        for a in  actions(state) :
            newstate = result(state, a)
            val2, move2 = self.max_value_ab(newstate, depth + 1, alpha, beta)
            if(val2 < value) :
                value = val2
                move = a
                beta = min(value, beta)
            if(value <= alpha) :
                return value, move
        return value,move

class RandOthelloState:
    '''A class to represent an othello game state'''

    def __init__(self, currentplayer, otherplayer, board_array = None, num_skips = 0):
        if board_array != None:
            self.board_array = board_array
        else:
            self.board_array = [[EMPTY] * SIZE for i in range(SIZE)]
            self.board_array[3][3] = WHITE
            self.board_array[4][4] = WHITE
            self.board_array[3][4] = BLACK
            self.board_array[4][3] = BLACK
            x1 = random.randrange(8)
            x2 = random.randrange(8)
            self.board_array[x1][0] = BLOCKED
            self.board_array[x2][7] = BLOCKED
        self.num_skips = num_skips
        self.current = currentplayer
        self.other = otherplayer


def player(state):
    return state.current

def actions(state):
    '''Return a list of possible actions given the current state
    '''
    legal_actions = []
    for i in range(SIZE):
        for j in range(SIZE):
            if result(state, (i,j)) != None:
                legal_actions.append((i,j))
    if len(legal_actions) == 0:
        legal_actions.append(SKIP)
    return legal_actions

def result(state, action):
    '''Returns the resulting state after taking the given action

    (This is the workhorse function for checking legal moves as well as making moves)

    If the given action is not legal, returns None

    '''
    # first, special case! an action of SKIP is allowed if the current agent has no legal moves
    # in this case, we just skip to the other player's turn but keep the same board
    if action == SKIP:
        newstate = RandOthelloState(state.other, state.current, copy.deepcopy(state.board_array), state.num_skips + 1)
        return newstate

    if state.board_array[action[0]][action[1]] != EMPTY:
        return None

    color = state.current.get_color()
    # create new state with players swapped and a copy of the current board
    newstate = RandOthelloState(state.other, state.current, copy.deepcopy(state.board_array))

    newstate.board_array[action[0]][action[1]] = color
    
    flipped = False
    directions = [(-1,-1), (-1,0), (-1,1), (0,-1), (0,1), (1,-1), (1,0), (1,1)]
    for d in directions:
        i = 1
        count = 0
        while i <= SIZE:
            x = action[0] + i * d[0]
            y = action[1] + i * d[1]
            if x < 0 or x >= SIZE or y < 0 or y >= SIZE:
                count = 0
                break
            elif newstate.board_array[x][y] == -1 * color:
                count += 1
            elif newstate.board_array[x][y] == color:
                break
            else:
                count = 0
                break
            i += 1

        if count > 0:
            flipped = True

        for i in range(count):
            x = action[0] + (i+1) * d[0]
            y = action[1] + (i+1) * d[1]
            newstate.board_array[x][y] = color

    if flipped:
        return newstate
    else:  
        # if no pieces are flipped, it's not a legal move
        return None

def terminal_test(state):
    '''Simple terminal test
    '''
    # if both players have skipped
    if state.num_skips == 2:
        return True

    # if there are no empty spaces
    empty_count = 0
    for i in range(SIZE):
        for j in range(SIZE):
            if state.board_array[i][j] == EMPTY:
                empty_count += 1
    if empty_count == 0:
        return True
    return False

def utility(state, player):
    '''Utility function
    '''
    color = player.get_color()

    # if there are no empty spaces
    score = 0
    #need to track tiles for terminal test, since utility doesn't corralate to actual number of tiles
    numPlayerTiles = 0
    numOppTiles = 0
    for i in range(SIZE):
        for j in range(SIZE):
            if state.board_array[i][j] == color:
                numPlayerTiles += 1
                #Corners are worth the most utility, cannot be captured
                if(((i == 0 or i == SIZE-1) and (j == 0 or j == SIZE-1))) :
                    score += 4
                #Edges are worth slightly more utility, fewer capture opportunities
                elif (j == 0 or j == SIZE-1) or (i == 0 or i == SIZE-1):
                    #On an edge and next to blocked space like a corner cannot be captured
                    if ((i < SIZE - 1 and state.board_array[i + 1][j] == BLOCKED) or (i > 0 and state.board_array[i - 1][j] == BLOCKED)) :
                        score += 4
                    elif ((j == SIZE - 2 and state.board_array[i][j + 1] == BLOCKED) or (j == 1 and state.board_array[i][j - 1] == BLOCKED)) :
                       score += 4
                    else :
                        score += 2
                # Not on an edge but adjacent to a blocked space, has one fewer capture opportunities. Does not seem to be good in practice to be here, hence why it is commented out
                #elif ((j == SIZE - 2 and (state.board_array[i][j + 1] == BLOCKED or state.board_array[i - 1][j + 1] == BLOCKED or state.board_array[i + 1][j + 1] == BLOCKED)) 
                #      or (j == 1 and (state.board_array[i][j - 1] == BLOCKED or state.board_array[i - 1][j - 1] == BLOCKED or state.board_array[i + 1][j - 1] == BLOCKED))) :
                #    score += 1.5
                else :
                    score += 1
            elif (state.board_array[i][j] != EMPTY and state.board_array[i][j] != BLOCKED):
                numOppTiles += 1
                score -= 1
    if(terminal_test(state)) :
        if numPlayerTiles > numOppTiles :
            score = 999999
        elif numPlayerTiles < numOppTiles :
            score = -999999
    return score

def display(state):
    '''Displays the current state in the terminal window
    '''
    print('  ', end='')
    for i in range(SIZE):
        print(i,end='')
    print()
    for i in range(SIZE):
        print(i, '', end='')
        for j in range(SIZE):
            if state.board_array[j][i] == WHITE:
                print('W', end='')
            elif state.board_array[j][i] == BLACK:
                print('B', end='')
            elif state.board_array[j][i] == BLOCKED:
                print('X', end='')
            else:
                print('-', end='')
        print()

def display_final(state):
    '''Displays the score and declares a winner (or tie)
    '''
    wcount = 0
    bcount = 0
    for i in range(SIZE):
        for j in range(SIZE):
            if state.board_array[i][j] == WHITE:
                wcount += 1
            elif state.board_array[i][j] == BLACK:
                bcount += 1

    print("Black: " + str(bcount))
    print("White: " + str(wcount))
    if wcount > bcount:
        print("White wins")
    elif wcount < bcount:
        print("Black wins")
    else:
        print("Tie")

def play_game(p1 = None, p2 = None):
    '''Plays a game with two players. By default, uses two humans
    '''
    if p1 == None:
        p1 = RandomPlayer(BLACK)
    if p2 == None:
        p2 = MinimaxPlayer(WHITE, 5)

    s = RandOthelloState(p1, p2)
    while True:
        action = p1.make_move(s)
        if action not in actions(s):
            print("Illegal move made by Black")
            print("White wins!")
            return
        s = result(s, action)
        if terminal_test(s):
            print("Game Over")
            display(s)
            display_final(s)
            return
        action = p2.make_move(s)
        if action not in actions(s):
            print("Illegal move made by White")
            print("Black wins!")
            return
        s = result(s, action)
        if terminal_test(s):
            print("Game Over")
            display(s)
            display_final(s)
            return

def main():
    start = time.time()
    for i in range(0, 5) :
      if i % 2 == 0 :
       print("minimax is white")
       play_game(MinimaxPlayer(WHITE, 5), RandomPlayer(BLACK))
       print("----------------")
      else :
          print("minimax is black")
          play_game(MinimaxPlayer(BLACK, 5), RandomPlayer(WHITE))
          print("----------------")
    end = time.time()
    print("minimax took " + str((end - start)) + " seconds.")
    print("----------------")
    start2 = time.time()
    for i in range(0, 5) :
      if i % 2 == 0 :
       print("ab is white")
       play_game(AlphabetaPlayer(WHITE, 5), RandomPlayer(BLACK))
       print("----------------")
      else :
          print("ab is black")
          play_game(AlphabetaPlayer(BLACK, 5), RandomPlayer(WHITE))
          print("----------------")
    end2 = time.time()
    print("ab took " + str((end2 - start2)) + " seconds.")
    start3 = time.time()
    for i in range(0, 5) :
      if i % 2 == 0 :
       print("ab is white")
       play_game(AlphabetaPlayer(WHITE, 8), MinimaxPlayer(BLACK, 5))
       print("----------------")
      else :
          print("ab is black")
          play_game(AlphabetaPlayer(BLACK, 8), MinimaxPlayer(WHITE, 5))
          print("----------------")
    end3 = time.time()
    print("ab v minimax took " + str((end3 - start3)) + " seconds.")
    print("----------------")
if __name__ == '__main__':
    main()
