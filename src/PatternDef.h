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

		const std::vector<Round> & getRounds() const {
			return m_rounds;
		}

		std::vector<Round> & getRounds() {
			return m_rounds;
		}

		void addRound(Operation::Operations & round)
		{
			m_rounds.push_back(round);
		}

	private:
		std::vector<Round> m_rounds;

	};
}