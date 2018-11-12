#include "ofApp.h"
#include "PatternGraph.h"

//--------------------------------------------------------------
ofApp::ofApp(const ofApp::Settings & settings)
	:
	m_settings(settings),
	m_viewRenderSettings({ false })
{}

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofEnableDepthTest();
	ofDisableArbTex();

	m_fps = 60.0f;
	ofSetFrameRate(m_fps);

	// configure cam
	m_cam.setDistance(30);
	m_cam.setNearClip(1.0f);
	m_cam.setFarClip(1000.0f);

	m_filepath = "llama.xml";

	try
	{
		m_ami = make_unique<Amigurumi>(m_filepath);
		m_viewParams = make_unique<PatternView::Params>(m_filepath);
		m_view = make_unique<PatternView>(m_ami->getPattern(), *m_viewParams);
	}
	catch (std::invalid_argument & e)
	{
		ofLogError("ofApp") << "Pattern graph failed: " << e.what();
		ofExit();
	}

	m_bRun = true;
	
	m_fixedUpdateMillis = 0.016f; // simulate update every 16ms
	m_leftOverTime = 0.0f;

	m_helpInfo =
		string("Left click to move camera \n") +
		"Right click to zoom \n" +
		"Space to reset \n" +
		"L to load new pattern \n" + 
		"P to pause and resume \n" +
		"S to run an update step \n"
		;
}

//--------------------------------------------------------------
void ofApp::update(){
	if (m_bRun)
	{
		// fixed update time, lasting a total of a frame time
		float elapsedTime = 2.0f / m_fps; // maximum allowed elapsed time is twice the fps, to avoid peaks
		elapsedTime = std::fmin(elapsedTime, ofGetLastFrameTime() * 1.0f);

		// add time that couldn't be used last frame
		elapsedTime += m_leftOverTime;

		// divide it up in chunks of fixed updates
		unsigned int timesteps = std::floor(elapsedTime / m_fixedUpdateMillis);

		// store time we couldn't use for the next frame.
		m_leftOverTime = elapsedTime - timesteps * m_fixedUpdateMillis;

		// update with a fixed timestep many times
		for (unsigned int i = 0; i < timesteps; i++) {
			updateStep(m_fixedUpdateMillis);
		}
	}
}

void ofApp::updateStep(float dt)
{
	m_view->update(dt);
}


//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(0);
	m_cam.begin();
		ofDrawAxis(10);
		m_view->render(m_viewRenderSettings);
		m_cam.end();

	ofDrawBitmapStringHighlight(m_helpInfo, glm::vec3(50, 50, 0));
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == ' ')
	{
		m_view = make_unique<PatternView>(m_ami->getPattern(), *m_viewParams);
	}
	if (key == 'l' || key == 'L')
	{
		auto res = ofSystemLoadDialog("Load pattern", false, "data");
		if (res.bSuccess)
		{
			m_filepath = res.filePath;
			try
			{
				m_ami = make_unique<Amigurumi>(m_filepath);
				m_viewParams = make_unique<PatternView::Params>(m_filepath);
				m_view = make_unique<PatternView>(m_ami->getPattern(), *m_viewParams);
			}
			catch (std::invalid_argument & e)
			{
				ofLogError("ofApp") << "Pattern graph failed: " << e.what();
				ofExit();
			}
		}
	}
	if (key == 'p' || key == 'P')
	{
		m_bRun = !m_bRun;
	}

	if (key == 's' || key == 'S')
	{
		this->updateStep(m_fixedUpdateMillis);
	}

	if (key == 'd' || key == 'D')
	{
		m_viewRenderSettings.debug = !m_viewRenderSettings.debug;
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
