#pragma once

#include "PatternDef.h"
#include "PatternMesh.h"

namespace ami
{
	class PatternView
	{
	public:
		PatternView();

		void setPattern(PatternDef * pattern);

		void render();

		void update(float deltaTime);

		PatternDef * m_pattern;

		PatternMesh m_mesh;
	};
}