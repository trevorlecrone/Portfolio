'''
sat_interface.py

A wrapper module for satinstance

Makes it easy to create a Knowledge Base (KB) object in python and 
give it clauses of the form
 A ~B
 ~A C
etc.

as a list of strings.

Once a KB object has been created, you can
    * test for satisfiability of the KB using is_satisfiable()
    * add another clause using add_clause()
    * test a single literal for satisfiability using test_literal()

by Andy Exley
Nov 2020
'''


from satinstance import SATInstance
from solvers import iterative_sat
from solvers.watchlist import setup_watchlist

class KB:
    def __init__(self, clauselist):
        '''Creates a knowledge base from a list of clauses

        clauselist must be a list of strings of the form
            ["A B C", "A ~B D", "~X ~Y",...
            etc.
            the letters need not be sequential.
            there must be whitespace between literals
        '''
        self.clauses = clauselist
        self.dirty = True
        self.answer = None

    def is_satisfiable(self):
        if self.dirty:
            self.dirty = False
            instance = SATInstance(self.clauses)
            n = len(instance.variables)
            watchlist = setup_watchlist(instance)
            if not watchlist:
                return False
            assignment = [None] * n
            self.answers = iterative_sat.solve(instance, watchlist, assignment, 0, False)
        count = 0
        for assignment in self.answers:
            count += 1

        if count == 0:
            return False
        else:
            return True

    def add_clause(self, clause):
        '''Adds the given clause to this KB
        '''
        self.clauses.append(clause)
        self.dirty = True
    
    def test_literal(self, literal):
        '''Creates a copy of this Knowledge Base with the given literal added,
        then tests to see if the new KB is satisfiable.

        If the new KB is not satisfiable, then we know that the original 
            KB |= ~literal
        '''
        nk = KB(self.clauses + [literal])
        return nk.is_satisfiable()

