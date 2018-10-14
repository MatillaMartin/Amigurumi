#include "PatternMesh.h"

#include "ofGraphics.h"

#include <numeric>

namespace ami
{
	PatternMesh::PatternMesh(const PatternGraph & graph)
		:
		m_pointDistance (1.0f),
		m_roundNum (0),
		m_minTension (0.1f),
		m_damping (0.1f),
		m_solveIterations (5)
	{

		unsigned int nodeIndex = 0; 
		for (auto & node : graph.getNodes())
		{
			float heightInc = m_pointDistance;
			float radius = m_pointDistance * 2.0f;
			float height = heightInc*nodeIndex/ 10.f;
			
			glm::vec3 vertex(0, height, -radius);
			vertex = glm::rotate(vertex, nodeIndex/10.f * (float)TWO_PI, glm::vec3(0.0f, 1.0f, 0.0f));

			m_mesh.addVertex(vertex);
			m_oldVec.push_back(vertex);

			nodeIndex++;
		}

		for (auto & edge : graph.getEdges())
		{
			setDistanceConstrain(edge.from, edge.to, edge.distance * m_pointDistance);
		}
		
		for (auto & face : graph.getFaces())
		{
			m_mesh.addIndices(&face.ids[0], 3);
		}
	}

	void PatternMesh::setDistanceConstrain(ofIndexType a, ofIndexType b, float distance)
	{
		// merging vertices does not add triangles, just adds a hard constraint of 0 distance between the vertices
		m_con[a].insert({ b, distance });
		m_con[b].insert({ a, distance });
	}
	void PatternMesh::setAngleConstrain(ofIndexType a, ofIndexType b, float degrees)
	{
		float A2 = m_pointDistance*m_pointDistance;

		float distance = std::sqrt(2*A2*( 1 - std::cos(degrees * DEG_TO_RAD )));

		m_soft_con[a].insert({ b, distance });
		m_soft_con[b].insert({ a, distance });
	}

	void PatternMesh::update(float deltaTime)
	{
		this->updateNormals();
		this->computeForces();
		this->verletUpdate(deltaTime);

		// solve constraints
		for (unsigned int i = 0; i < m_solveIterations; i++)
		{
			this->solveConstraints();
		}

		this->updateCenter();
		// center mesh
		for (auto & vert : m_mesh.getVertices())
		{
			vert.x -= m_center.x;
			vert.z -= m_center.z;
		}
	}

	void PatternMesh::verletUpdate(float deltaTime)
	{
		float dt2 = deltaTime * deltaTime;
		// verlet update
		for (auto & con = m_con.begin(); con != m_con.end(); con++)
		{
			glm::vec3 & expansion = m_expansionForce[con->first];
			glm::vec3 & vertex = m_mesh.getVertices()[con->first];
			glm::vec3 & oldVertex = m_oldVec[con->first];

			glm::vec3 acc = expansion; // inner expansion
			glm::vec3 vel = vertex - oldVertex; // velocity is last distance (inertia, no need for dt)
			oldVertex = vertex;

			glm::vec3 acceleration = acc * dt2;
			glm::vec3 velocity = vel * m_damping;
			vertex = vertex + velocity + acceleration;
		}
	}

	void PatternMesh::solveConstraints()
	{
		// solve constrains
		for (auto & con = m_con.begin(); con != m_con.end(); con++)
		{
			glm::vec3 & point0 = m_mesh.getVertices()[con->first];

			for (auto & index : con->second)
			{
				glm::vec3 & point1 = m_mesh.getVertices()[index.first];
				glm::vec3 distVec = point0 - point1;
				float dist = glm::length(distVec);
				if (dist == 0.0f) dist = std::numeric_limits<float>::epsilon(); // check for zero division
				glm::vec3 tension = distVec * (index.second - dist) / dist;

				point0 += tension * 0.5f; // update vertex following constraint
				point1 -= tension * 0.5f; // update vertex following constraint
			}

			m_mesh.getVertices()[0] = ofVec3f(0); // insist on this constraint
		}
		
		// solve soft constrains
		for (auto & con = m_soft_con.begin(); con != m_soft_con.end(); con++)
		{
			glm::vec3 & point0 = m_mesh.getVertices()[con->first];

			for (auto & index : con->second)
			{
				glm::vec3 & point1 = m_mesh.getVertices()[index.first];
				glm::vec3 distVec = point0 - point1;
				float dist = glm::length(distVec);
				// apply tension only if the distance is smaller than the desired distance
				if (index.second < dist)
				{
					if (dist == 0.0f) dist = std::numeric_limits<float>::epsilon(); // check for zero division
					glm::vec3 tension = distVec * (index.second - dist) / dist;

					point0 += tension * 0.5f; // update vertex following constraint
					point1 -= tension * 0.5f; // update vertex following constraint
				}
			}
		}
	}

	void PatternMesh::computeForces()
	{
		for (auto & con = m_con.begin(); con != m_con.end(); con++)
		{
			// expansion
			glm::vec3 avgNormal;
			for (auto & index : con->second)
			{
				// compute average normal of neighbours
				avgNormal += m_mesh.getNormal(index.first);
			}
			if (!con->second.empty())
			{
				avgNormal /= con->second.size();
			}

			m_expansionForce[con->first] = avgNormal * 2000.0f;
		}
	}

	void PatternMesh::updateCenter()
	{
		// find mesh center
		m_center = std::accumulate(m_mesh.getVertices().begin(), m_mesh.getVertices().end(), glm::vec3(0));
		m_center /= m_mesh.getNumVertices();
	}

	void PatternMesh::updateNormals()
	{
		m_mesh.clearNormals();

		unsigned int nVertices = m_mesh.getVertices().size();
		unsigned int nIndices = m_mesh.getIndices().size();

		std::vector<int> vertexFaceCount(nVertices);
		std::vector< glm::vec3 > vertexNormals(nVertices);
		std::vector< glm::vec3 > faceNormals = m_mesh.getFaceNormals();

		for (unsigned int i = 0; i < nIndices; i++)
		{
			int vertex = m_mesh.getIndices()[i];
			int face = ((int)(i / 3)) * 3; //OF issue, getFaceNormals() iterates +=3, so faceNormals should move like: 0, 3, 6, 9...
			vertexNormals[vertex] += faceNormals[face];
			vertexFaceCount[vertex]++;
		}
		for (unsigned int i = 0; i < vertexNormals.size(); i++)
		{
			vertexNormals[i] /= vertexFaceCount[i];
			vertexNormals[i] = glm::normalize(vertexNormals[i]);
		}

		m_mesh.addNormals(vertexNormals);
	}

	void PatternMesh::draw()
	{
		ofPushStyle();
		ofDisableDepthTest();
		ofEnableBlendMode(ofBlendMode::OF_BLENDMODE_ALPHA);
		ofSetColor(255);
		glPointSize(5.0f);
		m_mesh.drawVertices();
		//ofSetColor(200, 50);
		//m_mesh.draw();
		ofSetColor(200);
		m_mesh.drawWireframe();
		//ofSetColor(ofColor::red);
		//for (auto & tension : m_tension)
		//{
		//	glm::vec3 start = m_mesh.getVertex(tension.first);
		//	glm::vec3 end = start + tension.second;
		//	glBegin(GL_LINES);
		//	glVertex3f(start.x, start.y, start.z);
		//	glVertex3f(end.x, end.y, end.z);
		//	glEnd();
		//}
		//ofSetColor(ofColor::blue);
		//for (auto & tension : m_constraintTension)
		//{
		//	glm::vec3 start = m_mesh.getVertex(tension.first);
		//	glm::vec3 end = start + tension.second;
		//	glBegin(GL_LINES);
		//	glVertex3f(start.x, start.y, start.z);
		//	glVertex3f(end.x, end.y, end.z);
		//	glEnd();
		//}


		ofSetColor(ofColor::green);
		for (auto & tension : m_expansionForce)
		{
			glm::vec3 start = m_mesh.getVertex(tension.first);
			glm::vec3 end = start + glm::normalize(tension.second);
			glBegin(GL_LINES);
			glVertex3f(start.x, start.y, start.z);
			glVertex3f(end.x, end.y, end.z);
			glEnd();
		}
		
		ofSetLineWidth(2.0f);
		ofSetColor(ofColor::red);
		for (auto & con = m_con.begin(); con != m_con.end(); con++)
		{
			glm::vec3 & point0 = m_mesh.getVertices()[con->first];

			for (auto & index : con->second)
			{
				glm::vec3 & point1 = m_mesh.getVertices()[index.first];

				glm::vec3 start = point0;
				glm::vec3 end = point0 + glm::normalize(point1 - point0) * index.second; // show the correct distance

				glBegin(GL_LINES);
				glVertex3f(start.x, start.y, start.z);
				glVertex3f(end.x, end.y, end.z);
				glEnd();
			}
		}

		ofPopStyle();
	}

}


