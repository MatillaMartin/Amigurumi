#pragma once

#include <vector>
#include <list>
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

		class Graph
		{
		public:

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

			Graph() {}

			NodeIterator addNode()
			{
				Node node;
				node.id = m_nodes.size();
				m_nodes.push_back(node);
				return NodeIterator(m_nodes, node.id);
			}

			void useNodes(unsigned int n)
			{
				for (unsigned int i = 0; i < n; i++)
				{
					m_outline.pop_front();
				}
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

			const std::vector<Node> & getNodes() const {
				return m_nodes;
			}
			const std::vector<Edge> & getEdges() const {
				return m_edges;
			}
			const std::vector<Face> & getFaces() const {
				return m_faces;
			}

		private:
			std::vector<Node> m_nodes;
			std::vector<Edge> m_edges;
			std::vector<Face> m_faces;
			std::deque<NodeIterator> m_outline;
		};


		PatternGraph(const PatternDef & pattern);
		PatternGraph(const Graph & graph);

		const std::vector<Node> & getNodes() const {
			return m_graph.getNodes();
		}
		const std::vector<Edge> & getEdges() const {
			return m_graph.getEdges();
		}
		const std::vector<Face> & getFaces() const {
			return m_graph.getFaces();
		}

		PatternGraph join(const PatternGraph & graph, std::vector<std::pair<Node, Node>> & joints)
		{
			
		}

	private:
		void addOperation(Operation::Type type);

		Graph m_graph;

	};
}