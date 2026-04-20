import sat_interface

def tt2() :
    print("Truth-tellers and liars II")
    print("-------------------------")
    #did not include (A ~A) and (~A ~C A) clauses from first sentance, they are always true
    tt2prob = sat_interface.KB(["~A C",
                                "~B ~C",
                                "C B",
                                "~C B ~A",
                                "~B C",
                                "C A"])
    entailed = []
    if tt2prob.test_literal("A") == False:
        entailed.append(False)
        print("Amy is a liar")
    if tt2prob.test_literal("~A") == False:
        entailed.append(True)
        print("Amy is a truth-teller")
    if tt2prob.test_literal("B") == False:
        entailed.append(False)
        print("Bob is a liar")
    if tt2prob.test_literal("~B") == False:
        entailed.append(True)
        print("Bob is a truth-teller")
    if tt2prob.test_literal("C") == False:
        entailed.append(False)
        print("Cal is a liar")
    if tt2prob.test_literal("~C") == False:
        entailed.append(True)
        print("Cal is a truth-teller")
    print("-------------------------")
    return entailed

def tt3() :
    print("-------------------------")
    print("Truth-tellers and liars III")
    print("-------------------------")
    tt2prob = sat_interface.KB(["~A ~C",
                                "A C",
                                "~B A",
                                "~B C",
                                "~A ~C B",
                                "~C B",
                                "~B C"])
    entailed = []
    if tt2prob.test_literal("A") == False:
        entailed.append(False)
        print("Amy is a liar")
    if tt2prob.test_literal("~A") == False:
        entailed.append(True)
        print("Amy is a truth-teller")
    if tt2prob.test_literal("B") == False:
        entailed.append(False)
        print("Bob is a liar")
    if tt2prob.test_literal("~B") == False:
        entailed.append(True)
        print("Bob is a truth-teller")
    if tt2prob.test_literal("C") == False:
        entailed.append(False)
        print("Cal is a liar")
    if tt2prob.test_literal("~C") == False:
        entailed.append(True)
        print("Cal is a truth-teller")
    print("-------------------------")
    return entailed


def salt() :
    print("-------------------------")
    print("Robbery and a Salt")
    A = "Caterpillar stole the salt"
    B = "Bill the Lizard stole the salt"
    C = "Cheshire cat stole the salt"
    D = "Caterpillars statement"
    E = "Bill The Lizard's Statement"
    F = "Cheshire Cat's Statement"
    print("-------------------------")
    saltprob = sat_interface.KB(["~A ~B"
                                "~A ~C",
                                "A B C",
                                "~B ~A",
                                "~B ~C",
                                "B A C",
                                "~C ~A",
                                "~C ~B",
                                "C A B",
                                "D E F",
                                "~D ~E ~F",
                                "~D B",
                                "~B D",
                                "~E D",
                                "~D E",
                                "~F ~C",
                                "F C"])
    entailed = {}
    if saltprob.test_literal("A") == False:
        entailed[A] = False
        print(A +": is false")
    if saltprob.test_literal("~A") == False:
        entailed[A] = True
        print(A +": is true")
    if saltprob.test_literal("B") == False:
        entailed[B] = False
        print(B +": is false")
    if saltprob.test_literal("~B") == False:
        entailed[B] = True
        print(B +": is true")
    if saltprob.test_literal("C") == False:
        entailed[C] = False
        print(C +": is false")
    if saltprob.test_literal("~C") == False:
        entailed[C] = True
        print(C +": is true")
    if saltprob.test_literal("D") == False:
        entailed[D] = False
        print(D +": is false")
    if saltprob.test_literal("~D") == False:
        entailed[D] = True
        print(D +": is true")
    if saltprob.test_literal("E") == False:
        entailed[E] = False
        print(E +": is false")
    if saltprob.test_literal("~E") == False:
        entailed[E] = True
        print(E +": is true")
    if saltprob.test_literal("F") == False:
        entailed[F] = False
        print(F +": is false")
    if saltprob.test_literal("~F") == False:
        entailed[F] = True
        print(F +": is true")
    print("-------------------------")
    return entailed

def golf() :
    print("-------------------------")
    print("An honest name")
    T1 = "The first man is Tom"
    T2 = "The second man is Tom"
    T3 = "The third man is Tom"
    D1 = "The first man is Dick"
    D2 = "The second man is Dick"
    D3 = "The third man is Dick"
    H1 = "The first man is Harry"
    H2 = "The second man is Harry"
    H3 = "The third man is Harry"
    TR1 = "The first man told the truth"
    TR2 = "The second man told the truth"
    TR3 = "The third man told the truth"
    print("-------------------------")
    golfprob = sat_interface.KB(["~T1 ~T2",
                                "~T1 ~T3",
                                "T1 T2 T3",
                                "~T1 ~D1",
                                "~T1 ~H1",
                                "T1, D1, H1",
                                "~D1 ~D2",
                                "~D1 ~D3",
                                "D1 D2 D3",
                                "~T1 ~D1",
                                "~D1 ~H1",
                                "T1, D1, H1",
                                "~H1 ~H2",
                                "~H1 ~H3",
                                "H1 H2 H3",
                                "~T1 ~H1",
                                "~D1 ~H1",
                                "T1, D1, H1",
                                "~T1 ~T2",
                                "~T2 ~T3",
                                "T1 T2 T3",
                                "~T2 ~D2",
                                "~T2 ~H2",
                                "T2, D2, H2",
                                "~D1 ~D2",
                                "~D2 ~D3",
                                "D1 D2 D3",
                                "~T2 ~D2",
                                "~D2 ~H2",
                                "T2, D2, H2",
                                "~H1 ~H2",
                                "~H2 ~H3",
                                "H1 H2 H3",
                                "~T2 ~H2",
                                "~D2 ~H2",
                                "T2, D2, H2",
                                "~T1 ~T3",
                                "~T2 ~T3",
                                "T1 T2 T3",
                                "~T3 ~D3",
                                "~T3 ~H3",
                                "T3, D3, H3",
                                "~D1 ~D3",
                                "~D2 ~D3",
                                "D1 D2 D3",
                                "~T3 ~D3",
                                "~D3 ~H3",
                                "T3, D3, H3",
                                "~H1 ~H3",
                                "~H2 ~H3",
                                "H1 H2 H3",
                                "~T3 ~H3",
                                "~D3 ~H3",
                                "T3, D3, H3",
                                "~TR1 ~H1",
                                "H1 TR1",
                                "~TR2 ~H2",
                                "H2 TR2",
                                "~TR2 ~H2",
                                "H2 TR2",
                                "~TR1 H2",
                                "TR1 ~H2",
                                "~TR2 D2",
                                "TR2 ~D2",
                                "~TR3 T2",
                                "TR3 ~T2",
                                "~T1 ~TR3",
                                "~T1 ~TR2",
                                "T1 TR3 TR2",
                                "~T2 ~TR1",
                                "~T2 ~TR3",
                                "T2 TR3 TR1",
                                "~T3 ~TR1",
                                "~T3 ~TR2",
                                "T3 TR1 TR2",
                                "H1 TR1 D1",
                                "H2 TR2 D2",
                                "H3 TR3 D3"])
    entailed = {}
    if golfprob.test_literal("T1") == False:
        entailed[T1] = False
        #print(T1 +": is false")
    if golfprob.test_literal("~T1") == False:
        entailed[T1] = True
        print(T1)
    if golfprob.test_literal("T2") == False:
        entailed[T2] = False
        #print(T2 +": is false")
    if golfprob.test_literal("~T2") == False:
        entailed[T2] = True
        print(T2 )
    if golfprob.test_literal("T3") == False:
        entailed[T3] = False
        #print(T3 +": is false")
    if golfprob.test_literal("~T3") == False:
        entailed[T3] = True
        print(T3)
    if golfprob.test_literal("D1") == False:
        entailed[D1] = False
        #print(D1 +": is false")
    if golfprob.test_literal("~D1") == False:
        entailed[D1] = True
        print(D1)
    if golfprob.test_literal("D2") == False:
        entailed[D2] = False
        #print(D2 +": is false")
    if golfprob.test_literal("~D2") == False:
        entailed[D2] = True
        print(D2)
    if golfprob.test_literal("D3") == False:
        entailed[D3] = False
        #print(D3 +": is false")
    if golfprob.test_literal("~D3") == False:
        entailed[D3] = True
        print(D3)
    if golfprob.test_literal("H1") == False:
        entailed[H1] = False
        #print(H1 +": is false")
    if golfprob.test_literal("~H1") == False:
        entailed[H1] = True
        print(H1)
    if golfprob.test_literal("H2") == False:
        entailed[H2] = False
        #print(H2 +": is false")
    if golfprob.test_literal("~H2") == False:
        entailed[H2] = True
        print(H2)
    if golfprob.test_literal("H3") == False:
        entailed[H3] = False
        #print(H3 +": is false")
    if golfprob.test_literal("~H3") == False:
        entailed[H3] = True
        print(H3)
    if golfprob.test_literal("TR1") == False:
        entailed[TR1] = False
        #print(TR1 +": is false")
    if golfprob.test_literal("~TR1") == False:
        entailed[TR1] = True
        print(TR1)
    if golfprob.test_literal("TR2") == False:
        entailed[TR2] = False
        print(TR2 +": is false")
    if golfprob.test_literal("~TR2") == False:
        entailed[TR2] = True
        print(TR2)
    if golfprob.test_literal("TR3") == False:
        entailed[TR3] = False
        print(TR3 +": is false")
    if golfprob.test_literal("~TR3") == False:
        entailed[TR3] = True
        print(TR3)
    print("-------------------------")
    return entailed

def main():
    print(tt2())
    print(tt3())
    print(salt())
    print(golf())

if __name__ == '__main__':
    main()


