#include "Amigurumi.h"

namespace ami
{
	Amigurumi::Amigurumi(const std::string & file)
		:
		m_pattern(0)
	{
		const static std::string tagAmi = "Amigurumi";
		const static std::string tagCommand = "Command";

		if (ofFile::doesFileExist(file))
		{
			ofxXmlSettings data;

			data.load(file);

			data.pushTag(tagAmi);
			// for each pattern
			for (unsigned int commandIndex = 0; commandIndex < data.getNumTags(tagCommand); commandIndex++)
			{
				std::string type = data.getAttribute(tagCommand, "type", "", commandIndex);
				data.pushTag(tagCommand, commandIndex);
				try
				{
					command(type, data);
				}
				catch (std::invalid_argument & e)
				{
					throw std::invalid_argument("Error reading Command " + ofToString(commandIndex) + ", Command of type " + ofToString(type) + " failed: " + e.what());
				}

				data.popTag();
			}
			data.popTag(); // ami
		}
		else
		{
			throw std::invalid_argument("File " + file + " not found");
		}
	}

	void Amigurumi::command(const std::string & type, ofxXmlSettings & data)
	{
		static std::unordered_map<std::string, std::function<std::unique_ptr<Command>(ofxXmlSettings & data)>> commandFactory
		{
			{ "Pattern", [this](ofxXmlSettings & data) { return make_unique<PatternCommand>(*this, data); } },
			{ "Join", [this](ofxXmlSettings & data) { return make_unique<JoinCommand>(*this, data); } },
		};

		auto & it = commandFactory.find(type);
		if (it != commandFactory.end())
		{
			it->second(data); // run command
		}
		else
		{
			throw std::invalid_argument("Type " + type + " is not a valid command");
		}
	}

	Amigurumi::PatternCommand::PatternCommand(Amigurumi & ami, ofxXmlSettings & data)
	{
		PatternDef def;
		const static std::string tagOperation = "Operation";
		const static std::string tagRound = "Round";
		const static std::string tagPattern = "Pattern";

		unsigned int id = data.getAttribute(tagPattern, "id", 0);

		data.pushTag(tagPattern);
		// for each row in the pattern
		for (unsigned int roundIndex = 0; roundIndex < data.getNumTags(tagRound); roundIndex++)
		{
			data.pushTag(tagRound, roundIndex);

			Operation::Operations op;
			// for each operation in the row
			for (unsigned int operationIndex = 0; operationIndex < data.getNumTags(tagOperation); operationIndex++)
			{
				std::string type = data.getAttribute(tagOperation, "type", "", operationIndex);
				unsigned int count = data.getAttribute(tagOperation, "count", 1, operationIndex);
				if (type != "")
				{
					// get operation
					std::unique_ptr<Operation::Operation> operation = Operation::getOperation(type, data, operationIndex);
					// parse for special operations (MR and others)
					Operation::Operations operations = Operation::parseOperation(*operation, count);
					op.insert(op.end(), std::make_move_iterator(operations.begin()), std::make_move_iterator(operations.end()));
				}
				else
				{
					ofLogVerbose("PatternDigest") << "Invalid operation: Round{" << roundIndex << "} Pattern {" << id;
				}
			}

			def.addRound(std::move(op));

			data.popTag(); // round
		}


		auto it = ami.m_patterns.find(id);
		if (it != ami.m_patterns.end())
		{
			ami.m_patterns[id].append(def);
		}
		else
		{
			ami.m_patterns[id] = def;
		}
		ami.m_pattern = id; // set id to last pattern added

		data.popTag();  // pattern
	}

	Amigurumi::JoinCommand::JoinCommand(Amigurumi & ami, ofxXmlSettings & data)
	{
		const static std::string tagJoin = "Join";
		if (
			!data.attributeExists(tagJoin, "pattern") ||
			!data.attributeExists(tagJoin, "with") ||
			!data.attributeExists(tagJoin, "make"))
		{
			throw std::invalid_argument("Invalid Join command, missing \"pattern\", \"with\" or \"make\" attributes");
		}

		unsigned int pattern = data.getAttribute(tagJoin, "pattern", 0);
		unsigned int with = data.getAttribute(tagJoin, "with", 0);
		unsigned int make = data.getAttribute(tagJoin, "make", 0);

		auto it_pattern = ami.m_patterns.find(pattern);
		auto it_with = ami.m_patterns.find(with);
		if (it_pattern == ami.m_patterns.end() || it_with == ami.m_patterns.end())
		{
			throw std::invalid_argument("Invalid Join command, \"pattern\" or \"with\" patterns do not exist");
		}
		else
		{
			ami.m_patterns[make] = it_pattern->second.join(it_with->second); // create a new pattern
			ami.m_pattern = make; // set id to last pattern added
		}
	}
}