#pragma once

#include <vector>
#include <list>
#include <deque>
#include "ofMath.h"
#include "PatternDef.h"

namespace ami
{
	class PatternGraph
	{
	public:
		struct Data
		{
			Operation::Type op;
		};

		struct Face
		{
			ofIndexType ids[3];
		};

		struct Edge
		{
			ofIndexType from = 0;
			ofIndexType to = 0;
			float distance = 0.f;
		};

		struct Node
		{
			ofIndexType id = 0;

			ofIndexType last;
			ofIndexType under;
			ofIndexType next;

			Data data;
		};

		class NodeIterator
		{
		public:
			NodeIterator(std::vector<Node> & nodes, ofIndexType id) : m_nodes(&nodes), id(id) {}

			NodeIterator last() { return NodeIterator(*m_nodes, node().last); }
			NodeIterator under() { return NodeIterator(*m_nodes, node().under); }
			NodeIterator next() { return NodeIterator(*m_nodes, node().next); }

			Node & node() { return m_nodes->at(id); }

			ofIndexType id;

			bool operator!=(NodeIterator & other)
			{
				return id != other.id;
			}

			bool operator>(NodeIterator & other)
			{
				return id > other.id;
			}

			bool operator<(NodeIterator & other)
			{
				return id < other.id;
			}

			void operator++()
			{
				++id;
			}

			void operator--()
			{
				--id;
			}

		private:
			std::vector<Node> * m_nodes;
		};

		PatternGraph();
		PatternGraph(const PatternDef & pattern);
		
		void append(const PatternDef & pattern);

		PatternGraph join(const PatternGraph & graph);

		const std::vector<Node> & getNodes() const {
			return m_nodes;
		}
		const std::vector<Edge> & getEdges() const {
			return m_edges;
		}
		const std::vector<Face> & getFaces() const {
			return m_faces;
		}
		const std::deque<NodeIterator> & getOutline() const {
			return m_outline;
		}

		// visitor pattern
		void apply(const Operation::Chain & op);
		void apply(const Operation::Decrease & op);
		void apply(const Operation::FinishOff & op);
		void apply(const Operation::Increase & op);
		void apply(const Operation::Loop & op);
		void apply(const Operation::MagicRing & op);
		void apply(const Operation::SingleCrochet & op);
		void apply(const Operation::SlipStitch & op);
		void apply(const Operation::Operation & op);

	private:

		NodeIterator addNode(const Data & data)
		{
			Node node;
			node.id = m_nodes.size();
			node.data = data;
			node.next = 0; // there is no next yet!
			NodeIterator it(m_nodes, node.id);
			m_nodes.push_back(node);

			if (node.id > 0)
			{
				it.node().last = node.id - 1; 
				it.last().node().next = node.id; // asign ourselves as our last's next
			}

			addOutline(it);
			return it;
		}

		void addOutline(const NodeIterator & it)
		{
			m_outline.push_back(it);
		}

		bool popOutline(unsigned int n = 1)
		{
			if (m_outline.size() < n)
			{
				return false;
			}

			for (unsigned int i = 0; i < n; i++)
			{
				m_outline.pop_front();
			}
			return true;
		}

		void addEdge(ofIndexType from, ofIndexType to, float distance)
		{
			Edge edge;
			edge.from = from;
			edge.to = to;
			edge.distance = distance;
			m_edges.push_back(edge);
		}

		void addFace(ofIndexType a, ofIndexType b, ofIndexType c)
		{
			if (a == b || a == c || b == c) return; // check triangles are valid
			Face face;
			face.ids[0] = a;
			face.ids[1] = b;
			face.ids[2] = c;
			m_faces.push_back(face);
		}

		NodeIterator at(ofIndexType id)
		{
			return NodeIterator(m_nodes, id);
		}

		NodeIterator front()
		{
			return NodeIterator(m_nodes, m_outline.front().id);
		}

		NodeIterator back()
		{
			return NodeIterator(m_nodes, m_outline.back().id);
		}

		NodeIterator get(ofIndexType id)
		{
			return NodeIterator(m_nodes, id);
		}

	private:
		std::vector<Node> m_nodes;
		std::vector<Edge> m_edges;
		std::vector<Face> m_faces;
		std::deque<NodeIterator> m_outline;
	};
}