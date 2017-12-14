#pragma once

#include "ofUtils.h"

namespace ami
{
	class Operation
	{
	public:
		enum Type {
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
		};

		typedef vector<Operation::Type> Operations;

		static Operation::Type getOperation(const string & op)
		{
			if (op == "LP") return LP;
			//if (op == "CH") return CH;
			if (op == "SC") return SC;
			if (op == "INC") return INC;
			if (op == "DEC") return DEC;
			if (op == "MR") return MR;
			if (op == "FO") return FO;
		}
		
		static string getString(Operation::Type op)
		{
			switch (op)
			{
				case LP: return "LP"; break;
				//case CH: return "CH"; break;
				case SC: return "SC"; break;
				case INC: return "INC"; break;
				case DEC: return "DEC"; break;
				case MR: return "MR"; break;
				case FO: return "FO"; break;
				default: ofLogWarning("Operation") << "getString: Type not found";
			}
		}

		static unsigned int getRequiredStitches(Operation::Type op)
		{
			switch (op)
			{
			case LP: return 0; break;
			//case CH: return "1"; break;
			case SC: return 1; break;
			case INC: return 0; break;
			case DEC: return 2; break;
			//case MR: return 1; break;
			default: ofLogWarning("Operation") << "RequiredStitches: Type not found";
			}
		}

		static Operations parseOperation(Operation::Type op, unsigned int count)
		{
			// magic ring consists on "count inc"
			if (op == MR) return Operations(count, INC);
			// for the rest its simply the operation n times
			else
			{
				return Operations(count, op);
			}
		}
	};
}