#pragma once

#include "ofMain.h"

#include "Amigurumi.h"
#include "PatternView.h"

using namespace ami;

class ofApp : public ofBaseApp {

public:
	struct Settings
	{
		bool step;
	};

	ofApp(const ofApp::Settings & settings);
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	void updateStep(float dt);

	std::string m_helpInfo;

	std::string m_filepath;
	ofEasyCam m_cam;
	std::unique_ptr<Amigurumi> m_ami;
	std::unique_ptr<PatternView::Params> m_viewParams;
	std::unique_ptr<PatternView> m_view;

	float m_leftOverTime;
	float m_fixedUpdateMillis;
	float m_fps;
	bool m_bRun;
	PatternView::RenderSettings m_viewRenderSettings;

	ofApp::Settings m_settings;
};