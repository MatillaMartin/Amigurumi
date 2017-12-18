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
		m_mesh.update(deltaTime);
	}

	void PatternView::setPattern(const PatternDef & pattern)
	{
		m_mesh.setup();

		for (auto & round = pattern.getRounds().begin(); round != pattern.getRounds().end(); round++)
		{
			m_mesh.addRound(*round);
		}
	}
}