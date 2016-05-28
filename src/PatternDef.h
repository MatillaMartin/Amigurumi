#pragma once

#include "Operation.h"
#include <vector>

namespace ami
{
	class PatternDef
	{
	public:
		// A pattern definition consists on a series of rounds
		// Each round consists a series of operation types
		typedef Operation::Operations Round;

		PatternDef()
		{
			// all PatternDefinitions contain a Loop to start with... then MR or other stitches
			std::vector<Operation::Type> loop = { Operation::Type::LP };
			this->addRound(loop);
		}

		vector<PatternDef::Round>::iterator begin()
		{
			return rounds.begin();
		}

		vector<PatternDef::Round>::iterator end()
		{
			return rounds.end();
		}

		void addRound(Operation::Operations & round)
		{
			rounds.push_back(round);
		}

		bool checkValid()
		{
			if (rounds.size() == 1) return true; // there is only a loop

			for (auto round = rounds.begin() + 1; round != rounds.end() - 1; round++)
			{
				auto previousRound = round - 1;

				unsigned int previousRoundStitches = previousRound->size();

				// compute the needed previous stitches for the round
				bool bFirstStitch = true;
				unsigned int neededStitches = 0;
				for (Operation::Type type : *round)
				{
					unsigned int opStitches = Operation::getRequiredStitches(type);
					if (bFirstStitch && opStitches == 0)
					{
						opStitches = 1;
					}

					neededStitches += opStitches;
					bFirstStitch = false;
				}

				if (previousRoundStitches != neededStitches)
				{
					return false;
				}
			}
			
			return true;
		}

	private:
		vector<Round> rounds;

	};
}