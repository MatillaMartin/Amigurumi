#pragma once

#include "ofMesh.h"
#include "PatternDef.h"

#include <set>

namespace ami
{
	class PatternMesh
	{
	public:
		PatternMesh();

		void setup();
		void update(float deltaTime);

		void addRound(const Operation::Operations & op);
		void addOperation(Operation::Type type, unsigned int round, unsigned int roundIndex, unsigned int roundSize);

		void draw();
	private:

		void insertNextVertex(unsigned int round, unsigned int roundIndex, unsigned int roundSize);
		void addTriangle(ofIndexType tri0, ofIndexType tri1, ofIndexType tri2);
		void mergeVertices(ofIndexType a, ofIndexType b);
		void solveConstraints();
		void computeForces();
		void solveAnchors();
		void verletUpdate(float deltaTime);
		void updateCenter();
		void updateNormals();

		ofMesh m_mesh;
		
		ofIndexType m_behind;
		ofIndexType m_lastUnder;
		ofVec3f m_center;

		std::map <ofIndexType, ofVec3f> m_expansionForce;
		std::map <ofIndexType, std::set<std::pair<ofIndexType, float>>> m_fix;
		std::map <ofIndexType, std::set<std::pair<ofIndexType, float>>> m_con;

		std::vector<ofVec3f> m_oldVec;

		unsigned int m_roundNum;
		float m_pointDistance;
		float m_minTension;
		float m_damping;
		unsigned int m_solveIterations;
	};
}