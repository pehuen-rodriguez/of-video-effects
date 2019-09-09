#pragma once

#include "ofMain.h"
#include "ofxPostProcessing.h"
#include "ofxPostGlitch.h"
#include "ofxFilterLibrary.h"
#include "ofxOsc.h"

class ofApp : public ofBaseApp{
  public:
    void setup();
    void update();
    void draw();
    //////////////
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    //////////////
    ofVideoGrabber vidGrabber;
    float camWidth;
    float camHeight;
    float camDrawWidth;
    float camDrawHeight;
    float xCamDrawOffset;
    float yCamDrawOffset;
    float videoAspectRatio;
    //////////////
    void setupProcessingChain();
    ofxPostProcessing post;
    //////////////
    ofxPostGlitch	myGlitch;
    // ofFbo glitchFbo;
    ofFbo myFbo;
    //////////////
    void setupFilteringChain();
    int currentFilter;
    vector<AbstractFilter *> filters;
    AbstractFilter * filter1;
    AbstractFilter * filter2;
    AbstractFilter * filter3;
    //////////////
    float sliderX;
    float sliderY;
    //////////////
    ofxOscReceiver receiver;
    void advanceFilter();
    void checkMessages();
    void resetFilter();
    bool filterReseted;

    Abstract3x3ConvolutionFilter * convolutionFilter1;
    Abstract3x3ConvolutionFilter * convolutionFilter2;
    Abstract3x3ConvolutionFilter * convolutionFilter3;
};
