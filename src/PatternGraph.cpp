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

void PatternGraph::addOutline(const NodeIterator & it)
{
	m_outline.push_front(it.id);
}

bool PatternGraph::popOutline(unsigned int n)
{
	if (m_outline.size() < n)
	{
		return false;
	}

	for (unsigned int i = 0; i < n; i++)
	{
		m_outline.pop_back();
	}
	return true;
}

PatternGraph::NodeIterator PatternGraph::addNode(const Data & data)
{
	Node node;
	node.id = m_nodes.size();
	node.data = data;
	node.next = 0; // there is no next yet!
	NodeIterator it(m_nodes, node.id);
	m_nodes.push_back(node);

	if (!m_outline.empty())
	{
		NodeIterator last = front();
		it.node().last = last.id;
		last.node().next = node.id; // asign ourselves as our last's next
	}

	addOutline(it); // add to outline
	return it;
}

void PatternGraph::addEdge(ofIndexType from, ofIndexType to, float distance)
{
	Edge edge;
	edge.from = from;
	edge.to = to;
	edge.distance = distance;
	m_edges.push_back(edge);
}

void PatternGraph::addJoint(ofIndexType from, ofIndexType to)
{
	Joint joint;
	joint.from = from;
	joint.to = to;
	m_joints.push_back(joint);
}

void PatternGraph::addFace(ofIndexType a, ofIndexType b, ofIndexType c)
{
	if (a == b || a == c || b == c) return; // check triangles are valid
	Face face;
	face.ids[0] = a;
	face.ids[1] = b;
	face.ids[2] = c;
	m_faces.push_back(face);
}

PatternGraph PatternGraph::join(const PatternGraph & graph)
{
	PatternGraph pattern(*this);

	// save number of nodes, edges and faces
	unsigned int nnodes = pattern.getNodes().size();
	unsigned int nedges = pattern.getEdges().size();
	unsigned int nfaces = pattern.getFaces().size();
	unsigned int njoints = pattern.getJoints().size();
	unsigned int noutline = graph.getOutline().size();

	// append new nodes, edges and faces
	pattern.m_nodes.insert(pattern.m_nodes.end(), graph.getNodes().begin(), graph.getNodes().end());
	pattern.m_edges.insert(pattern.m_edges.end(), graph.getEdges().begin(), graph.getEdges().end());
	pattern.m_faces.insert(pattern.m_faces.end(), graph.getFaces().begin(), graph.getFaces().end());
	pattern.m_joints.insert(pattern.m_joints.end(), graph.getJoints().begin(), graph.getJoints().end());
	pattern.m_outline.insert(pattern.m_outline.begin(), graph.m_outline.begin(), graph.m_outline.end());

	// transform nodes, edges and faces so they have unique ids;
	std::transform(pattern.m_nodes.begin() + nnodes, pattern.m_nodes.end(), pattern.m_nodes.begin() + nnodes,
		[&nnodes](Node & node)
	{
		node.id += nnodes;
		node.last += nnodes;
		node.next += nnodes;
		//node.under += nnodes;
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

	std::transform(pattern.m_joints.begin() + njoints, pattern.m_joints.end(), pattern.m_joints.begin() + njoints,
		[&nnodes](Joint & joint)
	{
		joint.from += nnodes;
		joint.to += nnodes;
		return joint;
	});

	std::transform(pattern.m_outline.begin(), pattern.m_outline.begin() + noutline, pattern.m_outline.begin(),
		[&nnodes, &pattern](ofIndexType & it)
	{
		it += nnodes;
		return it;
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

	NodeIterator under = back();
	if (!popOutline()) // use up a node
	{
		throw std::invalid_argument("Not enough points to apply DEC");
	}
	NodeIterator under2 = back();
	if (!popOutline()) // use up a node
	{
		throw std::invalid_argument("Not enough points to apply DEC");
	}
	NodeIterator under3 = back();

	addEdge(node.id, node.last, 1.f);
	addEdge(node.id, under.id, 1.f);
	addEdge(node.id, under2.id, 1.f);
	addEdge(node.id, under3.id, 1.f);

	addFace(node.id, node.last, under.id);
	addFace(node.id, under.id, under2.id);
	addFace(node.id, under2.id, under3.id);
}

void PatternGraph::apply(const Operation::FinishOff & op)
{
	// this operation does not add a new vertex
	NodeIterator under = back();
	if (!popOutline()) // use up a node
	{
		throw std::invalid_argument("Not enough points to apply FO");
	}
	NodeIterator under2 = back();


	NodeIterator nextClose = under2;
	NodeIterator lastClose = front();
	while (nextClose < lastClose) // until last and next meets. Careful!
	{
		// add constraint
		addJoint(nextClose.id, lastClose.id);
		if (!popOutline()) // use up a node
		{
			throw std::invalid_argument("Not enough points to apply FO");
		}

		nextClose = back(); // advance next
		lastClose = lastClose.last(); // go back in last
	}
}

void PatternGraph::apply(const Operation::Increase & op)
{
	NodeIterator node_it = addNode(Data{ op.type });
	Node & node = node_it.node();

	NodeIterator under = back();

	addEdge(node.id, node.last, 1.f);
	addEdge(node.id, under.id, 1.f);

	addFace(node.id, node.last, under.id);
}

void PatternGraph::apply(const Operation::Loop & op)
{
	NodeIterator node_it = addNode(Data{ op.type });
	Node & node = node_it.node();

	// loop only adds the base stitch
	//node.under = node.id;
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

	NodeIterator under = back();
	if (!popOutline()) // use up a node
	{
		throw std::invalid_argument("Not enough points to apply SC");
	}
	NodeIterator under2 = back();

	addEdge(node.id, node.last, 1.f);
	addEdge(node.id, under.id, 1.f);
	addEdge(node.id, under2.id, 1.f);

	addFace(node.id, node.last, under.id);
	addFace(node.id, under.id, under2.id);
}

void PatternGraph::apply(const Operation::SlipStitch & op)
{
	// this operation does not add a new vertex
	NodeIterator under = back();
	if (!popOutline()) // use up a node
	{
		throw std::invalid_argument("Not enough points to apply SLST");
	}
	NodeIterator under2 = back();
	NodeIterator node = front();
	addEdge(node.id, under2.id, 1.f);
	addFace(node.id, under.id, under2.id);
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