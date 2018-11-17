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

		struct Joint
		{
			ofIndexType from = 0;
			ofIndexType to = 0;
		};

		struct Node
		{
			ofIndexType id = 0;

			ofIndexType last;
			//ofIndexType under;
			ofIndexType next;

			Data data;
		};

		class NodeIterator
		{
		public:
			NodeIterator(std::vector<Node> & nodes, ofIndexType id) : m_nodes(&nodes), id(id) {}

			NodeIterator last() { return NodeIterator(*m_nodes, node().last); }
			//NodeIterator under() { return NodeIterator(*m_nodes, node().under); }
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
		const std::vector<Joint> & getJoints() const {
			return m_joints;
		}
		const std::vector<Face> & getFaces() const {
			return m_faces;
		}
		const std::deque<ofIndexType> & getOutline() const {
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
		void apply(const Operation::Join & op);
		void apply(const Operation::Operation & op);

	private:

		NodeIterator addNode(const Data & data);

		void addOutline(const NodeIterator & it);

		bool popOutline(unsigned int n = 1);

		void addEdge(ofIndexType from, ofIndexType to, float distance);

		void addJoint(ofIndexType from, ofIndexType to);

		void addFace(ofIndexType a, ofIndexType b, ofIndexType c);

		NodeIterator at(ofIndexType id)
		{
			return NodeIterator(m_nodes, id);
		}

		NodeIterator front()
		{
			return NodeIterator(m_nodes, m_outline.front());
		}

		NodeIterator back()
		{
			return NodeIterator(m_nodes, m_outline.back());
		}

		NodeIterator get(ofIndexType id)
		{
			return NodeIterator(m_nodes, id);
		}

	private:
		std::vector<Node> m_nodes;
		std::vector<Edge> m_edges;
		std::vector<Joint> m_joints;
		std::vector<Face> m_faces;
		std::deque<ofIndexType> m_outline;
	};
}