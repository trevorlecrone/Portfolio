import argparse
import random
import copy

#--------Classes, Helpers and Constants------------------------

transition_matrix = [[0.7, 0.3], [0.4, 0.6]]
sensor_matrix = [[0.9, 0.1], [0.3, 0.7]]

class StateMessage:
    #contains previous state value, 1 for true, 0 for false, and -1 for root/prior, int denoting current state val, and val of probability of this state
    def __init__(self, prev_state: int, state: int, val: float,) :
        self.prev_state = prev_state
        self.state = state
        self.val = val

def validate_length_of_evidence (len_evidence, evidence) :
    if type(len_evidence) == int :
      return len_evidence == len(evidence)
    return False
  
def generate_message(prev_state_objects, current_state_val, evidence) :
    evidence_index = 1 - evidence
    state_index = 1 - current_state_val
    #generate a probablility for the new state using each of the previous state values, the transition and sensor matrixes and take the higher value
    v1 = prev_state_objects[0].val * transition_matrix[1 - prev_state_objects[0].state][state_index] * sensor_matrix[state_index][evidence_index]
    v2 = prev_state_objects[1].val * transition_matrix[1 - prev_state_objects[1].state][state_index] * sensor_matrix[state_index][evidence_index]
    prev_state = 0
    prob_state = v1
    if(v2 > v1) :
        prev_state = 1
        prob_state = v2
    return StateMessage(prev_state, current_state_val, round(prob_state, 7))

def get_sting_key(i, j) :
    return str(i) + ","+str(j)

def print_most_likely_sequence(messages, len_evidence) :
    final_false_message = messages[get_sting_key(len_evidence, 0)]
    final_true_message = messages[get_sting_key(len_evidence, 1)]
    #look at which of the two final states is more likely
    target_message = final_false_message if final_false_message.val > final_true_message.val else final_true_message
    most_likely_sequence = [target_message.state]
    prev_state = target_message.prev_state
    steps_back = 1
    while len_evidence - steps_back > 0 :
        #chose the state most likely to bring us to the current state, add the current state to the front of the sequence, and recurse by going to the most likely previous state
        target_message = messages[get_sting_key(len_evidence - steps_back, prev_state)]
        most_likely_sequence.insert(0, target_message.state)
        steps_back += 1
        prev_state = target_message.prev_state
    print(most_likely_sequence)




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
    message_map = {}
    prior = [0.5,0.5]
    messages = [StateMessage(-1, 0, 0.5), StateMessage(-1, 1, 0.5)]
    tempMessages = copy.deepcopy(messages)
    #do forward messaging/filtering
    for i in range(1, len_evidence + 1) :
        for j in range(2) :
            tempMessages[j] = generate_message(messages, j, evidence[i-1])
            message_map[get_sting_key(i,j)] = tempMessages[j]
        messages = copy.deepcopy(tempMessages)
    print_most_likely_sequence(message_map, len_evidence)
    