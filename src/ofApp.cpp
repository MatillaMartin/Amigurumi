#include "ofApp.h"

using namespace ami;

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofEnableDepthTest();
	ofDisableArbTex();

	fps = 60.0f;
	ofSetFrameRate(fps);

	// configure cam
	cam.setDistance(30);
	cam.setNearClip(0.01f);
	cam.setFarClip(10000.0f);

	filepath = "whale.xml";

	patterns = PatternDigest::digest(filepath);
	view.setPattern(&patterns[0]);

	bRun = true;
	
	fixedUpdateMillis = 0.016f; // simulate update every 16ms
	leftOverTime = 0.0f;

	helpInfo =
		string("Left click to move camera \n") +
		"Right click to zoom \n" +
		"Space to reset \n" +
		"L to load new pattern \n" + 
		"P to pause and resume \n"
		;
}

//--------------------------------------------------------------
void ofApp::update(){
	if (bRun)
	{
		// fixed update time, lasting a total of a frame time
		float elapsedTime = 2.0f / fps; // maximum allowed elapsed time is twice the fps, to avoid peaks
		elapsedTime = std::fmin(elapsedTime, ofGetLastFrameTime() * 1.0f);

		// add time that couldn't be used last frame
		elapsedTime += leftOverTime;

		// divide it up in chunks of fixed updates
		unsigned int timesteps = std::floor(elapsedTime / fixedUpdateMillis);

		// store time we couldn't use for the next frame.
		leftOverTime = elapsedTime - timesteps * fixedUpdateMillis;

		// update with a fixed timestep many times
		for (unsigned int i = 0; i < timesteps; i++) {
			view.update(fixedUpdateMillis);
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(0);
	cam.begin();
		ofDrawAxis(10);
		view.render();
	cam.end();

	ofDrawBitmapStringHighlight(helpInfo, ofVec3f(50, 50, 0));
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == ' ')
	{
		patterns = PatternDigest::digest(filepath);
		view.setPattern(&patterns[0]);
	}
	if (key == 'l' || key == 'L')
	{
		auto res = ofSystemLoadDialog("Load pattern", false, "data");
		if (res.bSuccess)
		{
			filepath = res.filePath;
			patterns = PatternDigest::digest(filepath);
			view.setPattern(&patterns[0]);
		}
	}
	if (key == 'p' || key == 'P')
	{
		bRun = !bRun;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
