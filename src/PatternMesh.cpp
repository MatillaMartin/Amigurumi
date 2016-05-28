#include "PatternMesh.h"

namespace ami
{
	PatternMesh::PatternMesh() 
	{}

	void PatternMesh::setup(float pointDistance)
	{
		m_pointDistance = pointDistance;
		m_mesh.clear();
		m_roundNum = 0;
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
					m_mesh.addIndex(m_current);
					m_mesh.addIndex(m_behind);
					m_mesh.addIndex(m_lastUnder + 1);
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
					m_mesh.addIndex(m_current);
					m_mesh.addIndex(m_behind);
					m_mesh.addIndex(m_lastUnder);
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
					m_mesh.addIndex(m_current);
					m_mesh.addIndex(m_behind);
					m_mesh.addIndex(m_lastUnder);

					m_mesh.addIndex(m_current);
					m_mesh.addIndex(m_lastUnder);
					m_mesh.addIndex(m_lastUnder + 1);

					m_mesh.addIndex(m_current);
					m_mesh.addIndex(m_lastUnder + 1);
					m_mesh.addIndex(m_lastUnder + 2);
				}
				m_behind++;
				m_lastUnder += 2;

				break;
			}

		}
	}

	void PatternMesh::draw()
	{
		glPointSize(2.0f);
		m_mesh.drawVertices();
		m_mesh.drawWireframe();
	}

}