import matplotlib.pyplot as plotter
#------------------CONSTANTS & Helpers----------------

PRISONERS_DILEMMA_MATRIX = [[[1.0, 1.0], [5.0, 0.0]], 
                            [[0.0, 5.0], [3.0, 3.0]]]
CHICKEN_MATRIX = [[[3.0, 3.0], [1.5, 3.5]],
                  [[3.5, 1.5], [1.0, 1.0]]]
MOVIE_COORD_MATRIX = [[[3.0, 2.0], [0.0, 0.0]], 
                      [[0.0, 0.0], [2.0, 3.0]]]

PRISONERS_DILEMMA_SECURITY_VAL = 1.0
CHICKEN_SECURITY_VAL = 1.5
MOVIE_COORD_SECURITY_VAL = 0.0

ACTION1 = 0
ACTION2 = 1

ACTIONS = [ACTION1, ACTION2]

NUM_ROUNDS = 100

def GetMaxMinAction(position: int, game: list) :
    if position == 0 :
        action1Min = min(game[0][0][0], game[0][1][0])
        action2Min = min(game[1][0][0], game[1][1][0])
        return ACTION1 if action1Min > action2Min else ACTION2
    else :
        action1Min = min(game[0][0][1], game[1][0][1])
        action2Min = min(game[0][1][1], game[1][1][1])
        return ACTION1 if action1Min > action2Min else ACTION2
    
def GetOppMinMaxAction(position: int, game: list) :
    if position == 0 :
        action1OppMax = max(game[0][0][1], game[0][1][1])
        action2OppMax = max(game[1][0][1], game[1][1][1])
        return ACTION1 if action1OppMax < action2OppMax else ACTION2
    else :
        action1OppMax = min(game[0][0][0], game[1][0][0])
        action2OppMax = min(game[0][1][0], game[1][1][0])
        return ACTION1 if action1OppMax < action2OppMax else ACTION2

def addToData(rowAgentAvgReward, colAgentAvgReward, data: list) :
    data.append(str(rowAgentAvgReward) + "," + str(colAgentAvgReward))

#------------------GAME CLASS & AGENTS----------------

class Game :
    def __init__(self, matrix: list, securityVal: float) :
        self.matrix = matrix
        self.securityVal = securityVal

class Agent :
    def __init__(self, position: int) :
        self.position = position
    def makePlay(self, game: Game) :
        return ACTIONS[0]
    def observe(self, prevRound: list) :
        return

class TFTAgent(Agent) :
    def __init__(self, position: int) :
        self.position = position
        self.prevOppAction = None
    def makePlay(self, game: Game) :
        if(not self.prevOppAction == None) :
            return self.prevOppAction
        else :
            return GetMaxMinAction(self.position, game.matrix)
    def observe(self, prevRound: list) :
        self.prevOppAction = prevRound[1-self.position]

class FicPlayAgent(Agent) :
    def __init__(self, position: int) :
        self.position = position
        self.oppPolicyModel = {}
        for action in ACTIONS :
            self.oppPolicyModel[action] = 1.0 / len(ACTIONS)
        self.roundCounter = 0
        self.oppAction1Count = 0
        self.oppAction2Count = 0
    def makePlay(self, game: Game) :
        matrix = game.matrix
        if self.position == 0 :
            a1ExpectedReward = self.oppPolicyModel[ACTION1] * matrix[0][0][0] + self.oppPolicyModel[ACTION2] * matrix[0][1][0]
            a2ExpectedReward = self.oppPolicyModel[ACTION1] * matrix[1][0][0] + self.oppPolicyModel[ACTION2] * matrix[1][1][0]
            return ACTION1 if a1ExpectedReward > a2ExpectedReward else ACTION2
        else :
            a1ExpectedReward = self.oppPolicyModel[ACTION1] * matrix[0][0][1] + self.oppPolicyModel[ACTION2] * matrix[1][0][1]
            a2ExpectedReward = self.oppPolicyModel[ACTION1] * matrix[0][1][1] + self.oppPolicyModel[ACTION2] * matrix[1][1][1]
            return ACTION1 if a1ExpectedReward > a2ExpectedReward else ACTION2
    def observe(self, prevRound: list) :
        self.roundCounter += 1
        oppAction = prevRound[1-self.position]
        if(oppAction == ACTION1) :
            self.oppAction1Count += 1
        else :
            self.oppAction2Count += 1
    def updateOppPolicyModel(self) :
        self.oppPolicyModel[ACTION1] = self.oppAction1Count / self.roundCounter
        self.oppPolicyModel[ACTION2] = self.oppAction2Count / self.roundCounter
    
class BullyAgent(Agent) :
    def __init__(self, position: int) :
        self.position = position
    def makePlay(self, game: Game) :
        return self.GetMaxBestResponseAction(game)
    def observe(self, prevRound) :
        return
    def GetMaxBestResponseAction(self, game: Game) :
        matrix = game.matrix
        if self.position == 0 :
            a1BestResponse = ACTION1 if matrix[0][0][1] > matrix[0][1][1] else ACTION2
            a2BestResponse = ACTION1 if matrix[1][0][1] > matrix[1][1][1] else ACTION2
            return ACTION1 if matrix[0][a1BestResponse][0] > matrix[1][a2BestResponse][0] else ACTION2
        else :
            a1BestResponse = ACTION1 if matrix[0][0][0] > matrix[1][0][0] else ACTION2
            a2BestResponse = ACTION1 if matrix[0][1][0] > matrix[1][1][0] else ACTION2
            return ACTION1 if matrix[a1BestResponse][0][1] > matrix[a2BestResponse][1][1] else ACTION2
    
class GodfatherAgent(Agent) :
    def __init__(self, position: int) :
        self.position = position
        self.offerRefused = False
        self.targetablePair = []
    def makePlay(self, game: Game) :
        self.targetablePair = self.GetTargetablePair(game)
        if self.offerRefused :
            return self.GetWorstActionForOpp(game)
        else :
            return self.targetablePair[self.position]
    def observe(self, prevRound) :
        if not (prevRound[0] == self.targetablePair[0] and prevRound[1] == self.targetablePair[1]) :
            self.offerRefused = True
    def GetWorstActionForOpp(self, game: Game) :
        return GetOppMinMaxAction(self.position, game.matrix)
    def GetTargetablePair(self, game: Game) :
        matrix = game.matrix
        for i in ACTIONS :
            for j in ACTIONS :
                if matrix[i][j][0] > game.securityVal and matrix[i][j][1] > game.securityVal :
                    return [i, j]
    
def playMatch(rowAgent, colAgnet, game: Game) :
    rowAgentReward = 0.0
    colAgentReward = 0.0
    for _ in range(NUM_ROUNDS) :
        rowAgentAction = rowAgent.makePlay(game)
        colAgentAction = colAgnet.makePlay(game)
        prevRound = [rowAgentAction, colAgentAction]
        rowAgent.observe(prevRound)
        colAgnet.observe(prevRound)
        rewards = game.matrix[rowAgentAction][colAgentAction]
        rowAgentReward += rewards[0]
        colAgentReward += rewards[1]
    return [rowAgentReward/100.0, colAgentReward/100.0]
    


#-----------------MAIN LOGIC----------------------------
#PD
pdGame = Game(PRISONERS_DILEMMA_MATRIX, PRISONERS_DILEMMA_SECURITY_VAL)
pd_t4t_t4t_row = 0.0
pd_t4t_t4t_column = 0.0
pd_t4t_fp_t4t = 0.0
pd_t4t_fp_fp = 0.0
pd_t4t_b_t4t = 0.0
pd_t4t_b_b = 0.0
pd_t4t_gf_t4t = 0.0
pd_t4t_gf_gf = 0.0

pd_fp_fp_row = 0.0
pd_fp_fp_column = 0.0
pd_fp_b_fp = 0.0
pd_fp_b_b = 0.0
pd_fp_gf_fp = 0.0
pd_fp_gf_gf = 0.0

pd_b_b_row = 0.0
pd_b_b_column = 0.0
pd_b_gf_b = 0.0
pd_b_gf_gf = 0.0

pd_gf_gf_row = 0.0
pd_gf_gf_column = 0.0

[pd_t4t_t4t_row, pd_t4t_t4t_column] = playMatch(TFTAgent(0), TFTAgent(1), pdGame)
[pd_t4t_fp_t4t, pd_t4t_fp_fp] = playMatch(TFTAgent(0), FicPlayAgent(1), pdGame)
[pd_t4t_b_t4t, pd_t4t_b_b] = playMatch(TFTAgent(0), BullyAgent(1), pdGame)
[pd_t4t_gf_t4t, pd_t4t_gf_gf] = playMatch(TFTAgent(0), GodfatherAgent(1), pdGame)

[pd_fp_fp_row, pd_fp_fp_column] = playMatch(FicPlayAgent(0), FicPlayAgent(1), pdGame)
[pd_fp_b_fp, pd_fp_b_b] = playMatch(FicPlayAgent(0), BullyAgent(1), pdGame)
[pd_fp_gf_fp, pd_fp_gf_gf] = playMatch(FicPlayAgent(0), GodfatherAgent(1), pdGame)

[pd_b_b_row, pd_b_b_column] = playMatch(BullyAgent(0), BullyAgent(1), pdGame)
[pd_b_gf_b, pd_b_gf_gf] = playMatch(BullyAgent(0), GodfatherAgent(1), pdGame)

[pd_gf_gf_row, pd_gf_gf_column] = playMatch(GodfatherAgent(0), GodfatherAgent(1), pdGame)

agentNames = ["Tit-for-tat ", "Fictitious Play", "Bully", "Godfather"]
data = []
row1 = []
addToData(pd_t4t_t4t_row, pd_t4t_t4t_column, row1)
addToData(pd_t4t_fp_t4t, pd_t4t_fp_fp, row1)
addToData(pd_t4t_b_t4t, pd_t4t_b_b, row1)
addToData(pd_t4t_gf_t4t, pd_t4t_gf_gf, row1)
row2 = []
row2.append("-")
addToData(pd_fp_fp_row, pd_fp_fp_column, row2)
addToData(pd_fp_b_fp, pd_fp_b_b, row2)
addToData(pd_fp_gf_fp, pd_fp_gf_gf, row2)
row3 = []
row3.append("-")
row3.append("-")
addToData(pd_b_b_row, pd_b_b_column, row3)
addToData(pd_b_gf_b, pd_b_gf_gf, row3)
row4 = []
row4.append("-")
row4.append("-")
row4.append("-")
addToData(pd_gf_gf_row, pd_gf_gf_column, row4)
data.append(row1)
data.append(row2)
data.append(row3)
data.append(row4)

fig, ax = plotter.subplots()
fig.patch.set_visible(False)
ax.axis('off')
ax.axis('tight')
ax.set_title("Prisoner's Dilemma")

table = plotter.table(rowLabels=agentNames, colLabels=agentNames, cellText=data, loc='center')
fig.tight_layout()
plotter.savefig('pd_table.png')

#Chicken
chickenGame = Game(CHICKEN_MATRIX, CHICKEN_SECURITY_VAL)
chicken_t4t_t4t_row = 0.0
chicken_t4t_t4t_column = 0.0
chicken_t4t_fp_t4t = 0.0
chicken_t4t_fp_fp = 0.0
chicken_t4t_b_t4t = 0.0
chicken_t4t_b_b = 0.0
chicken_t4t_gf_t4t = 0.0
chicken_t4t_gf_gf = 0.0

chicken_fp_fp_row = 0.0
chicken_fp_fp_column = 0.0
chicken_fp_b_fp = 0.0
chicken_fp_b_b = 0.0
chicken_fp_gf_fp = 0.0
chicken_fp_gf_gf = 0.0

chicken_b_b_row = 0.0
chicken_b_b_column = 0.0
chicken_b_gf_b = 0.0
chicken_b_gf_gf = 0.0

chicken_gf_gf_row = 0.0
chicken_gf_gf_column = 0.0

[chicken_t4t_t4t_row, chicken_t4t_t4t_column] = playMatch(TFTAgent(0), TFTAgent(1), chickenGame)
[chicken_t4t_fp_t4t, chicken_t4t_fp_fp] = playMatch(TFTAgent(0), FicPlayAgent(1), chickenGame)
[chicken_t4t_b_t4t, chicken_t4t_b_b] = playMatch(TFTAgent(0), BullyAgent(1), chickenGame)
[chicken_t4t_gf_t4t, chicken_t4t_gf_gf] = playMatch(TFTAgent(0), GodfatherAgent(1), chickenGame)

[chicken_fp_fp_row, chicken_fp_fp_column] = playMatch(FicPlayAgent(0), FicPlayAgent(1), chickenGame)
[chicken_fp_b_fp, chicken_fp_b_b] = playMatch(FicPlayAgent(0), BullyAgent(1), chickenGame)
[chicken_fp_gf_fp, chicken_fp_gf_gf] = playMatch(FicPlayAgent(0), GodfatherAgent(1), chickenGame)

[chicken_b_b_row, chicken_b_b_column] = playMatch(BullyAgent(0), BullyAgent(1), chickenGame)
[chicken_b_gf_b, chicken_b_gf_gf] = playMatch(BullyAgent(0), GodfatherAgent(1), chickenGame)

[chicken_gf_gf_row, chicken_gf_gf_column] = playMatch(GodfatherAgent(0), GodfatherAgent(1), chickenGame)

agentNames = ["Tit-for-tat ", "Fictitious Play", "Bully", "Godfather"]
data = []
row1 = []
addToData(chicken_t4t_t4t_row, chicken_t4t_t4t_column, row1)
addToData(chicken_t4t_fp_t4t, chicken_t4t_fp_fp, row1)
addToData(chicken_t4t_b_t4t, chicken_t4t_b_b, row1)
addToData(chicken_t4t_gf_t4t, chicken_t4t_gf_gf, row1)
row2 = []
row2.append("-")
addToData(chicken_fp_fp_row, chicken_fp_fp_column, row2)
addToData(chicken_fp_b_fp, chicken_fp_b_b, row2)
addToData(chicken_fp_gf_fp, chicken_fp_gf_gf, row2)
row3 = []
row3.append("-")
row3.append("-")
addToData(chicken_b_b_row, chicken_b_b_column, row3)
addToData(chicken_b_gf_b, chicken_b_gf_gf, row3)
row4 = []
row4.append("-")
row4.append("-")
row4.append("-")
addToData(chicken_gf_gf_row, chicken_gf_gf_column, row4)
data.append(row1)
data.append(row2)
data.append(row3)
data.append(row4)

fig, ax = plotter.subplots()
fig.patch.set_visible(False)
ax.axis('off')
ax.axis('tight')
ax.set_title("Chicken")

table = plotter.table(rowLabels=agentNames, colLabels=agentNames, cellText=data, loc='center')
fig.tight_layout()
plotter.savefig('chicken_table.png')

#Movie Coord
mcGame = Game(MOVIE_COORD_MATRIX, MOVIE_COORD_SECURITY_VAL)
mc_t4t_t4t_row = 0.0
mc_t4t_t4t_column = 0.0
mc_t4t_fp_t4t = 0.0
mc_t4t_fp_fp = 0.0
mc_t4t_b_t4t = 0.0
mc_t4t_b_b = 0.0
mc_t4t_gf_t4t = 0.0
mc_t4t_gf_gf = 0.0

mc_fp_fp_row = 0.0
mc_fp_fp_column = 0.0
mc_fp_b_fp = 0.0
mc_fp_b_b = 0.0
mc_fp_gf_fp = 0.0
mc_fp_gf_gf = 0.0

mc_b_b_row = 0.0
mc_b_b_column = 0.0
mc_b_gf_b = 0.0
mc_b_gf_gf = 0.0

mc_gf_gf_row = 0.0
mc_gf_gf_column = 0.0

[mc_t4t_t4t_row, mc_t4t_t4t_column] = playMatch(TFTAgent(0), TFTAgent(1), mcGame)
[mc_t4t_fp_t4t, mc_t4t_fp_fp] = playMatch(TFTAgent(0), FicPlayAgent(1), mcGame)
[mc_t4t_b_t4t, mc_t4t_b_b] = playMatch(TFTAgent(0), BullyAgent(1), mcGame)
[mc_t4t_gf_t4t, mc_t4t_gf_gf] = playMatch(TFTAgent(0), GodfatherAgent(1), mcGame)

[mc_fp_fp_row, mc_fp_fp_column] = playMatch(FicPlayAgent(0), FicPlayAgent(1), mcGame)
[mc_fp_b_fp, mc_fp_b_b] = playMatch(FicPlayAgent(0), BullyAgent(1), mcGame)
[mc_fp_gf_fp, mc_fp_gf_gf] = playMatch(FicPlayAgent(0), GodfatherAgent(1), mcGame)

[mc_b_b_row, mc_b_b_column] = playMatch(BullyAgent(0), BullyAgent(1), mcGame)
[mc_b_gf_b, mc_b_gf_gf] = playMatch(BullyAgent(0), GodfatherAgent(1), mcGame)

[mc_gf_gf_row, mc_gf_gf_column] = playMatch(GodfatherAgent(0), GodfatherAgent(1), mcGame)

agentNames = ["Tit-for-tat ", "Fictitious Play", "Bully", "Godfather"]
data = []
row1 = []
addToData(mc_t4t_t4t_row, mc_t4t_t4t_column, row1)
addToData(mc_t4t_fp_t4t, mc_t4t_fp_fp, row1)
addToData(mc_t4t_b_t4t, mc_t4t_b_b, row1)
addToData(mc_t4t_gf_t4t, mc_t4t_gf_gf, row1)
row2 = []
row2.append("-")
addToData(mc_fp_fp_row, mc_fp_fp_column, row2)
addToData(mc_fp_b_fp, mc_fp_b_b, row2)
addToData(mc_fp_gf_fp, mc_fp_gf_gf, row2)
row3 = []
row3.append("-")
row3.append("-")
addToData(mc_b_b_row, mc_b_b_column, row3)
addToData(mc_b_gf_b, mc_b_gf_gf, row3)
row4 = []
row4.append("-")
row4.append("-")
row4.append("-")
addToData(mc_gf_gf_row, mc_gf_gf_column, row4)
data.append(row1)
data.append(row2)
data.append(row3)
data.append(row4)

fig, ax = plotter.subplots()
fig.patch.set_visible(False)
ax.axis('off')
ax.axis('tight')
ax.set_title("Movie Coordination")

table = plotter.table(rowLabels=agentNames, colLabels=agentNames, cellText=data, loc='center')
fig.tight_layout()
plotter.savefig('mc_table.png')