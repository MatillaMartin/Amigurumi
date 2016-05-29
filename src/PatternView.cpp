#include "PatternView.h"

namespace ami
{
	PatternView::PatternView()
		:
		m_pattern(nullptr)
	{

	}

	void PatternView::render()
	{
		m_mesh.draw();
	}

	void PatternView::update(float deltaTime)
	{
		m_mesh.update(deltaTime);
	}

	void PatternView::setPattern(PatternDef * pattern)
	{
		m_pattern = pattern;

		m_mesh.setup(1.0f);

		for (auto round = m_pattern->begin(); round != m_pattern->end(); round++)
		{
			m_mesh.addRound(*round);
		}
	}
}