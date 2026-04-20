from collections import defaultdict
import mdp
import argparse
import copy

def CheckPrevRoundsForStateActionTuple(state, action, prevRounds) :
    for i in range(len(prevRounds)) :
        if prevRounds[i]['state'] == state and prevRounds[i]['action'] == action :
            return True
    return False

#------------------MAIN-------------------------------
parser = argparse.ArgumentParser()
parser.add_argument('reward', type=float)
parser.add_argument('discount', type=float)
parser.add_argument('epsilon', type=float)
args = parser.parse_args()
reward = args.reward
discount = args.discount
epsilon = args.epsilon

initialMDPStates = mdp.InitializeStandardStates(reward)
initialMDP = mdp.MDP(4, 3, initialMDPStates)

currentMDP = initialMDP
newMDP = initialMDP

mcAgent = mdp.MDPMCAgent(1,1, currentMDP)
returns = defaultdict(list)
for i in range(2000000) :
    #agent stores the policy since it is probabilistic
    startingAgentPolicy = copy.deepcopy(mcAgent.policy)
    startingAgentValue = copy.deepcopy(mcAgent.value)
    #Generate episode and zero return
    episode = mcAgent.GenerateEpisode()
    episodeReturn = 0.0
    for i in range(len(episode)) :
        index = len(episode) - 1 - i
        episodeRound = episode[index]
        episodeReturn = (discount * episodeReturn) + episodeRound['reward']
        #if this is first visit
        prevRounds = episode[0:index]
        if(not CheckPrevRoundsForStateActionTuple(episodeRound['state'], episodeRound['action'], prevRounds)) :
            #add return to vector
            column = episodeRound['state'].column
            row = episodeRound['state'].row
            stateActionKey = mdp.GetKey(column, row) + "," + str(episodeRound['action'])
            if(stateActionKey not in returns.keys()) :
                returns[stateActionKey] = [episodeReturn]
            else :
                returns[stateActionKey].append(episodeReturn)
            #set value to average return
            mcAgent.value[stateActionKey] = round(sum(returns[stateActionKey]) / len(returns[stateActionKey]), 5)
            # find optimal action
            optimalAction = mcAgent.GetOptimalAction(column, row)
            # the value we need to generate a random decimal between 0 and 1 higher than, equal to 1.0 - probability we choose the optimal action
            optimalThreshold = 1 - (epsilon/len(currentMDP.AllActions()) + (1.0 - epsilon))
            nonOptimalActions = currentMDP.AllActions()
            nonOptimalActions.remove(optimalAction)
            mcAgent.policy[mdp.GetKey(column, row)] = dict(optimalThreshold=optimalThreshold, optimalAction=optimalAction, nonOptimalActions=nonOptimalActions)

mcAgent.Print()