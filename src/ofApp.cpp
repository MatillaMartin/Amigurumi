#include "ofApp.h"

using namespace ami;

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofEnableDepthTest();
	ofDisableArbTex();

	// configure cam
	cam.setDistance(20);
	cam.setNearClip(0.01f);
	cam.setFarClip(100.0f);

	patterns = PatternDigest::digest("pattern.xml");
	view.setPattern(&patterns[0]);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(0);
	cam.begin();
		ofDrawAxis(10);
		ofPushStyle();
			ofSetColor(255);
			view.render();
		ofPopStyle();
	cam.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == ' ')
	{
		patterns = PatternDigest::digest("pattern.xml");
		view.setPattern(&patterns[0]);
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
