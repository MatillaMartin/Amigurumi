#include "PatternView.h"

namespace ami
{
	PatternView::PatternView()
	{}

	PatternView::PatternView(const PatternGraph & pattern, const Params & params)
	{
		m_mesh = PatternMesh(pattern, params.anchors);
	}

	void PatternView::render(const PatternView::RenderSettings & settings)
	{
		PatternMesh::DrawSettings drawsettings;
		if (settings.debug)
		{
			drawsettings = { true, false, true };
		}
		else
		{
			drawsettings = { false, false, false };
		}
		m_mesh.draw(drawsettings);
	}

	void PatternView::update(float deltaTime)
	{
		m_mesh.update(deltaTime);
	}
}