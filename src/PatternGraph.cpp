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
				op->apply(*this);
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

	// transform nodes, edges and faces so they have unique ids;
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

void PatternGraph::apply(const Operation::Chain & op)
{
	// nothing here
}

void PatternGraph::apply(const Operation::Decrease & op)
{
	NodeIterator node_it = addNode(Data{ op.type });
	Node & node = node_it.node();

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
}

void PatternGraph::apply(const Operation::FinishOff & op)
{
	// this operation does not add a new vertex
	NodeIterator nextClose = back().under().next();
	NodeIterator lastClose = back();
	while (nextClose < lastClose) // until last and next meets. Careful!
	{
		// add constraint
		addJoint(nextClose.id, lastClose.id);

		nextClose = nextClose.next(); // advance next
		lastClose = lastClose.last(); // go back in last
	}
}

void PatternGraph::apply(const Operation::Increase & op)
{
	NodeIterator node_it = addNode(Data{ op.type });
	Node & node = node_it.node();

	node.under = node_it.last().under().id;

	addEdge(node.id, node.last, 1.f);
	addEdge(node.id, node.under, 1.f);

	addFace(node.id, node.last, node_it.last().under().id);
}

void PatternGraph::apply(const Operation::Loop & op)
{
	NodeIterator node_it = addNode(Data{ op.type });
	Node & node = node_it.node();

	// loop only adds the base stitch
	node.under = node.id;
	node.last = node.id;
	node.next = node.id;
}

void PatternGraph::apply(const Operation::MagicRing & op)
{
	// nothing here
}

void PatternGraph::apply(const Operation::SingleCrochet & op)
{
	NodeIterator node_it = addNode(Data{ op.type });
	Node & node = node_it.node();

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
}

void PatternGraph::apply(const Operation::SlipStitch & op)
{
	// this operation does not add a new vertex
	NodeIterator under = back().under().next();
	NodeIterator node = back();
	addEdge(node.id, under.id, 0.f);
}

void PatternGraph::apply(const Operation::Join & op)
{
	// this operation does not add a new vertex, just joins them
	addJoint(op.node, op.with);
}

void PatternGraph::apply(const Operation::Operation & op)
{
	ofLogVerbose("PatternMesh") << "Operation not supported";
	throw std::invalid_argument("Operation not supported");
}