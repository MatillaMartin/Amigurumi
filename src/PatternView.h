#pragma once

#include "PatternDef.h"
#include "PatternMesh.h"

namespace ami
{
	class PatternView
	{
	public:
		PatternView();

		void setPattern(const PatternGraph & pattern, bool bStep = false);

		void render();

		void update(float deltaTime);

		PatternMesh m_mesh;

	private:
		struct StepByStep
		{
			bool bStep;
			std::vector<PatternDef::Round>::const_iterator nextRound;
			std::vector<PatternDef::Round>::const_iterator lastRound;
			float lastMillis;
			float stepPeriod;
		} m_sbs;
	};
}