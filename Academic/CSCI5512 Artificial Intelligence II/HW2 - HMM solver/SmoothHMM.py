import argparse
import random

#--------Helpers and Constants------------------------

transition_matrix = [[0.7, 0.3], [0.4, 0.6]]
sensor_matrix = [[0.9, 0.1], [0.3, 0.7]]

def validate_length_of_evidence (len_evidence, evidence) :
    if type(len_evidence) == int :
      return len_evidence == len(evidence)
    return False

def array_scalar_mult(array, scalar) :
    result = []
    for i in range(len(array)):
       result.append(array[i] * scalar)
    return result

def mult_arrays(array, array2) :
    result = []
    for i in range(len(array)):
        result.append(array[i] * array2[i])
    return result

def add_arrays(array, array2) :
    result = []
    for i in range(len(array)):
       result.append(array[i] + array2[i])
    return result

def normalize(array) :
    result = []
    total = array[0] + array[1]
    for i in range(len(array)):
       result.append(array[i]/total)
    return result
  
def forward(prev_prob_dist, evidence) :
    evidence_index = 1 - evidence
    a1 = array_scalar_mult(transition_matrix[0], prev_prob_dist[0])
    a2 = array_scalar_mult(transition_matrix[1], prev_prob_dist[1])
    intermediate = add_arrays(a1, a2)
    unnormalized = mult_arrays(intermediate, [sensor_matrix[0][evidence_index], sensor_matrix[1][evidence_index]])
    return normalize(unnormalized)

def backward(message, evidence) :
    evidence_index = 1 - evidence
    a1 = array_scalar_mult([transition_matrix[0][0], transition_matrix[1][0]], (sensor_matrix[0][evidence_index] * message[0]))
    a2 = array_scalar_mult([transition_matrix[0][1], transition_matrix[1][1]], (sensor_matrix[1][evidence_index] * message[1]))
    return add_arrays(a1, a2)

def print_smoothed_dist(smoothed, len_evidence) :
    for i in range(1, len_evidence + 1) :
        print(str(i) + ":" + str(smoothed[i]))

def print_smoothed_p_true(smoothed, len_evidence) :
    for i in range(1, len_evidence + 1) :
        print(str(i) + ":" + str(round(smoothed[i][0], 7)))
#------------------MAIN-------------------------------
parser = argparse.ArgumentParser()
parser.add_argument('len_evidence', type=int)
parser.add_argument('evidence', metavar='N', type=int, nargs='+', help='evidence')
args = parser.parse_args()
len_evidence = args.len_evidence
evidence = args.evidence
if not validate_length_of_evidence (len_evidence, evidence) :
    print("provided argument for evidence length must be an int and be equal to the number of discreet evidence points given")
else :
    #setup variables and follow book's psudocode algorithm
    forward_list = {}
    message = [1,1]
    prior = [0.5,0.5]
    forward_list[0] = prior
    smoothed_estimates = {}
    #do forward messaging/filtering
    for i in range(1, len_evidence + 1) :
        forward_list[i] = (forward(forward_list[i-1], evidence[i-1]))
    #do backward messaging
    for i in range(len_evidence) :
        smoothed_estimates[len_evidence - i] = normalize(mult_arrays(forward_list[len_evidence-i], message))
        message = backward(message, evidence[len_evidence-1-i])
    print_smoothed_p_true(smoothed_estimates, len_evidence)
