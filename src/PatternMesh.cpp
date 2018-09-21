#include "PatternMesh.h"

#include "ofGraphics.h"

#include <numeric>

namespace ami
{
	PatternMesh::PatternMesh(){}

	void PatternMesh::setup()
	{
		m_pointDistance = 1.0f;
		m_mesh.clear();
		m_oldVec.clear();
		m_con.clear();
		m_expansionForce.clear();
		m_fix.clear();
		m_roundNum = 0;
		m_minTension = 0.1f;
		m_damping = 0.1f;
		m_solveIterations = 5;
	}

	void PatternMesh::addRound(const Operation::Operations & op)
	{
		for (unsigned int nOp = 0; nOp < op.size(); nOp++)
		{
			addOperation(op[nOp], m_roundNum, nOp, op.size());
		}

		m_roundNum++;
	}

	void PatternMesh::insertNextVertex(unsigned int round, unsigned int roundIndex, unsigned int roundSize)
	{
		float radiusInc = m_pointDistance;
		float heightInc = m_pointDistance * 5.0f;
		float roundProgress = (float)roundIndex / (float)roundSize;
		float radius = radiusInc + radiusInc*round + radiusInc*roundProgress;
		float height = heightInc*round + heightInc*roundProgress;

		ofVec3f nextVertex(0, height, -radius);
		nextVertex.rotate(roundProgress * 360, ofVec3f(0, 1, 0));

		m_mesh.addVertex(nextVertex);
		m_oldVec.push_back(nextVertex);
	}

	void PatternMesh::addOperation(Operation::Type type, unsigned int round, unsigned int roundIndex, unsigned int roundSize)
	{
		ofIndexType m_current;

		switch (type)
		{
			case Operation::Type::LP:
			{
				this->insertNextVertex(round, roundIndex, roundSize);
				
				m_behind = m_mesh.getNumVertices() - 1;
				m_lastUnder = m_mesh.getNumVertices() - 1;
				break;
			}

			case Operation::Type::SC:
			{
				this->insertNextVertex(round, roundIndex, roundSize);

				// single stitch connected under
				m_current = m_mesh.getNumVertices() - 1;
				if (m_behind != m_lastUnder)
				{
					this->addTriangle(m_behind, m_lastUnder, m_lastUnder + 1);
					this->addTriangle(m_current, m_behind, m_lastUnder + 1);
				}
				m_behind++;
				m_lastUnder++;

				break;
			}
			case Operation::Type::INC:
			{
				this->insertNextVertex(round, roundIndex, roundSize);

				// connect to same as last stitch
				m_current = m_mesh.getNumVertices() - 1;
				if (m_behind != m_lastUnder)
				{
					this->addTriangle(m_current, m_behind, m_lastUnder);
				}
				m_behind++;

				break;
			}
			case Operation::Type::DEC:
			{
				this->insertNextVertex(round, roundIndex, roundSize);

				// connect to next two under
				m_current = m_mesh.getNumVertices() - 1;
				if (m_behind != m_lastUnder)
				{
					this->addTriangle(m_behind, m_lastUnder, m_lastUnder + 1);
					this->addTriangle(m_current, m_behind, m_lastUnder + 1);
					this->addTriangle(m_current, m_lastUnder + 1, m_lastUnder + 2);
				}
				m_behind++;
				m_lastUnder += 2;

				break;
			}
			case Operation::Type::FO:
			{
				// this operation does not add a new vertex
				unsigned int distance = m_behind - (m_lastUnder + 1);
				unsigned int halfDist = std::floor((float)distance / 2.0f);

				for (unsigned int i = 0; i < halfDist; i++)
				{
					// "a" points to first vertices, "b" points to complementary to stitch
					unsigned int a = m_lastUnder + 1 + i;
					unsigned int b = m_behind - i;

					if (a != b)
					{
						this->setDistanceConstrain(a, b, 0.0f);
					}
				}
				break;
			}
		}
	}

	void PatternMesh::addTriangle(ofIndexType tri0, ofIndexType tri1, ofIndexType tri2)
	{
		m_mesh.addIndex(tri0);
		m_mesh.addIndex(tri1);
		m_mesh.addIndex(tri2);
		this->setDistanceConstrain(tri0, tri1, m_pointDistance);
		this->setDistanceConstrain(tri1, tri2, m_pointDistance);
		this->setDistanceConstrain(tri2, tri0, m_pointDistance);
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
			ofVec3f & expansion = m_expansionForce[con->first];
			ofVec3f & vertex = m_mesh.getVertices()[con->first];
			ofVec3f & oldVertex = m_oldVec[con->first];

			ofVec3f acc = expansion; // inner expansion
			ofVec3f vel = vertex - oldVertex; // velocity is last distance (inertia, no need for dt)
			oldVertex = vertex;

			ofVec3f acceleration = acc * dt2;
			ofVec3f velocity = vel * m_damping;
			vertex = vertex + velocity + acceleration;
		}
	}

	void PatternMesh::solveConstraints()
	{
		// solve constrains
		for (auto & con = m_con.begin(); con != m_con.end(); con++)
		{
			ofPoint & point0 = m_mesh.getVertices()[con->first];

			for (auto & index : con->second)
			{
				ofPoint & point1 = m_mesh.getVertices()[index.first];
				ofVec3f distVec = point0 - point1;
				float dist = distVec.length();
				if (dist == 0.0f) dist = std::numeric_limits<float>::epsilon(); // check for zero division
				ofVec3f tension = distVec * (index.second - dist) / dist;

				point0 += tension * 0.5f; // update vertex following constraint
				point1 -= tension * 0.5f; // update vertex following constraint
			}

			m_mesh.getVertices()[0] = ofVec3f(0); // insist on this constraint
		}
		
		// solve soft constrains
		for (auto & con = m_soft_con.begin(); con != m_soft_con.end(); con++)
		{
			ofPoint & point0 = m_mesh.getVertices()[con->first];

			for (auto & index : con->second)
			{
				ofPoint & point1 = m_mesh.getVertices()[index.first];
				ofVec3f distVec = point0 - point1;
				float dist = distVec.length();
				// apply tension only if the distance is smaller than the desired distance
				if (index.second < dist)
				{
					if (dist == 0.0f) dist = std::numeric_limits<float>::epsilon(); // check for zero division
					ofVec3f tension = distVec * (index.second - dist) / dist;

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
			ofVec3f avgNormal;
			for (auto & index : con->second)
			{
				// compute average normal of neighbours
				avgNormal += m_mesh.getNormal(index.first);
			}
			if (!con->second.empty())
			{
				avgNormal /= con->second.size();
			}

			m_expansionForce[con->first] = avgNormal * 20000.0f;
		}
	}

	void PatternMesh::updateCenter()
	{
		// find mesh center
		m_center = std::accumulate(m_mesh.getVertices().begin(), m_mesh.getVertices().end(), ofVec3f(0));
		m_center /= m_mesh.getNumVertices();
	}

	void PatternMesh::updateNormals()
	{
		m_mesh.clearNormals();

		unsigned int nVertices = m_mesh.getVertices().size();
		unsigned int nIndices = m_mesh.getIndices().size();

		vector<int> vertexFaceCount(nVertices);
		vector< ofVec3f > vertexNormals(nVertices);
		vector< ofVec3f > faceNormals = m_mesh.getFaceNormals();

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
			vertexNormals[i].normalize();
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
		ofSetColor(200, 50);
		m_mesh.draw();
		ofSetColor(200);
		m_mesh.drawWireframe();
		//ofSetColor(ofColor::red);
		//for (auto & tension : m_tension)
		//{
		//	ofVec3f start = m_mesh.getVertex(tension.first);
		//	ofVec3f end = start + tension.second;
		//	glBegin(GL_LINES);
		//	glVertex3f(start.x, start.y, start.z);
		//	glVertex3f(end.x, end.y, end.z);
		//	glEnd();
		//}
		//ofSetColor(ofColor::blue);
		//for (auto & tension : m_constraintTension)
		//{
		//	ofVec3f start = m_mesh.getVertex(tension.first);
		//	ofVec3f end = start + tension.second;
		//	glBegin(GL_LINES);
		//	glVertex3f(start.x, start.y, start.z);
		//	glVertex3f(end.x, end.y, end.z);
		//	glEnd();
		//}


		ofSetColor(ofColor::green);
		for (auto & tension : m_expansionForce)
		{
			ofVec3f start = m_mesh.getVertex(tension.first);
			ofVec3f end = start + tension.second.getNormalized();
			glBegin(GL_LINES);
			glVertex3f(start.x, start.y, start.z);
			glVertex3f(end.x, end.y, end.z);
			glEnd();
		}
		
		//ofSetLineWidth(5.0f);
		//ofSetColor(ofColor::red);
		//for (auto & con = m_con.begin(); con != m_con.end(); con++)
		//{
		//	ofPoint & point0 = m_mesh.getVertices()[con->first];

		//	for (auto & index : con->second)
		//	{
		//		ofPoint & point1 = m_mesh.getVertices()[index.first];

		//		ofVec3f start = point0;
		//		ofVec3f end = start + (point1 - point0).getNormalized()*m_pointDistance;

		//		glBegin(GL_LINES);
		//		glVertex3f(start.x, start.y, start.z);
		//		glVertex3f(end.x, end.y, end.z);
		//		glEnd();
		//	}
		//}

		ofPopStyle();
	}

}


