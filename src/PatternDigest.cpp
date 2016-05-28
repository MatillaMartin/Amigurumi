#include "PatternDigest.h"

namespace ami
{
	const string PatternDigest::tagPattern = "Pattern";
	const string PatternDigest::tagRound = "Round";
	const string PatternDigest::tagOperation = "Operation";
	const string PatternDigest::attCount = "Count";

	vector<PatternDef> PatternDigest::digest(const string & file)
	{
		vector<PatternDef> patterns;

		if (ofFile::doesFileExist(file))
		{
			ofxXmlSettings data;

			data.load(file);

			// for each pattern
			for (unsigned int patternIndex = 0; patternIndex < data.getNumTags(tagPattern); patternIndex++)
			{
				PatternDef def;

				data.pushTag(tagPattern, patternIndex);
				// for each row in the pattern
				for (unsigned int roundIndex = 0; roundIndex < data.getNumTags(tagRound); roundIndex++)
				{
					data.pushTag(tagRound, roundIndex);

					Operation::Operations op;
					// for each operation in the row
					for (unsigned int operationIndex = 0; operationIndex < data.getNumTags(tagOperation); operationIndex++)
					{
						string operation = data.getValue(tagOperation, string(""), operationIndex);
						unsigned int count = data.getAttribute(tagOperation, attCount, 0);
						if (operation != "")
						{
							// get operation as type
							Operation::Type type = Operation::getOperation(operation);
							// parse for special operations (MR and others)
							vector<Operation::Type> operations = Operation::parseOperation(type, count);
							op.insert(op.end(), operations.begin(), operations.end());
						}
						else
						{
							ofLogVerbose("PatternDigest") << "Invalid operation: Round{" << roundIndex << "} Pattern {" << patternIndex;
						}
					}

					def.addRound(op);

					data.popTag();
				}

				patterns.push_back(def);
				data.popTag();
			}
		}
		else
		{
			ofLogVerbose("PatternData") << "File " << file << " not found";
		}

		return patterns;
	}
}