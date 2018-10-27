#include "PatternGraph.h"
#include "ofUtils.h"

using namespace ami;

PatternGraph::PatternGraph()
{}

PatternGraph::PatternGraph(const PatternDef & pattern)
{
	append(pattern);
}

void PatternGraph::append(const PatternDef & pattern)
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

PatternGraph PatternGraph::join(const PatternGraph & graph)
{
	PatternGraph pattern(*this);

	// save number of nodes, edges and faces
	unsigned int nnodes = pattern.getNodes().size();
	unsigned int nedges = pattern.getEdges().size();
	unsigned int nfaces = pattern.getFaces().size();

	// append new nodes, edges and faces
	pattern.m_nodes.insert(pattern.m_nodes.end(), graph.getNodes().begin(), graph.getNodes().end());
	pattern.m_edges.insert(pattern.m_edges.end(), graph.getEdges().begin(), graph.getEdges().end());
	pattern.m_faces.insert(pattern.m_faces.end(), graph.getFaces().begin(), graph.getFaces().end());

	// transform nodes, edges and faces so they are unique;
	std::transform(pattern.m_nodes.begin() + nnodes, pattern.m_nodes.end(), pattern.m_nodes.begin() + nnodes,
		[&nnodes](Node & node)
	{
		node.id += nnodes;
		node.last += nnodes;
		node.next += nnodes;
		node.under += nnodes;
		return node;
	});

	std::transform(pattern.m_edges.begin() + nedges, pattern.m_edges.end(), pattern.m_edges.begin() + nedges,
		[&nnodes](Edge & edge)
	{
		edge.from += nnodes;
		edge.to += nnodes;
		return edge;
	});

	std::transform(pattern.m_faces.begin() + nfaces, pattern.m_faces.end(), pattern.m_faces.begin() + nfaces,
		[&nnodes](Face & face)
	{
		for (auto & id : face.ids)
		{
			id += nnodes;
		}
		return face;
	});

	return pattern;
}

void PatternGraph::addOperation(Operation::Type type)
{
	// return number of nodes used
	unsigned int nodesUsed = 0;

	// Update next as necessary

	// Operations that do NOT add nodes
	switch (type)
	{
		case Operation::Type::FO:
		{
			// this operation does not add a new vertex
			NodeIterator nextClose = back().under().next();
			NodeIterator lastClose = back();
			while (nextClose < lastClose) // until last and next meets. Careful!
			{
				// add constraint
				addEdge(nextClose.id, lastClose.id, 0.f);

				nextClose = nextClose.next(); // advance next
				lastClose = lastClose.last(); // go back in last
			}
			return;
		}
		case Operation::Type::SLST:
		{

		}
	}

	// Operations that add nodes, add node first

	NodeIterator node_it = addNode();
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

			addEdge(node.id, node.last, 1.f);
			addEdge(node.id, node.under, 1.f);
			addEdge(node.id, node_it.last().under().id, 1.f);

			addFace(node.id, node.last, node_it.last().under().id);
			addFace(node.id, node_it.last().under().id, node.under);

			if (!popOutline()) // we have used up 1 nodes from the outline
			{
				throw std::invalid_argument("Not enough points to apply SC");
			}
			break;
		}
		case Operation::Type::INC:
		{
			// same under as last stitch
			node.under = node_it.last().under().id;

			addEdge(node.id, node.last, 1.f);
			addEdge(node.id, node.under, 1.f);

			addFace(node.id, node.last, node_it.last().under().id);

			break;
		}
		case Operation::Type::DEC:
		{
			// TODO: check if we have enough stitches

			// under is next next stitch, we are connecting 2
			node.under = node_it.last().under().next().next().id;


			addEdge(node.id, node.last, 1.f);
			addEdge(node.id, node_it.last().under().id, 1.f);
			addEdge(node.id, node_it.last().under().next().id, 1.f);
			addEdge(node.id, node.under, 1.f);

			addFace(node.id, node.last, node_it.last().under().id);
			addFace(node.id, node_it.last().under().id, node_it.under().last().id);
			addFace(node.id, node_it.under().last().id, node.under);

			if (!popOutline(2)) // we have used up 2 nodes from the outline
			{
				throw std::invalid_argument("Not enough points to apply DEC");
			}

			break;
		}

		default:
		{
			ofLogVerbose("PatternMesh") << "Operation not supported";
			throw std::invalid_argument("Operation not supported");
		}
	}
}