#pragma once

#include "ofMesh.h"

#include "PatternDef.h"

namespace ami
{
	class PatternMesh
	{
	public:
		PatternMesh();

		void setup(float pointDistance);

		void addRound(Operation::Operations op);
		void addOperation(Operation::Type type);
		
		void draw();
	private:

		ofMesh m_mesh;
		ofIndexType m_behind;
		ofIndexType m_lastUnder;
		ofIndexType m_current;

		unsigned int m_roundNum;
		float m_pointDistance;
	};
}