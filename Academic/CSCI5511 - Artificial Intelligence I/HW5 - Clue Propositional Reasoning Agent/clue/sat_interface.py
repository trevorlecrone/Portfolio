'''
sat_interface.py

A wrapper module for simple_sat 

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
        assert isinstance(clauselist, list)
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
            self.answer = iterative_sat.solve(instance, watchlist, assignment, 0, False)
        return self.answer

    def add_clause(self, clause):
        self.clauses.append(clause)
        self.dirty = True
    
    def test_literal(self, literal):
        '''Adds a single literal to the KB then tests to see if the KB is satisfiable
        '''
        nk = KB(self.clauses + [literal])
        return nk.is_satisfiable()

    def test_add_variable(self, variable):
        '''Tests a single variable against the KB by doing the following:
            uses test_literal on var
            uses test_literal on ~var
            if we find that either var or ~var is entailed by the KB, we add that to the clause

            return value:
                True if variable is entailed
                False if ~variable is entailed
                None if neither is entailed
        '''
        pv = self.test_literal(variable)
        nv = self.test_literal("~" + variable)
        if pv == True and nv == False:
            self.clauses.append(variable)
            return True
        elif pv == False and nv == True:
            self.clauses.append("~" + variable)
            return False
        else:
            return None
