#pragma once

#include "ofMain.h"
#include "ofxPostProcessing.h"
#include "ofxPostGlitch.h"
#include "ofxFilterLibrary.h"
#include "ofxOsc.h"

class ofApp : public ofBaseApp{
  public:
    // COMMON ///////////////
    void setup();
    void update();
    void draw();
    void keyPressed(int key);

    // VIDEO ////////////////
    ofVideoGrabber vidGrabber;
    float camWidth;
    float camHeight;
    float camDrawWidth;
    float camDrawHeight;
    float videoAspectRatio;
    float xCamDrawOffset;
    float yCamDrawOffset;

    // PROCESSES ////////////
    void setupProcessingChain();
    ofxPostProcessing post;
    ofxPostGlitch	myGlitch;
    ofFbo myFbo;

    // FILTERS //////////////
    void setupFilteringChain();
    void advanceFilter();
    void resetFilter();
    vector<AbstractFilter *> filters;
    AbstractFilter * filter1;
    AbstractFilter * filter2;
    AbstractFilter * filter3;
    Abstract3x3ConvolutionFilter * convolutionFilter1;
    Abstract3x3ConvolutionFilter * convolutionFilter2;
    Abstract3x3ConvolutionFilter * convolutionFilter3;
    bool filterReseted;
    int currentFilter;

    // OSC //////////////////
    void checkMessages();
    ofxOscReceiver receiver;
    float sliderX;
    float sliderY;
    std::string address;
    float value;
};
