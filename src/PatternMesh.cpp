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
		m_minTension = 0.01f;
	}

	void PatternMesh::update(float deltaTime)
	{
		this->computeTension();

		for (auto & con = m_con.begin(); con != m_con.end(); con++)
		{
			ofVec3f & constrain = m_constraintTension[con->first];
			ofVec3f & expansion = m_expansionTension[con->first];
			if (std::abs(constrain.length() - expansion.length()) < m_minTension)
			{
				continue;
			}

			// inter nodal tension
			m_mesh.getVertices()[con->first] += constrain * deltaTime;

			// inner expansion only for connected stitches, not for the last row
			if (con->second.size() > 4)
			{
				m_mesh.getVertices()[con->first] += expansion * deltaTime;
			}
		}

		// recenter mesh
		ofVec3f center;
		for (auto & vert : m_mesh.getVertices())
		{
			center += vert;
		}
		center /= m_mesh.getNumVertices();

		for (auto & vert : m_mesh.getVertices())
		{
			vert.x -= center.x*deltaTime;
			vert.z -= center.z*deltaTime;
		}

		for (unsigned int i = 1; i < m_mesh.getNumVertices(); i++)
		{
			ofVec3f & vert = m_mesh.getVertices().at(i);
			vert -= m_mesh.getVertex(0) * deltaTime;
		}
	}

	void PatternMesh::addRound(Operation::Operations op)
	{
		for (unsigned int nOp = 0; nOp < op.size(); nOp++)
		{
			addOperation(op[nOp], m_roundNum, nOp, op.size());
		}

		m_roundNum++;
	}

	void PatternMesh::addOperation(Operation::Type type, unsigned int round, unsigned int roundIndex, unsigned int roundSize)
	{
		float radiusInc = m_pointDistance;
		float heightInc = m_pointDistance * 5.0f;
		float roundProgress = (float)roundIndex / (float)roundSize;
		float radius = radiusInc + radiusInc*round + radiusInc*roundProgress;
		float height = heightInc + heightInc*round + heightInc*roundProgress;

		ofVec3f nextVertex(0, height, -radius);
		nextVertex.rotate(roundProgress * 360, ofVec3f(0, 1, 0));

		m_mesh.addVertex(nextVertex);

		ofIndexType m_current;

		switch (type)
		{
			case Operation::Type::LP:
			{			
				m_behind = m_mesh.getNumVertices() - 1;
				m_lastUnder = m_mesh.getNumVertices() - 1;
				break;
			}

			case Operation::Type::SC:
			{
				// single stitch connected under
				m_current = m_mesh.getNumVertices() - 1;
				if (m_behind != m_lastUnder)
				{
					this->addTriangle(m_current, m_behind, m_lastUnder + 1);
				}
				m_behind++;
				m_lastUnder++;

				break;
			}
			case Operation::Type::INC:
			{
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
				// connect to next two under
				m_current = m_mesh.getNumVertices() - 1;
				if (m_behind != m_lastUnder)
				{
					this->addTriangle(m_current, m_behind, m_lastUnder);
					this->addTriangle(m_current, m_lastUnder, m_lastUnder + 1);
					this->addTriangle(m_current, m_lastUnder + 1, m_lastUnder + 2);
				}
				m_behind++;
				m_lastUnder += 2;

				break;
			}

		}
	}

	void PatternMesh::addTriangle(ofIndexType tri0, ofIndexType tri1, ofIndexType tri2)
	{
		m_mesh.addIndex(tri0);
		m_mesh.addIndex(tri1);
		m_mesh.addIndex(tri2);
		m_con[tri0].insert(tri1);
		m_con[tri0].insert(tri2);
		m_con[tri1].insert(tri0);
		m_con[tri1].insert(tri2);
		m_con[tri2].insert(tri0);
		m_con[tri2].insert(tri1);
	}

	void PatternMesh::computeTension()
	{
		this->updateNormals();
		
		for (auto con = m_con.begin(); con != m_con.end(); con++)
		{
			ofPoint point0 = m_mesh.getVertex(con->first);

			std::vector<ofVec3f> & constraintTensions = m_constraintTensions[con->first];
			constraintTensions.clear();
			ofVec3f constraintTension;
			for (auto index : con->second)
			{
				ofVec3f distVec = m_mesh.getVertex(index) - point0;
				float dist = distVec.length();
				if (dist == 0.0f) dist = std::numeric_limits<float>::epsilon(); // check for zero division
				ofVec3f tension = distVec * (1.0f - m_pointDistance / dist) * 0.5f;
				constraintTensions.push_back(tension);
			}
			constraintTension = std::accumulate(constraintTensions.begin(), constraintTensions.end(), ofVec3f(0));

			m_constraintTension[con->first] = constraintTension;
			m_expansionTension[con->first] = m_mesh.getNormal(con->first) * m_pointDistance * 0.5f;
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


