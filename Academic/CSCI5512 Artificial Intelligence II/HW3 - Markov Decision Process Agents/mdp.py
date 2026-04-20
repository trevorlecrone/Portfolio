import random
#----------------CONSTANTS, HELPERS, & CLASSES-------------------------
# action constants
UP = 1
RIGHT = 2
DOWN = 3
LEFT = 4

actionNames = ["UP", "RIGHT", "LEFT", "DOWN"]

TERMINAL_REWARD_4_3 = 1.0
TERMINAL_REWARD_4_2 = -1.0

def GetRightPerpendicularMove(action: int) :
    if action == UP:
        return RIGHT
    elif action == RIGHT:
        return DOWN
    elif action == DOWN:
        return LEFT
    elif action == LEFT:
        return UP
    return "INVALID ACTION"

def GetLeftPerpendicularMove(action: int) :
    if action == UP:
        return LEFT
    elif action == RIGHT:
        return UP
    elif action == DOWN:
        return RIGHT
    elif action == LEFT:
        return DOWN
    return "INVALID ACTION"

def GetKey(column: int, row: int):
        return str(column) + "," + str(row)

def InitializeStandardStates(standardReward: float) :
    states = {}
    for row in range(1, 4) :
        for col in range(1, 5) :
            key = GetKey(col, row)
            state = None
            if (row != 2 or col != 2) :
                if (not (row == 2 and col == 4) and not (row == 3 and col == 4)) :
                    state = MDPState(col, row, 0.0, False, True, standardReward, None)
                else :
                    reward = TERMINAL_REWARD_4_2
                    if (row == 3) :
                        reward = TERMINAL_REWARD_4_3
                    state = MDPState(col, row, 0.0, True, True, reward, None)
            else :
                state = MDPState(col, row, 0.0, False, False, None, None)
            states[key] = state
    return states

def CheckConvergence(currentMDP, prevMDP) :
    currentStates = list(currentMDP.states.values())
    prevStates = list(prevMDP.states.values())
    for i in range(len(currentStates)) :
        if (prevStates[i].value != currentStates[i].value or prevStates[i].policy != currentStates[i].policy) :
            return False
    return True

class MDPState:
    #a single state in the MDP, contains row and column coords, state value, flags indicating if the state is terminal or reachable, and the policy (not used for MC) for the state
    def __init__(self, column: int, row: int, value: float, terminal: bool, reachable: bool, reward: float, policy: int) :
        self.column = column
        self.row = row
        self.value = value
        self.terminal = terminal
        self.reachable = reachable
        self.reward = reward
        self.policy = policy
    
    def PrintState(self) :
        print("(" + str(self.column) + "," + str(self.row) + "): " + str(self.value) + ", " + actionNames[self.policy - 1])

class MDP:
    #an MDP, contains number of rows and columns, and all individual states
    def __init__(self, columns: int, rows: int, states: dict) :
        self.columns = columns
        self.rows = rows
        self.states = states

    def AvailableActions(self, column: int, row: int) :
        actions = []
        if row < self.rows :
            if(self.states[GetKey(column, row + 1)].reachable) :
                actions.append(UP)
        if column < self.columns :
            if(self.states[GetKey(column + 1, row)].reachable) :
                actions.append(RIGHT)
        if row > 1 :
            if(self.states[GetKey(column, row - 1)].reachable) :
                actions.append(DOWN)
        if column > 1 :
            if(self.states[GetKey(column - 1, row)].reachable) :
                actions.append(LEFT)
        return actions
    
    def AllActions(self) :
        return [UP, RIGHT, DOWN, LEFT]

    def InitializeValueFunction(self, value):
        for state in self.states.values() :
            if(state.reachable and not state.terminal) :
                state.value = value

    def InitializePolicy(self):
        for state in self.states.values() :
            if(state.reachable and not state.terminal) :
                state.policy = self.AvailableActions(state.column, state.row)[0]

    def GetTransitionCoords(self, column, row, action) : 
        if action == UP:
            return [column, row + 1]
        elif action == RIGHT:
            return [column + 1, row]
        elif action == DOWN:
            return [column, row - 1]
        elif action == LEFT:
            return [column - 1, row]

    def PrintStates(self):
        print("State: State-values v(s), Policy")
        for state in self.states.values() :
            if(state.reachable and not state.terminal) :
                state.PrintState()

    def ActionExpectedVal(self, column, row, action, discount) :
        #possible transitions from current state
        availableActions = self.AvailableActions(column, row)

        #Expected value for moving in desired direction
        chosenActionCoords = self.GetTransitionCoords(column, row, action)
        chosenActionState = self.states[GetKey(chosenActionCoords[0], chosenActionCoords[1])]
        expectedVal = (chosenActionState.reward + discount * chosenActionState.value) * 0.8

        #expected value for moving to the right and left perpendicularly, staying in same state if a wall prevents the transition
        rightPerpendicularMoveCoords = [column, row]
        leftPerpendicularMoveCoords = [column, row]
        if(GetRightPerpendicularMove(action) in availableActions) :
            rightPerpendicularMoveCoords = self.GetTransitionCoords(column, row, GetRightPerpendicularMove(action))
        rightPerpendicularMoveState = self.states[GetKey(rightPerpendicularMoveCoords[0], rightPerpendicularMoveCoords[1])]
        expectedVal += (rightPerpendicularMoveState.reward + discount * rightPerpendicularMoveState.value) * 0.1
        
        if(GetLeftPerpendicularMove(action) in availableActions) :
            leftPerpendicularMoveCoords = self.GetTransitionCoords(column, row, GetLeftPerpendicularMove(action))
        leftPerpendicularMoveState = self.states[GetKey(leftPerpendicularMoveCoords[0], leftPerpendicularMoveCoords[1])]
        expectedVal += (leftPerpendicularMoveState.reward + discount * leftPerpendicularMoveState.value) * 0.1
        return expectedVal

class MDPMCAgent:
    #an MDP agent used for monte-carlo episodes, knows current row and column, and the MDP it has been given, the state-action values and the policy the MC equation will use
    def __init__(self, column: int, row: int, mdp: MDP) :
        self.column = column
        self.row = row
        self.mdp = mdp
        self.policy = self.GenerateInitialPolicy()
        self.value = self.GenerateInitialValue()

    def MakeTransition(self, action) : 
        if action == UP:
            self.row += 1
        elif action == RIGHT:
            self.column += 1
        elif action == DOWN:
            self.row -= 1
        elif action == LEFT:
            self.column -= 1

    def GenerateInitialPolicy(self) : 
        policy = {}
        for i in range(1, self.mdp.columns + 1) :
            for j in range (1, self.mdp.rows + 1) :
                state = self.mdp.states[GetKey(i, j)]
                #optimal threshold is the value we always choose the optimal policy for
                if(state.reachable and not state.terminal) :
                    policy[GetKey(i, j)] = dict(optimalThreshold = 1.0, optimalAction = None, nonOptimalActions=self.mdp.AllActions())
        return policy
    
    #state-actio
    def GenerateInitialValue(self) : 
        value = {}
        for i in range(1, self.mdp.columns + 1) :
            for j in range (1, self.mdp.rows + 1) :
                state = self.mdp.states[GetKey(i, j)]
                #optimal threshold is the value we always choose the optimal policy for
                if(state.reachable and not state.terminal) :
                    for action in self.mdp.AvailableActions(i, j) :
                        value[GetKey(i, j) + "," + str(action)] = 0.0
        return value
    
    def CheckMCAgentConvergence(self, oldPolicy, oldValue) : 
        for i in range(1, self.mdp.columns + 1) :
            for j in range (1, self.mdp.rows + 1) :
                state = self.mdp.states[GetKey(i, j)]
                #optimal threshold is the value we always choose the optimal policy for
                if(state.reachable and not state.terminal) :
                    if not self.policy[GetKey(i, j)] == oldPolicy[GetKey(i, j)] :
                        return False
                    for action in self.mdp.AvailableActions(i, j) :
                        if not self.value[GetKey(i, j) + "," + str(action)] == oldValue[GetKey(i, j) + "," + str(action)] :
                            return False
        return True
    
    #if a rand value between 0 and 1 is greater than the optimal threshold, return the optimal action, else choose a random non optimal action
    #this assumes that the python random function is sufficiently fair, and non optimal actions have even chances of being chosen
    def GetActionFromPolicy(self, column, row) : 
        val = random.random()
        policyEntry = self.policy[GetKey(column, row)]
        if (val > policyEntry['optimalThreshold']) :
            return policyEntry['optimalAction']
        else :
            nonOptimalIndex = random.randrange(0, len(policyEntry['nonOptimalActions']))
            return policyEntry['nonOptimalActions'][nonOptimalIndex]

    def TakeAction(self, action) :
        sampleVal = random.random()
        availableActions = self.mdp.AvailableActions(self.column, self.row)
        if (action not in availableActions) :
            return
        if (sampleVal < 0.1) :
            if (GetRightPerpendicularMove(action) in availableActions) :
                self.MakeTransition(GetRightPerpendicularMove(action))
                return
            else :
                return
        elif(sampleVal >= 0.1 and sampleVal < 0.2) :
            if (GetLeftPerpendicularMove(action) in availableActions) :
                self.MakeTransition(GetLeftPerpendicularMove(action))
                return
            else :
                return
        else :
            self.MakeTransition(action)

    def GenerateEpisode(self) :
        self.column = 1
        self.row = 1
        currentState = self.mdp.states[GetKey(self.column, self.row)]
        result = []
        while not currentState.terminal :
            action = self.GetActionFromPolicy(self.column, self.row)
            self.TakeAction(action)
            reward = self.mdp.states[GetKey(self.column, self.row)].reward
            result.append(dict(state= currentState, action=action, reward=reward))
            currentState = self.mdp.states[GetKey(self.column, self.row)]
        return result
    
    def GetOptimalAction(self, column, row) :
        maxVal = -100000
        optimalAction = None
        for action in self.mdp.AllActions() :
            if (GetKey(column, row) + "," + str(action)) in self.value.keys() :
                val = self.value[GetKey(column, row) + "," + str(action)]
                if(val > maxVal) :
                    maxVal = val
                    optimalAction = action
        return optimalAction
    
    def Print(self) :
        print("State:                Action-values q(s, a),                         Policy")
        for state in self.mdp.states.values() :
            #optimal threshold is the value we always choose the optimal policy for
            if(state.reachable and not state.terminal) :
                print("(" + str(state.column) + "," + str(state.row) + "):",end =" ")
                for action in self.mdp.AllActions() :
                    print(actionNames[action - 1] + ": " + str(self.value[GetKey(state.column, state.row) + "," + str(action)]),end =" ")
                print("|  " + actionNames[self.policy[GetKey(state.column, state.row) ]['optimalAction'] - 1])
        return True