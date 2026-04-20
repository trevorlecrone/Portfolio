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
#commented code was used to get true likelyhood of R10 = T in test runs
#def array_scalar_mult(array, scalar) :
#    result = []
#    for i in range(len(array)):
#       result.append(array[i] * scalar)
#    return result

#def mult_arrays(array, array2) :
#    result = []
#    for i in range(len(array)):
#        result.append(array[i] * array2[i])
#    return result

#def add_arrays(array, array2) :
#    result = []
#    for i in range(len(array)):
#       result.append(array[i] + array2[i])
#    return result

#def normalize(array) :
#    result = []
#    total = array[0] + array[1]
#    for i in range(len(array)):
#       result.append(array[i]/total)
#    return result

#def forward(prev_prob_dist, evidence) :
#    evidence_index = 1 - evidence
#    a1 = array_scalar_mult(transition_matrix[0], prev_prob_dist[0])
#    a2 = array_scalar_mult(transition_matrix[1], prev_prob_dist[1])
#    intermediate = add_arrays(a1, a2)
#    unnormalized = mult_arrays(intermediate, [sensor_matrix[0][evidence_index], sensor_matrix[1][evidence_index]])
#    return normalize(unnormalized)

def sample_state(prev_state) :
    rand = random.random()
    if(rand < transition_matrix[1 - prev_state][0]) :
        return 1
    return 0

def full_sample(prev_state, weight, evidence) :
    curr_state = sample_state(prev_state)
    weight = weight * sensor_matrix[1-curr_state][1-evidence]
    return WeightedSample(curr_state, weight)


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
    #generate all samples
    for i in range(num_samples) :
        initialState = 1 if random.random() > 0.5 else 0
        sample = WeightedSample(1, initialState)
        #go through all time steps
        for  j in range(num_steps) :
            sample = full_sample(sample.state, sample.weight, evidence[j])
        samples.append(sample)
    #normalize weights and return result
    weight_of_target_samples = 0
    weight_of_all_samples = 0
    for sample in samples :
        if sample.state == 1 :
            weight_of_target_samples += sample.weight
        weight_of_all_samples += sample.weight
    print(weight_of_target_samples/weight_of_all_samples)

    #forward_list = {}
    #message = [1,1]
    #prior = [0.5,0.5]
    #forward_list[0] = prior
    #for i in range(1, num_steps + 1) :
    #    forward_list[i] = (forward(forward_list[i-1], evidence[i-1]))
    #print(forward_list)
            

