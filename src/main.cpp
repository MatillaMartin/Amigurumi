#include "ofMain.h"
#include "ofApp.h"
#include "cxxopts.hpp"

//========================================================================
int main(int argc, char *argv[]) {
	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:

	ofApp::Settings settings;
	settings.step = false;

	try
	{
		cxxopts::Options options(argv[0], " - amigurumi simulator");
		options
			.positional_help("[optional args]")
			.show_positional_help();

		options
			.allow_unrecognised_options()
			.add_options()
			("s,step", "Add points one by one and build amigurumi step by step", cxxopts::value<bool>(settings.step))
			;

		auto result = options.parse(argc, argv);

		ofRunApp(new ofApp(settings));
	}
	catch (const cxxopts::OptionException& e)
	{
		std::cout << "error parsing options: " << e.what() << std::endl;
		exit(1);
	}
}
