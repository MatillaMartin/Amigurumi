#pragma once

#include "PatternDef.h"
#include "PatternMesh.h"

namespace ami
{
	class PatternView
	{
	public:
		PatternView();

		void setPattern(const PatternDef & pattern);

		void render();

		void update(float deltaTime);

		PatternMesh m_mesh;
	};
}