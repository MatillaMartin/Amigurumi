#pragma once

#include "ofxXmlSettings.h"
#include "PatternGraph.h"

namespace ami
{
	class Amigurumi
	{
	public:
		Amigurumi(const std::string & file);
		PatternGraph & getPattern() { return m_patterns.at(m_pattern); }

	private:
		struct Command
		{
		};
		
		struct PatternCommand : public Command
		{
			PatternCommand(Amigurumi & ami, ofxXmlSettings & data);
		};

		struct JoinCommand : public Command
		{
			JoinCommand(Amigurumi & ami, ofxXmlSettings & data);
		};

		void command(const std::string & type, ofxXmlSettings & data);

		std::map<unsigned int, PatternGraph> m_patterns;
		unsigned int m_pattern;

	};

}