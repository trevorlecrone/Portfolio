import gymnasium as gym
import random
import pickle
import matplotlib.pyplot as plotter
import argparse
import copy
#------------------CONSTANTS & Helpers----------------

SOUTH = 0
NORTH = 1
EAST = 2
WEST = 3
PICKUP = 4
DROPOFF = 5

ACTIONS = [SOUTH, NORTH, EAST, WEST, PICKUP, DROPOFF]


def GetOptimalAction(qvals: map, state: int) :
    maxVal = -100000
    optimalAction = None
    for action in ACTIONS :
        val = qvals[state][action]
        if(val > maxVal) :
            maxVal = val
            optimalAction = action
    return optimalAction

def UpdateQval(qvals: map, state: int, action: int, reward: int, state2: int, action2: int, learningRate: float, discount: float):
    qvals[state][action] = qvals[state][action] + (learningRate * (reward + (discount * qvals[state2][action2]) - qvals[state][action]))

def EGreedySelect(qvals: map, state: int, policy: map, epsilon: float) :
    nonOptimalActions = copy.deepcopy(ACTIONS)
    optimalAction = GetOptimalAction(qvals, state)
    policy[state] = optimalAction
    nonOptimalActions.remove(optimalAction)
    randVal = random.random()
    if(randVal > epsilon) :
        return optimalAction
    else :
        return nonOptimalActions[random.randrange(5)]

#not really helpful since not every state is always updated, fixed number of episodes works fine       
def CheckConvergence(qvals: map, oldQVals: map):
    for state in range(500) :
        for action in ACTIONS :
            if not oldQVals[state][action] == qvals[state][action] :
                return False
    return True

#------------------MAIN-------------------------------
parser = argparse.ArgumentParser()
parser.add_argument('epsilon', type=float)
parser.add_argument('learningRate', type=float)
parser.add_argument('discount', type=float)
args = parser.parse_args()
epsilon = args.epsilon
learningRate = args.learningRate
discount = args.discount

#set up vars
qvals = {}
oldQVals = {}
policy = {}
totalReward = 0.0
cumulativeRewards = {}
#initialize qvals
for i in range(500) :
    qvals[i] = {}
    for action in ACTIONS :
        qvals[i][action] = 1

#initialize policy
for i in range(500) :
    policy[i] = SOUTH

#initialize environment
env = gym.make("Taxi-v3")
startState, info = env.reset()
terminal = False
currentState = startState
action = EGreedySelect(qvals, currentState, policy, epsilon)
#converged = False

#SARSA agent
for i in range(3000) :
    while not terminal :
        oldState = currentState
        currentState, reward, terminated, truncated, info = env.step(action)
        totalReward += reward
        action2 = EGreedySelect(qvals, currentState, policy, epsilon)
        UpdateQval(qvals, oldState, action, reward, currentState, action2, learningRate, discount)
        action = action2
        terminal = terminated or truncated
    #converged = CheckConvergence(qvals, oldQVals)
    currentState, info = env.reset()
    terminal = False
    if(i % 250 == 0) :
        epsilon *= 0.8
    cumulativeRewards[i + 1] = totalReward / (i + 1)

# gather random agent data
rand_totalReward = 0.0
rand_cumulativeRewards = {}
terminal = False
for j in range(3000):
    observation, info = env.reset()
    while not terminal :
        action = env.action_space.sample()  # sample a random action
        observation, reward, terminated, truncated, info = env.step(action)
        rand_totalReward += reward
        terminal = terminated or truncated
    terminal = False
    rand_cumulativeRewards[j + 1] = rand_totalReward / (j + 1)

print("hi")
#dump to pickle files
qVals_filehandler = open("sarsa_q_vals.pickle", 'wb')
pickle.dump(qvals, qVals_filehandler)
policy_filehandler = open("sarsa_policy.pickle", 'wb')
pickle.dump(policy, policy_filehandler)

#plot cumulative reward data
sarsa_x = cumulativeRewards.keys()
sarsa_y = cumulativeRewards.values()
plotter.plot(sarsa_x, sarsa_y, label = "SARSA agent")

rand_x = rand_cumulativeRewards.keys()
rand_y = rand_cumulativeRewards.values()
plotter.plot(rand_x, rand_y, label = "random agent")

plotter.xlabel('Episode')
# naming the y axis
plotter.ylabel('Agent total reward per episode')
# giving a title to my graph
plotter.title('SARSA vs. Random agent total reward per episode')

plotter.legend()

plotter.savefig('sarsa_total_reward.png')

env.close()