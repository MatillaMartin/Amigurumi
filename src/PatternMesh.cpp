#include "PatternMesh.h"

#include "ofGraphics.h"

#include <numeric>

namespace ami
{
	PatternMesh::PatternMesh() 
	{}

	void PatternMesh::setup(float pointDistance)
	{
		if (pointDistance == 0.0f) pointDistance = 1.0f;
		m_pointDistance = pointDistance;
		m_mesh.clear();
		m_con.clear();
		m_constraintTension.clear();
		m_constraintTensions.clear();
		m_expansionTension.clear();
		m_roundNum = 0;
		m_minTension = 0.1f;
	}

	void PatternMesh::addRound(Operation::Operations op)
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
						this->mergeVertices(a, b);
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
		m_con[tri0].insert({ tri1, m_pointDistance });
		m_con[tri0].insert({ tri2, m_pointDistance });
		m_con[tri1].insert({ tri0, m_pointDistance });
		m_con[tri1].insert({ tri2, m_pointDistance });
		m_con[tri2].insert({ tri0, m_pointDistance });
		m_con[tri2].insert({ tri1, m_pointDistance });
	}

	void PatternMesh::mergeVertices(ofIndexType a, ofIndexType b)
	{
		// merging vertices does not add triangles, just adds a hard constraint of 0 distance between the vertices
		m_con[a].insert({ b, 0.0f });
		m_con[b].insert({ a, 0.0f });
	}

	void PatternMesh::update(float deltaTime)
	{
		// find mesh center
		ofVec3f center = std::accumulate(m_mesh.getVertices().begin(), m_mesh.getVertices().end(), ofVec3f(0));
		center /= m_mesh.getNumVertices();

		this->computeTension();

		unsigned int stopped = 0;
		for (auto & con = m_con.begin(); con != m_con.end(); con++)
		{
			ofVec3f & constrain = m_constraintTension[con->first];
			ofVec3f & expansion = m_expansionTension[con->first];
			ofVec3f & vertex = m_mesh.getVertices()[con->first];

			if ((constrain + expansion).length() < m_minTension)
			{
				stopped++;
				continue;
			}

			// inter nodal tension
			vertex += constrain * deltaTime;

			// inner expansion 
			vertex += expansion * deltaTime;
		}
		ofLogVerbose("PatterMesh") << "Stopped: " << stopped << "/" << m_mesh.getNumVertices();

		// center mesh
		for (auto & vert : m_mesh.getVertices())
		{
			vert.x -= center.x*deltaTime;
			vert.z -= center.z*deltaTime;
		}

		// center first vertex
		for (auto & it = m_mesh.getVertices().begin() + 1; it != m_mesh.getVertices().end(); ++it)
		{
			*it -= m_mesh.getVertex(0) * deltaTime;
		}
	}

	void PatternMesh::computeTension()
	{
		this->updateNormals();
		
		for (auto con = m_con.begin(); con != m_con.end(); con++)
		{
			const ofPoint & point0 = m_mesh.getVertex(con->first);

			// constrain
			std::vector<ofVec3f> & constraintTensions = m_constraintTensions[con->first];
			constraintTensions.clear();
			ofVec3f constraintTension;
			for (auto index : con->second)
			{
				ofVec3f distVec = m_mesh.getVertex(index.first) - point0;
				float dist = distVec.length();
				if (dist == 0.0f) dist = std::numeric_limits<float>::epsilon(); // check for zero division
				ofVec3f tension = distVec * (1.0f - index.second / dist) * 0.5f;
				constraintTensions.push_back(tension);
			}
			constraintTension = std::accumulate(constraintTensions.begin(), constraintTensions.end(), ofVec3f(0));
			m_constraintTension[con->first] = constraintTension;

			// expansion
			ofVec3f expansion;
			expansion = m_mesh.getNormal(con->first) * m_pointDistance * 0.1f;

			m_expansionTension[con->first] = expansion;
		}
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

		ofSetColor(ofColor(255));
		glPointSize(3.0f);
		m_mesh.drawVertices();
		m_mesh.drawWireframe();
		//m_mesh.draw();
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
		ofSetColor(ofColor::blue);
		for (auto & tension : m_constraintTension)
		{
			ofVec3f start = m_mesh.getVertex(tension.first);
			ofVec3f end = start + tension.second;
			glBegin(GL_LINES);
			glVertex3f(start.x, start.y, start.z);
			glVertex3f(end.x, end.y, end.z);
			glEnd();
		}

		ofSetColor(ofColor::red);
		ofSetLineWidth(5.0f);
		for (auto & constraint : m_constraintTensions)
		{
			for (auto & tension : constraint.second)
			{
				ofVec3f start = m_mesh.getVertex(constraint.first);
				ofVec3f end = start + tension;
				glBegin(GL_LINES);
				glVertex3f(start.x, start.y, start.z);
				glVertex3f(end.x, end.y, end.z);
				glEnd();
			}
		}
		ofSetLineWidth(1.0f);

		ofSetColor(ofColor::green);
		for (auto & tension : m_expansionTension)
		{
			ofVec3f start = m_mesh.getVertex(tension.first);
			ofVec3f end = start + tension.second;
			glBegin(GL_LINES);
			glVertex3f(start.x, start.y, start.z);
			glVertex3f(end.x, end.y, end.z);
			glEnd();
		}
		ofPopStyle();
	}

}


