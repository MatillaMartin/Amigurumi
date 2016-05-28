#include "PatternMesh.h"

namespace ami
{
	PatternMesh::PatternMesh() 
	{}

	void PatternMesh::setup(float pointDistance)
	{
		m_pointDistance = pointDistance;
		m_mesh.clear();
	}

	void PatternMesh::addRound(Operation::Operations op)
	{
		for (Operation::Type type : op)
		{
			addOperation(type);
		}
	}

	void PatternMesh::addOperation(Operation::Type type)
	{
		switch (type)
		{
			case Operation::Type::LP:
			{
				m_mesh.addVertex(ofVec3f(0));
			
				m_behind = m_mesh.getNumVertices() - 1;
				m_lastUnder = m_mesh.getNumVertices() - 1;

				nextPoint = ofVec3f(0, 0, -1) * m_pointDistance;
				break;
			}

			case Operation::Type::SC:
			{
				// single stitch connected under
				m_mesh.addVertex(nextPoint);

				m_current = m_mesh.getNumVertices() - 1;
				if (m_behind != m_lastUnder)
				{
					m_mesh.addIndex(m_current);
					m_mesh.addIndex(m_behind);
					m_mesh.addIndex(m_lastUnder + 1);
				}
				m_behind++;
				m_lastUnder++;

				nextPoint += ofVec3f(0,1,0).cross((nextPoint - m_mesh.getVertex(m_lastUnder)).normalized()) * m_pointDistance;
				nextPoint += ofVec3f(0, 0.1f, 0);
				break;
			}
			case Operation::Type::INC:
			{
				// connect to same as last stitch
				m_mesh.addVertex(nextPoint);

				m_current = m_mesh.getNumVertices() - 1;
				if (m_behind != m_lastUnder)
				{
					m_mesh.addIndex(m_current);
					m_mesh.addIndex(m_behind);
					m_mesh.addIndex(m_lastUnder);
				}
				m_behind++;

				
				nextPoint += ofVec3f(0, 1, 0).cross((nextPoint - m_mesh.getVertex(m_lastUnder)).normalized()) * m_pointDistance;
				nextPoint += ofVec3f(0, 0.1f, 0);
				break;
			}
			case Operation::Type::DEC:
			{
				// connect to next two under
				m_mesh.addVertex(nextPoint);

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

				nextPoint += ofVec3f(0, 1, 0).cross((nextPoint - m_mesh.getVertex(m_lastUnder)).normalized()) * m_pointDistance;
				nextPoint += ofVec3f(0, 0.1f, 0);
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