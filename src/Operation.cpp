#include "Operation.h"
#include "PatternGraph.h"

using namespace ami::Operation;

Loop::Loop()
	:
	OperationCRTP(Type::LP)
{}


Chain::Chain()
	:
	OperationCRTP(Type::CH)
{}


SingleCrochet::SingleCrochet()
	:
	OperationCRTP(Type::SC)
{}


Increase::Increase()
	:
	OperationCRTP(Type::INC)
{}


Decrease::Decrease()
	:
	OperationCRTP(Type::DEC)
{}


MagicRing::MagicRing()
	:
	OperationCRTP(Type::MR)
{}


SlipStitch::SlipStitch()
	:
	OperationCRTP(Type::SLST)
{}

Join::Join(unsigned int node, unsigned int with)
	:
	OperationCRTP(Type::JOIN),
	node(node),
	with(with)
{}

FinishOff::FinishOff()
	:
	OperationCRTP(Type::FO)
{}