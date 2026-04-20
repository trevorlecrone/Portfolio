import mdp
import argparse
import copy

#------------------MAIN-------------------------------
parser = argparse.ArgumentParser()
parser.add_argument('reward', type=float)
parser.add_argument('discount', type=float)
args = parser.parse_args()
reward = args.reward
discount = args.discount

initialMDPStates = mdp.InitializeStandardStates(reward)
initialMDP = mdp.MDP(4, 3, initialMDPStates)
initialMDP.InitializeValueFunction(1.0)
initialMDP.InitializePolicy()

currentMDP = initialMDP
newMDP = initialMDP
while True :
    newMDP = copy.deepcopy(currentMDP)
    #large n to ensure convergence
    for i in range(10000) :
        for state in newMDP.states.values() :
            if(state.reachable and not state.terminal) :
                val = newMDP.ActionExpectedVal(state.column, state.row, state.policy, discount)
                state.value = val
        #escape early if we have converged for efficiency sake
        if(mdp.CheckConvergence(newMDP, currentMDP)) :
            break
    for state in newMDP.states.values() :
        if(state.reachable and not state.terminal) :
            maxVal = -10000000000
            for action in newMDP.AvailableActions(state.column, state.row) :
                val = newMDP.ActionExpectedVal(state.column, state.row, action, discount)
                if(val > maxVal) :
                    maxVal = val
                    state.policy = action
    if(mdp.CheckConvergence(newMDP, currentMDP)) :
        break
    else :
        currentMDP = copy.deepcopy(newMDP)

newMDP.PrintStates()