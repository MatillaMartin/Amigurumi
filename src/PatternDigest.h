#pragma once

#include "ofUtils.h"
#include "ofxXmlSettings.h"
#include "PatternDef.h"

namespace ami
{
	class PatternDigest
	{
	public:
		const static string tagPattern;
		const static string tagRound;
		const static string tagOperation;
		const static string attCount;

		static vector<PatternDef> digest(const string & file);
	};

}