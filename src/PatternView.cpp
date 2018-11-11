#include "PatternView.h"

namespace ami
{
	PatternView::PatternView()
	{}

	PatternView::PatternView(const PatternGraph & pattern, const Params & params)
	{
		m_mesh = PatternMesh(pattern, params.anchors);
	}

	void PatternView::render()
	{
		m_mesh.draw();
	}

	void PatternView::update(float deltaTime)
	{
		m_mesh.update(deltaTime);
	}
}