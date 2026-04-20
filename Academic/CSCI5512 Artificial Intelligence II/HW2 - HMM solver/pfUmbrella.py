import argparse
import random

#--------Classes, Helpers and Constants------------------------

transition_matrix = [[0.7, 0.3], [0.3, 0.7]]
sensor_matrix = [[0.9, 0.1], [0.2, 0.8]]

class WeightedSample:
    #contains previous state value, 1 for true, 0 for false, and a weight
    def __init__(self, state: list, weight: float,) :
        self.state = state
        self.weight = weight

def validate_args(num_samples, num_steps, evidence) :
    if type(num_steps) == int and type(num_samples) == int :
      return num_steps == len(evidence)
    return False

def sample_state(prev_state) :
    rand = random.random()
    if(rand < transition_matrix[1 - prev_state][0]) :
        return 1
    return 0

def sample_state_from_weights(weight_dist) :
    rand = random.random()
    if(rand < weight_dist[0]) :
        return 1
    return 0


#------------------MAIN-------------------------------
parser = argparse.ArgumentParser()
parser.add_argument('num_samples', type=int)
parser.add_argument('num_steps', type=int)
parser.add_argument('evidence', metavar='N', type=int, nargs='+', help='evidence')
args = parser.parse_args()
num_samples = args.num_samples
num_steps = args.num_steps
evidence = args.evidence

if not validate_args(num_samples, num_steps, evidence) :
    print("provided argument for evidence length must be an int and be equal to the number of discreet evidence points given")
else :
    samples = []
    #initialize data with prior
    for i in range(num_samples) :
        initialState = 1 if random.random() > 0.5 else 0
        sample = WeightedSample(1, initialState)
        samples.append(sample)
    #compute for each timestep
    for  i in range(num_steps) :
        #initial sample and assign weight
        for j in range(num_samples) :
            state_from_sample = sample_state(samples[j].state)
            samples[j].state = state_from_sample
            samples[j].weight = sensor_matrix[1-samples[j].state][1-evidence[i]]
        #normalize weights
        weight_dist = [0, 0]
        total_weight = 0
        for j in range(num_samples) :
            sample = samples[j]
            total_weight += sample.weight
            weight_dist[1-sample.state] += sample.weight
        weight_dist = [weight_dist[0]/total_weight, weight_dist[1]/total_weight]
        #re-sample
        sample_sum = 0
        for j in range(num_samples) :
            state_from_sample = sample_state_from_weights(weight_dist)
            samples[j].state = state_from_sample
            sample_sum += state_from_sample
            samples[j].weight = 0
        num_target_state = 0
        for j in range(num_samples) :
            if(samples[j].state == 1) :
                num_target_state += 1
    prob_target_state = num_target_state/num_samples
    print(prob_target_state)
        
            

