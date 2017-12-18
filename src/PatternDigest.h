#pragma once

#include "ofUtils.h"
#include "ofxXmlSettings.h"
#include "PatternDef.h"

namespace ami
{
	class PatternDigest
	{
	public:
		static vector<PatternDef> digest(const string & file);
	};

}