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

		void addTriangle(ofIndexType tri0, ofIndexType tri1, ofIndexType tri2);
		void computeTension();
		void updateNormals();

		ofMesh m_mesh;
		
		ofIndexType m_behind;
		ofIndexType m_lastUnder;

		std::map <ofIndexType, glm::vec3> m_tension, m_idealTension, m_expansionTension;
		std::map <ofIndexType, std::set<ofIndexType>> m_con;

		unsigned int m_roundNum;
		float m_pointDistance;
	};
}