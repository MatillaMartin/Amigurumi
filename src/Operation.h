#pragma once

#include "ofLog.h"
#include <map>

namespace ami
{
	class Operation
	{
	public:
		enum class Type {
			// loop
			LP,
			// crochet
			CH,
			// single crochet
			SC,
			// increase
			INC,
			// decrease
			DEC,
			// magic ring
			MR,
			// finish off
			FO,
			// nothing
			NONE
		};

		typedef std::vector<Operation::Type> Operations;

		static Operation::Type getOperation(const std::string & op)
		{
			const static std::map<std::string, Type> m_operations =
			{
				{ "LP", Type::LP },
				{ "SC", Type::SC },
				{ "INC", Type::INC },
				{ "DEC", Type::DEC },
				{ "MR", Type::MR },
				{ "FO", Type::FO }
			};

			auto & op_it = m_operations.find(op);
			if (op_it != m_operations.end())
			{
				return op_it->second;
			}

			ofLogWarning("Operation") << "getType: String " << op << " not found";
			return Type::NONE;
		}
		
		static std::string getString(Operation::Type op)
		{
			const static std::map<Type, std::string> m_operations =
			{
				{ Type::LP, "LP" },
				{ Type::SC, "SC" },
				{ Type::INC, "INC" },
				{ Type::DEC, "DEC" },
				{ Type::MR, "MR" },
				{ Type::FO, "FO" }
			};

			auto & op_it = m_operations.find(op);
			if (op_it != m_operations.end())
			{
				return op_it->second;
			}

			ofLogWarning("Operation") << "getString: Type not found";
			return "NONE";
		}

		static unsigned int getRequiredStitches(Operation::Type op)
		{
			switch (op)
			{
				case Type::LP: return 0; break;
				//case Type::CH: return "1"; break;
				case Type::SC: return 1; break;
				case Type::INC: return 0; break;
				case Type::DEC: return 2; break;
				//case MR: return 1; break;
				default: ofLogWarning("Operation") << "RequiredStitches: Type not found";
			}
		}

		static Operations parseOperation(Operation::Type op, unsigned int count)
		{
			if (op == Type::MR)
			{
				return Operations(count, Type::INC); // magic ring consists on "count inc"
			}
			// for the rest its simply the operation n times
			else
			{
				return Operations(count, op);
			}
		}
	};
}