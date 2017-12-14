#pragma once

#include "ofMain.h"

#include "PatternDigest.h"
#include "PatternView.h"

using namespace ami;

class ofApp : public ofBaseApp {

public:
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

	string helpInfo;

	string filepath;
	ofEasyCam cam;
	PatternView view;
	vector<PatternDef> patterns;

	float leftOverTime;
	float fixedUpdateMillis;
	float fps;
	bool bRun;
};