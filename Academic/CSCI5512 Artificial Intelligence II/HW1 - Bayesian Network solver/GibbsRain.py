import argparse
import random

#--------Helpers and Constants------------------------

#don't actually need to use compliments in the code
prob_c__r_w_s = 0.444
#prob_nc__r_w_s = 0.556

prob_c__nr_w_s = 0.048
#prob_nc__nr_w_s = 0.952

prob_r__c_w_s = 0.815
#prob_nn__c_w_s = 0.185

prob_r__nc_w_s = 0.216
#prob_nr__nc_w_s = 0.784

def validate_num_steps (num_steps) :
    if type(num_steps) == int :
      return True
    return False

def r_true_in_state(state) :
    if state[0] > 0 :
      return True
    return False

def sample_c_and_update_state(state) :
    if state[0] > 0 :
      if random.random() <= prob_c__r_w_s :
        state[1] = 1
      else :
         state[1] = 0
    else :
      if random.random() <= prob_c__nr_w_s :
         state[1] = 1
      else :
         state[1] = 0
    return state

def sample_r_and_update_state(state) :
    if state[1] > 0 :
      if random.random() <= prob_r__c_w_s :
        state[0] = 1
      else :
         state[0] = 0
    else :
      if random.random() <= prob_r__nc_w_s :
         state[0] = 1
      else :
         state[0] = 0
    return state
  
#------------------MAIN-------------------------------
parser = argparse.ArgumentParser()
parser.add_argument('num_steps', nargs='?')
args = parser.parse_args()
num_steps = args.num_steps
if (num_steps == None) :
    num_steps = 100
if not validate_num_steps (num_steps) :
    print("provided arg for num_steps is not an int, provide a valid int for num_steps")
else :
    #will not track S and W since they never change, state is a representation of [R, C] where 1 indicates a true value, and 0 indicates a false value
    num_steps = int(num_steps)
    r = random.randint(0,1)
    c = random.randint(0,1)
    num_states_with_r_true = 0
    state = [r, c]
    for step in range(num_steps) :
      if(step % 2 == 0) :
        state = sample_c_and_update_state(state)
      else :
        state = sample_r_and_update_state(state)
      if state[0] > 0 :
        num_states_with_r_true += 1
    prob_r_given_samples = num_states_with_r_true / num_steps
    print("probability of rain given sprinkler is running and grass is wet is estimated to be " + str(prob_r_given_samples) + " given " + str(num_steps) + " samples")