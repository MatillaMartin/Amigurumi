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

		void setup(float pointDistance);

		void update(float deltaTime);

		void addRound(Operation::Operations op);
		void addOperation(Operation::Type type, unsigned int round, unsigned int roundIndex, unsigned int roundSize);

		void draw();
	private:

		void insertNextVertex(unsigned int round, unsigned int roundIndex, unsigned int roundSize);
		void addTriangle(ofIndexType tri0, ofIndexType tri1, ofIndexType tri2);
		void mergeVertices(ofIndexType a, ofIndexType b);
		void computeTension();
		void updateNormals();

		ofMesh m_mesh;
		
		ofIndexType m_behind;
		ofIndexType m_lastUnder;

		std::map <ofIndexType, vector<ofVec3f>> m_constraintTensions;
		std::map <ofIndexType, ofVec3f> m_constraintTension, m_expansionTension;
		std::map <ofIndexType, std::set<std::pair<ofIndexType, float>>> m_con;

		unsigned int m_roundNum;
		float m_pointDistance;
		float m_minTension;
	};
}