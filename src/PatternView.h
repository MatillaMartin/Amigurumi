#pragma once

#include "PatternDef.h"
#include "PatternMesh.h"

namespace ami
{
	class PatternView
	{
	public:
		struct RenderSettings
		{
			bool debug;
		};

		class Params
		{
		public:
			Params(const string & filepath)
			{
				const static std::string tagView = "View";
				const static std::string tagAnchor = "Anchor";

				if (ofFile::doesFileExist(filepath))
				{
					ofxXmlSettings data;

					data.load(filepath);

					data.pushTag(tagView);
					// for each anchor
					anchors.reserve(data.getNumTags(tagAnchor));

					for (unsigned int anchorIndex = 0; anchorIndex < data.getNumTags(tagAnchor); anchorIndex++)
					{
						unsigned int node;
						if (data.attributeExists(tagAnchor, "node", anchorIndex))
						{
							node = data.getAttribute(tagAnchor, "node", 0, anchorIndex);
						}
						else
						{
							throw std::invalid_argument("Error reading View: Anchor " + ofToString(anchorIndex) + " does not contain node attribute");
						}

						glm::vec3 anchor;
						if (data.attributeExists(tagAnchor, "x", anchorIndex) &&
							data.attributeExists(tagAnchor, "y", anchorIndex) &&
							data.attributeExists(tagAnchor, "z", anchorIndex))
						{
							anchor.x = data.getAttribute(tagAnchor, "x", 0.f, anchorIndex);
							anchor.y = data.getAttribute(tagAnchor, "y", 0.f, anchorIndex);
							anchor.z = data.getAttribute(tagAnchor, "z", 0.f, anchorIndex);
						}
						else
						{
							throw std::invalid_argument("Error reading View: Anchor " + ofToString(anchorIndex) + " does not contain xyz attributes");
						}

						anchors.push_back(PatternMesh::Anchor({ node, anchor }));
					}
					data.popTag(); // view

				}
				else
				{
					throw std::invalid_argument("File " + filepath + " not found");
				}
			}

			std::vector<PatternMesh::Anchor> anchors;
		};

		PatternView();
		PatternView(const PatternGraph & pattern, const Params & params);

		void render(const PatternView::RenderSettings & settings);

		void update(float deltaTime);

		PatternMesh m_mesh;

	private:
		struct StepByStep
		{
			bool bStep;
			std::vector<PatternDef::Round>::const_iterator nextRound;
			std::vector<PatternDef::Round>::const_iterator lastRound;
			float lastMillis;
			float stepPeriod;
		} m_sbs;
	};
}