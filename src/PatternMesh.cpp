#include "PatternMesh.h"

#include "ofGraphics.h"

namespace ami
{
	PatternMesh::PatternMesh() 
	{}

	void PatternMesh::setup(float pointDistance)
	{
		m_pointDistance = pointDistance;
		m_mesh.clear();
		m_con.clear();
		m_tension.clear();
		m_idealTension.clear();
		m_expansionTension.clear();

		m_roundNum = 0;
	}

	void PatternMesh::update(float deltaTime)
	{
		this->computeTension();

		for (auto con = m_con.begin(); con != m_con.end(); con++)
		{
			// inter nodal tension
			m_mesh.getVertices()[con->first] += m_idealTension[con->first] *deltaTime;

			// inner expansion only for connected stitches, not for the last row
			if (con->second.size() > 4)
				m_mesh.getVertices()[con->first] += m_expansionTension[con->first] * deltaTime;
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
			vert -= center*deltaTime;
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
		float angle = 0, radius = 0;
		if (roundSize > 2)
		{
			angle = (180.0f / (float)(roundSize-1))*DEG_TO_RAD;
			radius = (float)m_pointDistance / (2.0f * sin(angle));
		}

		ofVec3f orientation(0, 0, -1);
		orientation.rotate(((float)roundIndex / (float)(roundSize)) * 360, ofVec3f(0, 1, 0));
		
		float roundHeight = round*0.2f;

		ofIndexType m_current;

		switch (type)
		{
			case Operation::Type::LP:
			{
				m_mesh.addVertex(ofVec3f(0));
			
				m_behind = m_mesh.getNumVertices() - 1;
				m_lastUnder = m_mesh.getNumVertices() - 1;
				break;
			}

			case Operation::Type::SC:
			{
				// single stitch connected under
				m_mesh.addVertex(ofVec3f(orientation.x, roundHeight, orientation.z));

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
				m_mesh.addVertex(ofVec3f(orientation.x, roundHeight, orientation.z));

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
				m_mesh.addVertex(ofVec3f(orientation.x, roundHeight, orientation.z));

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

			ofVec3f currentTension;
			ofVec3f idealTension;
			for (auto index : con->second)
			{
				ofVec3f dist = m_mesh.getVertex(index) - point0;
				currentTension += dist;
				idealTension += (dist - dist.getNormalized()*m_pointDistance);
			}

			m_tension[con->first] = currentTension;
			m_idealTension[con->first] = idealTension;
			m_expansionTension[con->first] = m_mesh.getNormal(con->first)*0.5f;
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
		for (auto & tension : m_idealTension)
		{
			ofVec3f start = m_mesh.getVertex(tension.first);
			ofVec3f end = start + tension.second;
			glBegin(GL_LINES);
			glVertex3f(start.x, start.y, start.z);
			glVertex3f(end.x, end.y, end.z);
			glEnd();
		}
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


