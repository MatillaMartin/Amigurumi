#include "ofApp.h"

using namespace ami;

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofEnableDepthTest();
	ofDisableArbTex();

	ofSetFrameRate(60.0f);

	// configure cam
	cam.setDistance(30);
	cam.setNearClip(0.01f);
	cam.setFarClip(10000.0f);

	filepath = "pattern.xml";

	patterns = PatternDigest::digest(filepath);
	view.setPattern(&patterns[0]);

	helpInfo =
		string("Left click to move camera \n") +
		"Right click to zoom \n" +
		"Space to reset \n" +
		"L to load new pattern \n"
		;
}

//--------------------------------------------------------------
void ofApp::update(){
	view.update(ofGetLastFrameTime()*10.0f);
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
