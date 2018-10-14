#include "PatternGraph.h"
#include "ofUtils.h"

using namespace ami;

PatternGraph::PatternGraph(const PatternDef & pattern)
{
	unsigned int roundIndex = 0;
	for (auto & round : pattern.getRounds())
	{
		unsigned int operationIndex = 0;
		for (auto & op : round)
		{
			try
			{
				addOperation(op);
			}
			catch (std::invalid_argument & e)
			{
				std::stringstream ss;
				ss << "Round " << ofToString(roundIndex) << ", Operation " << ofToString(operationIndex) << " failed: " << e.what();
				throw std::invalid_argument(ss.str());
			}

			operationIndex++;
		}
		roundIndex++;
	}
}

void PatternGraph::addOperation(Operation::Type type)
{
	// Update next as necessary

	// Operations that do NOT add nodes
	switch (type)
	{
		case Operation::Type::FO:
		{
			// this operation does not add a new vertex
			Graph::NodeIterator nextClose = m_graph.back().under().next();
			Graph::NodeIterator lastClose = m_graph.back();
			while (nextClose < lastClose) // until last and next meets. Careful!
			{
				// add constraint
				m_graph.addEdge(nextClose.id, lastClose.id, 0.f);

				nextClose = nextClose.next(); // advance next
				lastClose = lastClose.last(); // go back in last
			}
			return;
		}
	}

	// Operations that add nodes, add node first

	Graph::NodeIterator node_it = m_graph.addNode();
	Node & node = node_it.node();

	node.data.op = type;
	node.next = 0; // there is no next yet!
	node.last = 0; // default
	if (node.id > 0)
	{
		node.last = node.id - 1;
		node_it.last().node().next = node.id; // asign ourselves as our last's next
	}

	switch (type)
	{
		case Operation::Type::LP:
		{
			// loop only adds the base stitch
			node.under = node.id;
			node.last = node.id;
			node.next = node.id;
			break;
		}

		case Operation::Type::SC:
		{
			// TODO: check if we have enough stitches

			node.under = node_it.last().under().next().id;

			m_graph.addEdge(node.id, node.last, 1.f);
			m_graph.addEdge(node.id, node.under, 1.f);
			m_graph.addEdge(node.id, node_it.last().under().id, 1.f);

			m_graph.addFace(node.id, node.last, node_it.last().under().id);
			m_graph.addFace(node.id, node_it.last().under().id, node.under);

			break;
		}
		case Operation::Type::INC:
		{
			// same under as last stitch
			node.under = node_it.last().under().id;

			m_graph.addEdge(node.id, node.last, 1.f);
			m_graph.addEdge(node.id, node.under, 1.f);

			m_graph.addFace(node.id, node.last, node_it.last().under().id);

			break;
		}
		case Operation::Type::DEC:
		{
			// TODO: check if we have enough stitches

			// under is next next stitch, we are connecting 2
			node.under = node_it.last().under().next().next().id;


			m_graph.addEdge(node.id, node.last, 1.f);
			m_graph.addEdge(node.id, node_it.last().under().id, 1.f);
			m_graph.addEdge(node.id, node_it.last().under().next().id, 1.f);
			m_graph.addEdge(node.id, node.under, 1.f);

			m_graph.addFace(node.id, node.last, node_it.last().under().id);
			m_graph.addFace(node.id, node_it.last().under().id, node_it.under().last().id);
			m_graph.addFace(node.id, node_it.under().last().id, node.under);

			break;
		}

		default:
		{
			ofLogVerbose("PatternMesh") << "Operation not supported";
			throw std::invalid_argument("Operation not supported");
		}
	}
}
