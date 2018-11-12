#include "PatternMesh.h"

#include "ofGraphics.h"

#include <numeric>

namespace ami
{
	PatternMesh::PatternMesh(const PatternGraph & graph, const std::vector<Anchor> & anchors)
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

			m_properties[node.id] = { false, node.id, node.data.op };

			nodeIndex++;
		}

		for (auto & edge : graph.getEdges())
		{
			setDistanceConstrain(edge.from, edge.to, edge.distance * m_pointDistance);
		}
		
		for (auto & joint : graph.getJoints())
		{
			setJoint(joint.from, joint.to);
		}

		for (auto & face : graph.getFaces())
		{
			m_mesh.addIndices(&face.ids[0], 3);
		}

		for (auto & it : graph.getOutline())
		{
			m_outline.push_back(it.id);
		}

		m_anchors = anchors;
		for (auto & anchor : anchors)
		{
			m_properties[anchor.node].isFix = true;
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
	void PatternMesh::setJoint(ofIndexType a, ofIndexType b)
	{
		m_joints[a].insert(b);
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
		// solve anchors
		for (auto & anchor : m_anchors)
		{
			m_mesh.getVertices()[anchor.node] = anchor.anchor; // insist on this constraint
		}

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

				//if(!m_properties[con->first].isFix)
				//{ 
					point0 += tension * 0.5f; // update vertex following constraint
				//}
				//if (!m_properties[index.first].isFix)
				//{
					point1 -= tension * 0.5f; // update vertex following constraint
				//}
			}
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

		// solve joints at the end
		for (auto & joint = m_joints.begin(); joint != m_joints.end(); joint++)
		{
			glm::vec3 & point0 = m_mesh.getVertices()[joint->first];

			for (auto & index : joint->second)
			{
				m_mesh.getVertices()[index] = point0;
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

	void PatternMesh::draw(const PatternMesh::DrawSettings & settings)
	{
		ofPushStyle();
		ofDisableDepthTest();
		ofEnableBlendMode(ofBlendMode::OF_BLENDMODE_ALPHA);
		ofSetColor(255);
		glPointSize(5.0f);
		m_mesh.drawVertices();
		ofSetColor(200, 50);
		m_mesh.draw();
		ofSetColor(200);
		ofSetLineWidth(2.0f);
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

		if (settings.forces)
		{
			ofSetColor(ofColor::green);
			glBegin(GL_LINES);
			for (auto & tension : m_expansionForce)
			{
				glm::vec3 start = m_mesh.getVertex(tension.first);
				glm::vec3 end = start + glm::normalize(tension.second);
				glVertex3f(start.x, start.y, start.z);
				glVertex3f(end.x, end.y, end.z);
			}
			glEnd();
		}
		
		//ofSetLineWidth(2.0f);
		//for (auto & con = m_con.begin(); con != m_con.end(); con++)
		//{  


		//	glm::vec3 & point0 = m_mesh.getVertices()[con->first];

		//	for (auto & index : con->second)
		//	{
		//		glm::vec3 & point1 = m_mesh.getVertices()[index.first];

		//		glm::vec3 start = point0;
		//		float distance = glm::distance2(point1, point0);
		//		glm::vec3 end = point0 + glm::normalize(point1 - point0) * index.second; // show the correct distance

		//		ofColor color = ofColor::red;
		//		ofSetColor(color.lerp(ofColor::white, distance / m_pointDistance));

		//		glBegin(GL_LINES);
		//		glVertex3f(start.x, start.y, start.z);
		//		glVertex3f(end.x, end.y, end.z);
		//		glEnd();
		//	}
		//}

		if (settings.outline)
		{
			if (!m_outline.empty())
			{
				ofSetColor(ofColor::blue);
				glBegin(GL_LINES);
				for (auto it = m_outline.begin(); it != m_outline.end() - 1; it++)
				{
					glm::vec3 start = m_mesh.getVertex(*it);
					glm::vec3 end = m_mesh.getVertex(*(it + 1));

					glVertex3f(start.x, start.y, start.z);
					glVertex3f(end.x, end.y, end.z);
				}
				glEnd();

				glPointSize(6.0f);
				ofSetColor(ofColor::purple);
				glBegin(GL_POINTS);
				glm::vec3 point = m_mesh.getVertex(m_outline.back());
				glVertex3f(point.x, point.y, point.z);
				glEnd();
			}
		}

		if (settings.id)
		{
			ofSetColor(ofColor::white);
			for (auto & prop : m_properties)
			{
				ofDrawBitmapString(ofToString(prop.second.id), m_mesh.getVertex(prop.second.id));
			}
		}

		ofPopStyle();
	}

}


