#pragma once

#include "ofMesh.h"
#include "PatternDef.h"

#include <set>
#include "PatternGraph.h"

namespace ami
{
	class PatternMesh
	{
	public:
		PatternMesh() {};
		PatternMesh(const PatternGraph & graph);

		void update(float deltaTime);

		void draw();
	private:
		struct Properties
		{
			bool isFix;
		};

		void addTriangle(ofIndexType tri0, ofIndexType tri1, ofIndexType tri2);
		void setDistanceConstrain(ofIndexType a, ofIndexType b, float distance);
		void setAngleConstrain(ofIndexType a, ofIndexType b, float degrees);
		void solveConstraints();
		void computeForces();
		void verletUpdate(float deltaTime);
		void updateCenter();
		void updateNormals();

		ofMesh m_mesh;

		glm::vec3 m_center;

		std::map <ofIndexType, glm::vec3> m_expansionForce;
		std::map <ofIndexType, Properties> m_properties;
		std::map <ofIndexType, std::set<std::pair<ofIndexType, float>>> m_con;
		std::map <ofIndexType, std::set<std::pair<ofIndexType, float>>> m_soft_con;

		std::vector<glm::vec3> m_oldVec;

		unsigned int m_roundNum;
		float m_pointDistance;
		float m_minTension;
		float m_damping;
		unsigned int m_solveIterations;
	};
}