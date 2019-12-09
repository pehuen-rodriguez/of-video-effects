#include "ofApp.h"

// COMMON ///////////////
void ofApp::setup()
{
  ofDisableArbTex();
  ofEnableSmoothing();
  ofEnableAlphaBlending();
  ofSetVerticalSync(true);

  camWidth = 1280;
  camHeight = 720;

  camDrawWidth = ofGetWidth();
  videoAspectRatio = camDrawWidth / camWidth;
  camDrawHeight = camHeight * videoAspectRatio;

  yCamDrawOffset = (camHeight - camDrawHeight) / 2;

  vidGrabber.setDeviceID(1);
  vidGrabber.setDesiredFrameRate(30);
  vidGrabber.initGrabber(camWidth, camHeight);

  setupProcessingChain();
  setupFilteringChain();

  receiver.setup(17024);

  address = "no_address";
  value = 0.0;
}
void ofApp::update()
{
  vidGrabber.update();
  checkMessages();
  if (!filterReseted)
  {
    resetFilter();
  }
}
void ofApp::draw()
{
  post.begin();
    filters[currentFilter]->begin();
      vidGrabber.draw(0, yCamDrawOffset, camDrawWidth, camDrawHeight);
    filters[currentFilter]->end();
  post.end();

  ofDrawBitmapString("For address: " + address, 100, 100);
  ofDrawBitmapString("Value was: " + ofToString(value), 100, 110);
}
void ofApp::keyPressed(int key)
{
  // 0-8 ////////////////////
  unsigned idx = key - '0';
  if (idx < post.size()) 
  {
    post[idx]->setEnabled(!post[idx]->getEnabled());
  }

  // ADVANCE FILTER AND FS //
  if (key == ' ')
  {
    advanceFilter();
  }
  if (key == 'f')
  {
    ofSetFullscreen(true);
  }

  // TOGGLE FXS /////////////
  if (key == 'a')
    myGlitch.toggleFx(OFXPOSTGLITCH_CONVERGENCE);
  if (key == 's')
    myGlitch.toggleFx(OFXPOSTGLITCH_GLOW);
  if (key == 'd')
    myGlitch.toggleFx(OFXPOSTGLITCH_SHAKER);
  if (key == 'g')
    myGlitch.toggleFx(OFXPOSTGLITCH_TWIST);
  if (key == 'h')
    myGlitch.toggleFx(OFXPOSTGLITCH_OUTLINE);
  if (key == 'j')
    myGlitch.toggleFx(OFXPOSTGLITCH_NOISE);
  if (key == 'k')
    myGlitch.toggleFx(OFXPOSTGLITCH_SLITSCAN);
  if (key == 'l')
    myGlitch.toggleFx(OFXPOSTGLITCH_SWELL);
  if (key == ',')
    myGlitch.toggleFx(OFXPOSTGLITCH_INVERT);

  if (key == 'q')
    myGlitch.toggleFx(OFXPOSTGLITCH_CR_HIGHCONTRAST);
  if (key == 'w')
    myGlitch.toggleFx(OFXPOSTGLITCH_CR_BLUERAISE);
  if (key == 'e')
    myGlitch.toggleFx(OFXPOSTGLITCH_CR_REDRAISE);
  if (key == 'r')
    myGlitch.toggleFx(OFXPOSTGLITCH_CR_GREENRAISE);
  if (key == 't')
    myGlitch.toggleFx(OFXPOSTGLITCH_CR_BLUEINVERT);
  if (key == 'y')
    myGlitch.toggleFx(OFXPOSTGLITCH_CR_REDINVERT);
  if (key == 'u')
    myGlitch.toggleFx(OFXPOSTGLITCH_CR_GREENINVERT);
}

// PROCESSES ////////////
void ofApp::setupProcessingChain()
{
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

// FILTERS //////////////
void ofApp::setupFilteringChain()
{
  currentFilter = 0;
  filterReseted = true;

  filters.push_back(new SketchFilter(camDrawWidth, camDrawHeight));
  filters.push_back(new LaplacianFilter(camDrawWidth, camDrawHeight, ofVec2f(1, 1)));

  Abstract3x3ConvolutionFilter *convolutionFilter1 = new Abstract3x3ConvolutionFilter(camDrawWidth, camDrawHeight);
  convolutionFilter1->setMatrix(-1, 0, 1, -2, 0, 2, -1, 0, 1);
  filters.push_back(convolutionFilter1);

  Abstract3x3ConvolutionFilter *convolutionFilter2 = new Abstract3x3ConvolutionFilter(camDrawWidth, camDrawHeight);
  convolutionFilter2->setMatrix(4, 4, 4, 4, -32, 4, 4, 4, 4);
  filters.push_back(convolutionFilter2);

  Abstract3x3ConvolutionFilter *convolutionFilter3 = new Abstract3x3ConvolutionFilter(camDrawWidth, camDrawHeight);
  convolutionFilter3->setMatrix(1.2, 1.2, 1.2, 1.2, -9.0, 1.2, 1.2, 1.2, 1.2);
  filters.push_back(convolutionFilter3);

  vector<GradientMapColorPoint> colors;
  for (float percent = 0.0; percent <= 1.0; percent += 0.1)
  {
    colors.push_back(GradientMapColorPoint(ofRandomuf(), 0.0, 0.0, percent));
  }
  filters.push_back(new GradientMapFilter(colors));
}
void ofApp::advanceFilter()
{
  currentFilter++;
  if (currentFilter >= filters.size())
  {
    currentFilter = 0;
  }
}
void ofApp::resetFilter()
{
  // ofLogNotice() << "sliderX: " << sliderX << " - X: " << sliderX * camDrawWidth;
  // ofLogNotice() << "sliderY: " << sliderY << " - Y: " << sliderY * camDrawHeight;
  switch (currentFilter)
  {
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
    for (float percent = 0.0; percent <= 1.0; percent += 0.1)
    {
      colors.push_back(GradientMapColorPoint(ofRandomuf(), 0.0, 0.0, percent));
    }
    filters[currentFilter] = new GradientMapFilter(colors);
    break;
  }
  filterReseted = true;
}

// OSC //////////////////
void ofApp::checkMessages()
{
  ofxOscMessage msg;
  while (receiver.hasWaitingMessages())
  {
    receiver.getNextMessage(msg);

    // effects_on/x
    // record_on/x
    // sliderX/x /y
    // sliderY/x /y
    // sets/x
    // effects/x
    // da_ring/x /y /z
    address = msg.getAddress();
    value = msg.getArgAsFloat(0);

    if (msg.getAddress() == "/sliderX/x")
    {
      sliderX = msg.getArgAsFloat(0) / 4;
      filterReseted = false;
    }
    if (msg.getAddress() == "/sliderY/x")
    {
      sliderY = msg.getArgAsFloat(0) / 4;
      filterReseted = false;
    }
  }
}