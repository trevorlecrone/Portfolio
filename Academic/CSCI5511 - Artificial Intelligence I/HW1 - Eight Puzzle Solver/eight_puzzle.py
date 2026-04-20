import sys
import time
import argparse
# invoke script with `--initialState` (`-i`) argument
# script validates provided initial state is valid

#----------------CONSTANTS-----------------------------
#use 9 in place of 0 so states with a 0 in the upper left of the grid can be represented
GOAL_STATE = 123894765
GOAL_INDICIES = {
    0 : 1,
    1 : 2,
    2 : 3,
    3 : 8,
    4 : 9,
    5 : 4,
    6 : 7,
    7 : 6,
    8 : 5
}

RIGHT = "RIGHT"
LEFT = "LEFT"
UP = "UP"
DOWN = "DOWN"
AVAILABLE_ACTIONS = {
    0 : [LEFT, UP],
    1 : [RIGHT, LEFT, UP],
    2 : [RIGHT, UP],
    3 : [LEFT, UP, DOWN],
    4 : [RIGHT, LEFT, UP, DOWN],
    5 : [RIGHT, UP, DOWN],
    6 : [LEFT, DOWN],
    7 : [RIGHT, LEFT, DOWN],
    8 : [RIGHT, DOWN],
}

CUTOFF = "CUTOFF"
MANHATTAN = "MANHATTAN"
WRONG_TILES = "WRONG_TILES"

#----------------HELPERS--------------------------------

def find_nine(state):
    stateString = str(state)
    try :
        return stateString.index('9')
    except :
        return "State must contain a 9 indicating location of empty space"
    
def opposite_action(action) :
    if action == RIGHT :
        return LEFT
    elif action== LEFT :
        return RIGHT
    elif action == UP :
        return DOWN
    elif action == DOWN :
        return UP
    
def validate_initial_state (state) :
    if state is None :
        return False
    correctLengthAndUnique = len(list(state)) == 9 and len(set(state)) == 9
    if ('0' in state and correctLengthAndUnique and state.isdigit()) :
        return True
    return False

    
def visualize(state):
    stateString = str(state).replace('9', '0')
    print("[" + stateString[0] + "]" + "[" + stateString[1] + "]" + "[" + stateString[2] + "]")
    print("[" + stateString[3] + "]" + "[" + stateString[4] + "]" + "[" + stateString[5] + "]")
    print("[" + stateString[6] + "]" + "[" + stateString[7] + "]" + "[" + stateString[8] + "]")

def apply_action(state, action) :
    powerAtNine = 8-find_nine(state)
    state -= 9 * 10**powerAtNine
    if action == RIGHT :
        state += (state // 10**(powerAtNine + 1) % 10) * 10**powerAtNine
        state -= (state // 10**(powerAtNine + 1) % 10) * 10**(powerAtNine + 1)
        state += 9 * 10**(powerAtNine + 1)
    elif action == LEFT :
        state += (state // 10**(powerAtNine - 1) % 10) * 10**powerAtNine
        state -= (state // 10**(powerAtNine - 1) % 10) * 10**(powerAtNine - 1)
        state += 9 * 10**(powerAtNine - 1) 
    elif action == UP :
        state += (state // 10**(powerAtNine - 3) % 10) * 10**powerAtNine
        state -= (state // 10**(powerAtNine - 3) % 10) * 10**(powerAtNine - 3)
        state += 9 * 10**(powerAtNine - 3)
    elif action == DOWN :
        state += (state // 10**(powerAtNine + 3) % 10) * 10**powerAtNine
        state -= (state // 10**(powerAtNine + 3) % 10) * 10**(powerAtNine + 3)
        state += 9 * 10**(powerAtNine + 3)
    return state

#---------------------DATA MODEL AND SEARCHES-------------------------------

class Node:
    def __init__(self, state: int, parent=None, action=None, cost=0) :
        self.state = state
        self.parent = parent
        self.action = action
        self.cost = cost

    def child_node(self, action) :
        state = apply_action(self.state, action)
        return Node(state, self, action, self.cost + 1)

def cycle_check(node: Node) :
    #we just undid the previous action
    if node.parent.action == opposite_action(node.action) :
        return True
    return False
    
def get_sequence(node: Node):
    actions = []
    while node.parent is not None:
        actions.append(node.action)
        node = node.parent
    actions.reverse()
    return actions


def breadth_first(state) :
    start = Node(int(state))
    if(start.state == GOAL_STATE) :
        return "NO ACTIONS, INITIAL STATE IS GOAL STATE"
    else :
        frontier = [start]
        reached = [start.state]
        count = 0
        while len(frontier) > 0 :
            node = frontier.pop(0)
            emptyPosition = find_nine(node.state)
            for action in AVAILABLE_ACTIONS[emptyPosition] :
                child = node.child_node(action)
                if(child.state == GOAL_STATE) :
                    return get_sequence(child)
                if(child.state not in reached) :
                    reached.append(child.state)
                    frontier.append(child)
                    count += 1
        return "FAILURE"
    
def depth_limited(state, maxDepth) :
    start = Node(int(state))
    if(start.state == GOAL_STATE) :
        return "NO ACTIONS, INITIAL STATE IS GOAL STATE"
    else :
        frontier = [start]
        count = 0
        result = None
        while len(frontier) > 0 :
            node = frontier.pop()
            emptyPosition = find_nine(node.state)
            for action in AVAILABLE_ACTIONS[emptyPosition] :
                child = node.child_node(action)
                if(child.state == GOAL_STATE) :
                    return get_sequence(child)
                if(child.cost > maxDepth) :
                    result = CUTOFF
                elif(not cycle_check(child)) :
                    frontier.append(child)
                    count += 1
        return result

def iterative_deepening(state) :
    maxDepth = 1
    #all solvable 8-puzzles can be solved in 31 slides, below limit was useful when testing
    #while maxDepth < 32:
    while True :
        result = depth_limited(state, maxDepth)
        if(result != CUTOFF) :
            return result
        maxDepth += 1

def num_wrong_tiles(state) :
    numWrongTiles = 0
    i = 8
    while i >= 0 :
        targetPower = 8 - i
        digit = state // 10**targetPower % 10
        if digit != GOAL_INDICIES[i] and digit != 9 :
            numWrongTiles += 1
        i  -= 1
    return numWrongTiles

def  get_single_tile_manhattan_distance(tileIndex, digit) :
    targetIndex = list(GOAL_INDICIES.values()).index(digit)
    distance = 0
    #Based on position of target, try to make a vertical move to move closer is possible. 
    #if on the same row already, move to target position, otherwise try to align for a vertical move
    while tileIndex != targetIndex :
        if tileIndex > targetIndex :
            if tileIndex - 3 >= targetIndex :
                tileIndex -= 3
                distance += 1
            #same "row"
            elif tileIndex // 3 == targetIndex // 3 :
                tileIndex -= 1
                distance += 1
            else :
                tileIndex += 1
                distance += 1
        else :
            if tileIndex + 3 <= targetIndex :
                tileIndex += 3
                distance += 1
            #same "row"
            elif tileIndex // 3 == targetIndex // 3 :
                tileIndex += 1
                distance += 1
            else :
                tileIndex -= 1
                distance += 1
    return distance

def manhattan_distance(state):
    manhattanDistance = 0
    i = 8
    while i >= 0 :
        targetPower = 8 - i
        digit = state // 10**targetPower % 10
        if digit != GOAL_INDICIES[i] and digit != 9 :
            manhattanDistance += get_single_tile_manhattan_distance(i, digit)
        i  -= 1
    return manhattanDistance

def run_heuristic(state, heuristic) :
    if heuristic == MANHATTAN :
        return manhattan_distance(state)
    elif heuristic == WRONG_TILES :
        return num_wrong_tiles(state)
    else :
        return None

def astar(state, heuristic) :
    start = Node(int(state))
    #since action cost is constant, and we pop off the by min astar, in keeping path of cost shouldn't be necesarry
    reached = [start.state]
    frontier = {run_heuristic(start.state, heuristic): [start]}
    while len(frontier) > 0 :
        minAstarValue = min(frontier.keys())
        node = frontier[minAstarValue].pop(0)
        if len(frontier[minAstarValue]) == 0:
            frontier.pop(minAstarValue)
        reached.append(node.state)
        emptyPosition = find_nine(node.state)
        for action in AVAILABLE_ACTIONS[emptyPosition] :
                child = node.child_node(action)
                if(child.state == GOAL_STATE) :
                    return get_sequence(child)
                elif child.state not in reached :
                    newAstarValue = child.cost + run_heuristic(child.state, heuristic)
                    if newAstarValue in frontier :
                        frontier[newAstarValue].append(child)
                    else :
                        frontier[newAstarValue] = [child]

#------------------SIMPLE TESTS----------------------       
#visualize(GOAL_STATE)
#print()
#visualize(apply_action(GOAL_STATE, RIGHT))
#print()
#visualize(apply_action(GOAL_STATE, LEFT))
#print()
#visualize(apply_action(GOAL_STATE, UP))
#print()
#visualize(apply_action(GOAL_STATE, DOWN))
#print()
#print(num_wrong_tiles(GOAL_STATE))
#print(num_wrong_tiles(apply_action(GOAL_STATE, DOWN)))

#print(manhattan_distance(GOAL_STATE))
#print(manhattan_distance(apply_action(GOAL_STATE, DOWN)))

#print(num_wrong_tiles(987654321))

#print(manhattan_distance(987654321))

#visualize(987654321)

#------------------MAIN-------------------------------
parser = argparse.ArgumentParser()
parser.add_argument('--initialState', '-i', help="a 9 digit number representing the initial state of the 8 puzzle", type= str)
args = parser.parse_args()
providedState = args.initialState
if not validate_initial_state(providedState) :
    print("Initial state must be provided in command line as the '--initialState' or '-i' argument. Initial state needs to be a 9 digit number containing all digits 0-8 exactly one time. Please provide a valid state")
else :
    print ("Initial State : " + providedState)
    adjustedState = providedState.replace('0', '9')
    visualize(int(adjustedState))
    print()

    print("Finding solution using breadth first search")
    bfStart = time.time()
    breadthFirstList = breadth_first(adjustedState)
    bfEnd = time.time()
    print("Breadth first Solution:")
    print(breadthFirstList)
    print("Breadth first took: " + str((bfEnd - bfStart)) + " seconds")
    print("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")
    print()

    print("Finding solution using iterative deepening search")
    idStart = time.time()
    iterativeDeepeningList = iterative_deepening(adjustedState)
    idEnd= time.time()
    print("Iterative deepening Solution:")
    print(iterativeDeepeningList)
    print("iterative deepening first took: " + str((idEnd - idStart)) + " seconds")
    print("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")
    print()

    print("Finding solution using A* with number of wrong tiles heuristic")
    aStarWrongTilesStart = time.time()
    wrongTilesList = astar(adjustedState, WRONG_TILES)
    aStarWrongTilesEnd = time.time()
    print("A* with number of wrong tiles heuristic solution:")
    print(wrongTilesList)
    print("A* with number of wrong tiles heuristic took:" + str((aStarWrongTilesEnd - aStarWrongTilesStart)) + " seconds")
    print("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")
    print()

    print("Finding solution using A* with manhattan distance heuristic")
    aStarManDistanceStart = time.time()
    manhattanDistanceList = astar(adjustedState, MANHATTAN)
    aStarManDistanceEnd = time.time()
    print("A* with manhattan distance heuristic solution:")
    print(manhattanDistanceList)
    print("A* with manhattan distance heuristic took:" + str((aStarManDistanceEnd - aStarManDistanceStart))  + " seconds")