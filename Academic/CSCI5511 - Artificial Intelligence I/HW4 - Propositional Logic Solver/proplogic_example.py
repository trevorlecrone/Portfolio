import sat_interface

def tt1():
    '''
    Example of how to interact with sat_interface module

    Propositions:
        A: Amy is a truth-teller
        B: Bob is a truth-teller
        C: Cal is a truth-teller

    return a list containing all entailed propositions or negated propositions
    '''
    print("Truth-tellers and liars I")
    print("-------------------------")
    ttprob = sat_interface.KB(["~A ~B",
                                "B A",
                                "~B ~C",
                                "C B",
                                "~C ~A",
                                "~C ~B",
                                "A B C"])

    entailed = []
    if ttprob.test_literal("A") == False:
        entailed.append(False)
        print("Amy is a liar")
    if ttprob.test_literal("~A") == False:
        entailed.append(True)
        print("Amy is a truth-teller")
    if ttprob.test_literal("B") == False:
        entailed.append(False)
        print("Bob is a liar")
    if ttprob.test_literal("~B") == False:
        entailed.append(True)
        print("Bob is a truth-teller")
    if ttprob.test_literal("C") == False:
        entailed.append(False)
        print("Cal is a liar")
    if ttprob.test_literal("~C") == False:
        entailed.append(True)
        print("Cal is a truth-teller")
    print("-------------------------")
    return entailed

def main():
    print(tt1())

if __name__ == '__main__':
    main()
