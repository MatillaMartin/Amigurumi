#pragma once

#include "ofLog.h"
#include "ofxXmlSettings.h"
#include <map>

namespace ami
{
	class PatternGraph;
	namespace Operation
	{
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
			// slip stitch
			SLST,
			// join
			JOIN,
			// nothing
			NONE
		};

		class Operation
		{
		public:
			Operation(Type type) : type(type) {};
			Type type;
			virtual Operation * clone() const = 0;
			virtual void apply(ami::PatternGraph & pattern) = 0;
		};

		typedef std::vector<std::unique_ptr<Operation>> Operations;

		struct Loop : public Operation
		{
			Loop();
			Loop * clone() const override { return new Loop(*this); }
			void apply(ami::PatternGraph & pattern) override;
		};
		struct Chain : public Operation
		{
			Chain();
			Chain * clone() const override { return new Chain(*this); }
			void apply(ami::PatternGraph & pattern) override;
		};
		struct SingleCrochet : public Operation
		{
			SingleCrochet();
			SingleCrochet * clone() const  override { return new SingleCrochet(*this); }
			void apply(ami::PatternGraph & pattern) override;
		};
		struct Increase : public Operation
		{
			Increase();
			Increase * clone() const  override { return new Increase(*this); }
			void apply(ami::PatternGraph & pattern) override;
		};
		struct Decrease : public Operation
		{
			Decrease();
			Decrease * clone() const  override { return new Decrease(*this); }
			void apply(ami::PatternGraph & pattern) override;
		};
		struct MagicRing : public Operation
		{
			MagicRing();
			MagicRing *clone() const  override { return new MagicRing(*this); }
			void apply(ami::PatternGraph & pattern) override;
		};
		struct SlipStitch : public Operation
		{
			SlipStitch();
			SlipStitch *clone() const  override { return new SlipStitch(*this); }
			void apply(ami::PatternGraph & pattern) override;
		};
		struct Join : public Operation
		{
			Join(unsigned int node = 0, unsigned int with = 0);
			Join *clone() const  override { return new Join(*this); }
			void apply(ami::PatternGraph & pattern) override;
			unsigned int node;
			unsigned int with;
		};
		struct FinishOff : public Operation
		{
			FinishOff();
			FinishOff *clone() const  override { return new FinishOff(*this); }
			void apply(ami::PatternGraph & pattern) override;
		};

		static std::unique_ptr<Operation> getOperation(const std::string & type, ofxXmlSettings & data, int which)
		{
			static std::unordered_map<std::string, std::function<std::unique_ptr<Operation>(ofxXmlSettings&, int)>> operationFactory
			{
				{ "LP",		[](ofxXmlSettings & data, int which) { return std::make_unique<Loop>(); } },
				{ "SC",		[](ofxXmlSettings & data, int which) { return std::make_unique<SingleCrochet>(); } },
				{ "INC",	[](ofxXmlSettings & data, int which) { return std::make_unique<Increase>(); } },
				{ "DEC",	[](ofxXmlSettings & data, int which) { return std::make_unique<Decrease>(); } },
				{ "MR",		[](ofxXmlSettings & data, int which) { return std::make_unique<MagicRing>(); } },
				{ "SLST",	[](ofxXmlSettings & data, int which) { return std::make_unique<SlipStitch>(); } },
				{ "JOIN",	[](ofxXmlSettings & data, int which)
					{
						unsigned int node = data.getAttribute("Operation", "node", 0, which);
						unsigned int with = data.getAttribute("Operation", "with", 0, which);
						return std::make_unique<Join>(node, with);
					}
				},
				{ "FO",		[](ofxXmlSettings & data, int which) { return std::make_unique<FinishOff>(); } }
			};

			auto & it = operationFactory.find(type);
			if (it != operationFactory.end())
			{
				return it->second(data, which); // run command
			}
			else
			{
				throw std::invalid_argument("Operation " + type + " is not a valid operation");
			}
		}

		static std::string getString(Type op)
		{
			const static std::map<Type, std::string> m_operations =
			{
				{ Type::LP, "LP" },
				{ Type::SC, "SC" },
				{ Type::INC, "INC" },
				{ Type::DEC, "DEC" },
				{ Type::MR, "MR" },
				{ Type::SLST, "SLST" },
				{ Type::JOIN, "JOIN" },
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

		static Operations parseOperation(const Operation & op, unsigned int count)
		{
			Operations ops;
			ops.reserve(count);
			if (op.type == Type::MR)
			{
				for (unsigned int i = 0; i < count; i++)
				{
					ops.push_back(std::make_unique<Increase>());
				}
			}
			// for the rest its simply the operation n times
			else
			{
				for (unsigned int i = 0; i < count; i++)
				{
					ops.push_back(std::unique_ptr<Operation>(op.clone()));
				}
			}

			return ops;
		}
	}
}