#include "PatternView.h"

namespace ami
{
	PatternView::PatternView()
	{}

	void PatternView::render()
	{
		m_mesh.draw();
	}

	void PatternView::update(float deltaTime)
	{
		//if (m_sbs.bStep) // are we doing step by step?
		//{
		//	if (m_sbs.nextRound != m_sbs.lastRound) // any more rounds left?
		//	{
		//		if (ofGetElapsedTimeMillis() - m_sbs.lastMillis > m_sbs.stepPeriod) // has enough time passed by?
		//		{
		//			m_sbs.lastMillis = ofGetElapsedTimeMillis();
		//			m_mesh.addRound(*m_sbs.nextRound);
		//			m_sbs.nextRound++;
		//		}
		//	}
		//}

		m_mesh.update(deltaTime);
	}

	void PatternView::setPattern(const PatternGraph & graph, bool bStep)
	{
		m_sbs.bStep = bStep;

		m_mesh = PatternMesh(graph);

		//if (bStep) // setup the step by step
		//{
		//	m_sbs.lastMillis = 0.0f;
		//	m_sbs.stepPeriod = 1000.0f;
		//	m_sbs.nextRound = pattern.getRounds().begin();
		//	m_sbs.lastRound = pattern.getRounds().end();
		//}
		//else // if we dont want to go step by step then just add all the rounds in one go
		//{
		//	for (auto & round = pattern.getRounds().begin(); round != pattern.getRounds().end(); round++)
		//	{
		//		m_mesh.addRound(*round);
		//	}
		//}
	}
}