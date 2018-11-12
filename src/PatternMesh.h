#pragma once

#include "ofMesh.h"
#include "PatternDef.h"

#include <set>
#include <unordered_map>
#include "PatternGraph.h"

namespace ami
{
	class PatternMesh
	{
	public:
		struct Anchor
		{
			ofIndexType node;
			glm::vec3 anchor;
		};

		struct DrawSettings
		{
			bool id = false;
			bool forces = false;
			bool outline = false;
		};

		PatternMesh() {};
		PatternMesh(const PatternGraph & graph, const std::vector<Anchor> & anchors);

		void update(float deltaTime);

		void draw(const PatternMesh::DrawSettings & settings);
	private:
		struct Properties
		{
			bool isFix;
			unsigned int id;
			Operation::Type type;
		};

		void setDistanceConstrain(ofIndexType a, ofIndexType b, float distance);
		void setAngleConstrain(ofIndexType a, ofIndexType b, float degrees);
		void setJoint(ofIndexType a, ofIndexType b);
		void solveConstraints();
		void computeForces();
		void verletUpdate(float deltaTime);
		void updateCenter();
		void updateNormals();

		ofMesh m_mesh;

		glm::vec3 m_center;

		std::unordered_map <ofIndexType, glm::vec3> m_expansionForce;
		std::unordered_map <ofIndexType, Properties> m_properties;
		std::unordered_map <ofIndexType, std::set<std::pair<ofIndexType, float>>> m_con;
		std::unordered_map <ofIndexType, std::set<ofIndexType>> m_joints;
		std::unordered_map <ofIndexType, std::set<std::pair<ofIndexType, float>>> m_soft_con;
		std::vector<ofIndexType> m_outline;
		
		std::vector<Anchor> m_anchors;

		std::vector<glm::vec3> m_oldVec;

		unsigned int m_roundNum;
		float m_pointDistance;
		float m_minTension;
		float m_damping;
		unsigned int m_solveIterations;
	};
}