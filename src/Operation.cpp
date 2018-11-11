#include "Operation.h"
#include "PatternGraph.h"

using namespace ami::Operation;

Loop::Loop()
	:
	Operation(Type::LP)
{}


Chain::Chain()
	:
	Operation(Type::CH)
{}


SingleCrochet::SingleCrochet()
	:
	Operation(Type::SC)
{}


Increase::Increase()
	:
	Operation(Type::INC)
{}


Decrease::Decrease()
	:
	Operation(Type::DEC)
{}


MagicRing::MagicRing()
	:
	Operation(Type::MR)
{}


SlipStitch::SlipStitch()
	:
	Operation(Type::SLST)
{}

Join::Join(unsigned int node, unsigned int with)
	:
	Operation(Type::JOIN),
	node(node),
	with(with)
{}

FinishOff::FinishOff()
	:
	Operation(Type::FO)
{}


// visitor pattern for graph
void Loop::apply(ami::PatternGraph & pattern) { pattern.apply(*this); }
void Chain::apply(ami::PatternGraph & pattern) { pattern.apply(*this); }
void SingleCrochet::apply(ami::PatternGraph & pattern) { pattern.apply(*this); }
void Increase::apply(ami::PatternGraph & pattern) { pattern.apply(*this); }
void Decrease::apply(ami::PatternGraph & pattern) { pattern.apply(*this); }
void MagicRing::apply(ami::PatternGraph & pattern) { pattern.apply(*this); }
void SlipStitch::apply(ami::PatternGraph & pattern) { pattern.apply(*this); }
void Join::apply(ami::PatternGraph & pattern) { pattern.apply(*this); }
void FinishOff::apply(ami::PatternGraph & pattern) { pattern.apply(*this); }
