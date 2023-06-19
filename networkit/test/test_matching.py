#!/usr/bin/env python3

import random
import unittest
from copy import copy

import networkit as nk

class TestMatching(unittest.TestCase):

	def setUp(self):
		self.g = nk.readGraph("input/PGPgiantcompo.graph", nk.Format.METIS)
		self.gw = copy(self.g)
		nk.graphtools.randomizeWeights(self.gw)
	
	def hasUnmatchedNeighbors(self, g, m):
		for e in g.iterEdges():
			if not m.isMatched(e[0]) and not m.isMatched(e[1]):
				return True
		return False

	def testPathGrowingMatcher(self):
		def runAlgo(g):
			pgm = nk.matching.PathGrowingMatcher(self.g)
			pgm.run()
			m = pgm.getMatching()

		runAlgo(self.g)
		runAlgo(self.gw)

	def testPathGrowingMatcherEdgeScores(self):
		G = nk.Graph(5)
		G.addEdge(0,1)
		G.addEdge(2,1)
		G.addEdge(3,1)
		G.addEdge(2,4)
		G.indexEdges()
		edgeScores = [0.5, 1.5, 1.0, 2.0]
		pgmes = nk.matching.PathGrowingMatcher(G, edgeScores)
		pgmes.run()
		self.assertEqual(len(pgmes.getMatching().getVector()), 5)
	
	def testSuitorMatcher(self):

		def doTest(g):
			m1 = nk.matching.SuitorMatcher(g, False).run().getMatching()
			nk.graphtools.sortEdgesByWeight(g, True)
			self.assertTrue(m1.isProper(g))
			self.assertFalse(self.hasUnmatchedNeighbors(g, m1))

			m2 = nk.matching.SuitorMatcher(g, True).run().getMatching()
			self.assertTrue(m2.isProper(g))
			self.assertFalse(self.hasUnmatchedNeighbors(g, m2))
			for u in g.iterNodes():
				self.assertEqual(m1.mate(u), m2.mate(u))

		doTest(self.g)
		doTest(self.gw)

	def testMatching(self):
		#test all functions of abstract Matching class 
		m1 = nk.matching.Matching(4)
		G = nk.Graph(4)
		G.addEdge(0,1) # G: 0 -- 1
		G.addEdge(0,2) #	|
		G.addEdge(2,3) #	2 -- 3
		m1.match(0,1)
		m1.match(2,3)
		self.assertTrue(m1.isProper(G))
		self.assertTrue(m1.areMatched(0,1))
		self.assertTrue(m1.isMatched(0))
		self.assertTrue(m1.isMatched(1))
		self.assertEqual(m1.size(G), 2)
		self.assertEqual(m1.mate(1), 0)
		self.assertEqual(m1.weight(G), 2.0)
		G2=nk.Graph(2)#size of matching
		Par=m1.toPartition(G2)
		self.assertEqual(len(Par), 2)
		self.assertListEqual(m1.getVector(), [1, 0, 3, 2])
		m1.unmatch(0,1)
		self.assertFalse(m1.areMatched(0,1))	

if __name__ == "__main__":
	unittest.main()
