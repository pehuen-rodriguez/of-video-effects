#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  ofDisableArbTex();
  ofEnableSmoothing();
  ofEnableAlphaBlending();

  camWidth = 1280;
  camHeight = 720;

  camDrawWidth = ofGetWidth();
  videoAspectRatio = camDrawWidth / camWidth;
  camDrawHeight = camHeight * videoAspectRatio;

  yCamDrawOffset = (camHeight - camDrawHeight) / 2;

  vidGrabber.setDeviceID(1);
  vidGrabber.setDesiredFrameRate(30);
  vidGrabber.initGrabber(camWidth, camHeight);

  // glitchFbo.allocate(camDrawWidth, camDrawHeight);
  // myFbo.allocate(camDrawWidth, camDrawHeight);
  // myGlitch.setup(&myFbo);

  setupProcessingChain();
  setupFilteringChain();
  ofSetVerticalSync(true);

  receiver.setup(17024);
}

//--------------------------------------------------------------
void ofApp::update() {
  // myFbo.begin();
  //   vidGrabber.draw(xCamDrawOffset, 0, camDrawWidth, camDrawHeight);
  // myFbo.end();

  vidGrabber.update();
  checkMessages();
  if (!filterReseted) {
    resetFilter();
  }
}

//--------------------------------------------------------------
void ofApp::draw() {
  post.begin();
    filters[currentFilter]->begin();
      vidGrabber.draw(0, yCamDrawOffset, camDrawWidth, camDrawHeight);
    filters[currentFilter]->end();
  post.end();
  // ofSetColor(255);
  // ofDrawBitmapString("Filtro: " + filters[currentFilter]->getName(), ofPoint(40, 20));
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
  unsigned idx = key - '0';
  if (idx < post.size()) {
    post[idx]->setEnabled(!post[idx]->getEnabled());
  }

  if (key==' ') {
    advanceFilter();
  }

  if (key == 'a') myGlitch.toggleFx(OFXPOSTGLITCH_CONVERGENCE);
  if (key == 's') myGlitch.toggleFx(OFXPOSTGLITCH_GLOW);
  if (key == 'd') myGlitch.toggleFx(OFXPOSTGLITCH_SHAKER);
  if (key == 'f') {
    ofSetFullscreen(true);
  }
  if (key == 'g') myGlitch.toggleFx(OFXPOSTGLITCH_TWIST);
  if (key == 'h') myGlitch.toggleFx(OFXPOSTGLITCH_OUTLINE);
  if (key == 'j') myGlitch.toggleFx(OFXPOSTGLITCH_NOISE);
  if (key == 'k') myGlitch.toggleFx(OFXPOSTGLITCH_SLITSCAN);
  if (key == 'l') myGlitch.toggleFx(OFXPOSTGLITCH_SWELL);
  if (key == ',') myGlitch.toggleFx(OFXPOSTGLITCH_INVERT);

  if (key == 'q') myGlitch.toggleFx(OFXPOSTGLITCH_CR_HIGHCONTRAST);
  if (key == 'w') myGlitch.toggleFx(OFXPOSTGLITCH_CR_BLUERAISE);
  if (key == 'e') myGlitch.toggleFx(OFXPOSTGLITCH_CR_REDRAISE);
  if (key == 'r') myGlitch.toggleFx(OFXPOSTGLITCH_CR_GREENRAISE);
  if (key == 't') myGlitch.toggleFx(OFXPOSTGLITCH_CR_BLUEINVERT);
  if (key == 'y') myGlitch.toggleFx(OFXPOSTGLITCH_CR_REDINVERT);
  if (key == 'u') myGlitch.toggleFx(OFXPOSTGLITCH_CR_GREENINVERT);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
}

void ofApp::setupProcessingChain() {
  post.init(ofGetWidth(), ofGetHeight());
  post.createPass<FxaaPass>()->setEnabled(false);
  post.createPass<BloomPass>()->setEnabled(false);
  post.createPass<DofPass>()->setEnabled(false);
  post.createPass<KaleidoscopePass>()->setEnabled(false);
  post.createPass<NoiseWarpPass>()->setEnabled(false);
  post.createPass<PixelatePass>()->setEnabled(false);
  post.createPass<EdgePass>()->setEnabled(false);
  post.createPass<VerticalTiltShifPass>()->setEnabled(false);
  post.createPass<GodRaysPass>()->setEnabled(false);
}

void ofApp::setupFilteringChain() {
  currentFilter = 0;
  filterReseted = true;

  filters.push_back(new SketchFilter(camDrawWidth, camDrawHeight));
  filters.push_back(new LaplacianFilter(camDrawWidth, camDrawHeight, ofVec2f(1, 1)));

  Abstract3x3ConvolutionFilter * convolutionFilter1 = new Abstract3x3ConvolutionFilter(camDrawWidth, camDrawHeight);
  convolutionFilter1->setMatrix(-1, 0, 1, -2, 0, 2, -1, 0, 1);
  filters.push_back(convolutionFilter1);

  Abstract3x3ConvolutionFilter * convolutionFilter2 = new Abstract3x3ConvolutionFilter(camDrawWidth, camDrawHeight);
  convolutionFilter2->setMatrix(4, 4, 4, 4, -32, 4, 4,  4, 4);
  filters.push_back(convolutionFilter2);

  Abstract3x3ConvolutionFilter * convolutionFilter3 = new Abstract3x3ConvolutionFilter(camDrawWidth, camDrawHeight);
  convolutionFilter3->setMatrix(1.2,  1.2, 1.2, 1.2, -9.0, 1.2, 1.2,  1.2, 1.2);
  filters.push_back(convolutionFilter3);

  vector<GradientMapColorPoint> colors;
  for (float percent=0.0; percent<=1.0; percent+= 0.1) {
    colors.push_back(GradientMapColorPoint(ofRandomuf(), ofRandomuf(), ofRandomuf(), percent));
  }
  filters.push_back(new GradientMapFilter(colors));
}

void ofApp::advanceFilter() {
  currentFilter++;
  if (currentFilter >= filters.size()) {
    currentFilter = 0;
  }
}

void ofApp::checkMessages() {
  while (receiver.hasWaitingMessages()) {
    ofxOscMessage msg;
    receiver.getNextMessage(msg);

    if (msg.getAddress() == "/convergence") {
      post[1]->setEnabled(!post[1]->getEnabled());
    }
    if (msg.getAddress() == "/shaker") {
      post[2]->setEnabled(!post[2]->getEnabled());
    }
    if (msg.getAddress() == "/cutslider") {
      post[3]->setEnabled(!post[3]->getEnabled());
    }
    if (msg.getAddress() == "/twist") {
      post[4]->setEnabled(!post[4]->getEnabled());
    }
    if (msg.getAddress() == "/sliderX") {
      sliderX = msg.getArgAsFloat(0) / 4;
      filterReseted = false;
    }
    if (msg.getAddress() == "/sliderY") {
      sliderY = msg.getArgAsFloat(0) / 4;
      filterReseted = false;
    }
    if (msg.getAddress() == "/play") {
      advanceFilter();
    }
    if (msg.getAddress() == "/set0") {
      currentFilter = 0;
      filterReseted = false;
    }
    if (msg.getAddress() == "/set1") {
      currentFilter = 1;
      filterReseted = false;
    }
    if (msg.getAddress() == "/set2") {
      currentFilter = 2;
      filterReseted = false;
    }
  }
}

void ofApp::resetFilter() {
  // ofLogNotice() << "sliderX: " << sliderX << " - X: " << sliderX * camDrawWidth;
  // ofLogNotice() << "sliderY: " << sliderY << " - Y: " << sliderY * camDrawHeight;
  switch (currentFilter) {
    case 0:
      filters[currentFilter] = new SketchFilter(sliderX * camDrawWidth, sliderY * camDrawHeight);
      break;
    case 1:
      filters[currentFilter] = new LaplacianFilter(sliderX * camDrawWidth, sliderY * camDrawHeight, ofVec2f(1, 1));
      break;
    case 2:
      convolutionFilter1 = new Abstract3x3ConvolutionFilter(sliderX * camDrawWidth, sliderY * camDrawHeight);
      convolutionFilter1->setMatrix(-1, 0, 1, -2, 0, 2, -1, 0, 1);
      filters[currentFilter] = convolutionFilter1;
      break;
    case 3:
      convolutionFilter2 = new Abstract3x3ConvolutionFilter(sliderX * camDrawWidth, sliderY * camDrawHeight);
      convolutionFilter2->setMatrix(4, 4, 4, 4, -32, 4, 4, 4, 4);
      filters[currentFilter] = convolutionFilter2;
      break;
    case 4:
      convolutionFilter3 = new Abstract3x3ConvolutionFilter(sliderX * camDrawWidth, sliderY * camDrawHeight);
      convolutionFilter3->setMatrix(1.2, 1.2, 1.2, 1.2, -9.0, 1.2, 1.2, 1.2, 1.2);
      filters[currentFilter] = convolutionFilter3;
      break;
    case 5:
      vector<GradientMapColorPoint> colors;
      for (float percent=0.0; percent<=1.0; percent+= 0.1) {
        colors.push_back(GradientMapColorPoint(ofRandomuf(), ofRandomuf(), ofRandomuf(), percent));
      }
      filters[currentFilter] = new GradientMapFilter(colors);
      break;
  }
  filterReseted = true;
}