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
		}

		const std::vector<Round> & getRounds() const {
			return m_rounds;
		}

		std::vector<Round> & getRounds() {
			return m_rounds;
		}

		void addRound(Operation::Operations && round)
		{
			m_rounds.push_back(std::move(round));
		}

	private:
		std::vector<Round> m_rounds;

	};
}