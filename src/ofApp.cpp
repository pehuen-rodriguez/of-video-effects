#include "ofApp.h"

// COMMON ///////////////
void ofApp::setup()
{
  deviceId = 0;
  ofDisableArbTex();
  ofEnableSmoothing();
  ofEnableAlphaBlending();

  camWidth = 1280;
  camHeight = 720;

  camDrawWidth = ofGetWidth();
  videoAspectRatio = camDrawWidth / camWidth;
  camDrawHeight = camHeight * videoAspectRatio;

  yCamDrawOffset = (camHeight - camDrawHeight) / 2;

  vidGrabber.setDeviceID(deviceId);
  vidGrabber.setDesiredFrameRate(30);
  vidGrabber.initGrabber(camWidth, camHeight);

  setupProcessingChain();
  setupFilteringChain();

  receiver.setup(17024);

  playOn = true;
  effectsOn = true;
  recordOn = true;
  ofSetVerticalSync(true);
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
  ofBackground(ofColor::black);

  if(playOn) {
    if(effectsOn) {
      post.begin();
        filters[currentFilter]->begin();
          vidGrabber.draw(0, yCamDrawOffset, camDrawWidth, camDrawHeight);
        filters[currentFilter]->end();
      post.end();
    } else {
      vidGrabber.draw(0, yCamDrawOffset, camDrawWidth, camDrawHeight);
    }
  }
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
    vidGrabber.close();
    if (deviceId == 1)
    {
      deviceId = 0;
    } else
    {
      deviceId = 1;
    }
    vidGrabber.setDeviceID(deviceId);
    vidGrabber.setDesiredFrameRate(30);
    vidGrabber.initGrabber(camWidth, camHeight);
  }
  if (key == 'f')
  {
    ofSetFullscreen(true);
  }

  // TOGGLE FXS /////////////
  if (key == 'a')
  {
    vidGrabber.close();
    vidGrabber.setDeviceID(0);
    vidGrabber.setDesiredFrameRate(30);
    vidGrabber.initGrabber(camWidth, camHeight);
  }
  if (key == 's')
  {
    vidGrabber.close();
    vidGrabber.setDeviceID(1);
    vidGrabber.setDesiredFrameRate(30);
    vidGrabber.initGrabber(camWidth, camHeight);
  }
  if (key == 'd')
  {
    vidGrabber.close();
    vidGrabber.setDeviceID(2);
    vidGrabber.setDesiredFrameRate(30);
    vidGrabber.initGrabber(camWidth, camHeight);
  }
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
    // play_on/x
    // switch_camera/x
    // effects_on/x
    // record_on/x
    // sliderX/x /y
    // sliderY/x /y
    // sets/x
    // effects/x
    // da_ring/x /y /z
    receiver.getNextMessage(msg);
    address = msg.getAddress();

    if (address == "/switch_camera/x") {
      deviceId = msg.getArgAsBool(0) ? 1 : 2;
      vidGrabber.close();
      vidGrabber.setDeviceID(deviceId);
      vidGrabber.setDesiredFrameRate(30);
      vidGrabber.initGrabber(camWidth, camHeight);
    } else if (address == "/play_on/x")
      playOn = msg.getArgAsBool(0);
    else if (address == "/effects_on/x")
      effectsOn = msg.getArgAsBool(0);
    else if (address == "/record_on/x")
      recordOn = msg.getArgAsBool(0);
    else if (address == "/sliderX/x" || address == "/da_ring/x") {
      sliderX = msg.getArgAsFloat(0);
      filterReseted = false;
    } else if (address == "/da_ring/y" || address == "/sliderY/x") {
      sliderY = msg.getArgAsFloat(0);
      filterReseted = false;
    } else if (address == "/sets/x") {
      for (int idx = 0; idx <= 5; idx += 1)
      {
        if (msg.getArgAsBool(idx))
        {
          currentFilter = idx;
          filterReseted = false;
          break;
        }
      }
    } else if (address == "/effects/x") {
      for (int idx = 0; idx <= 4; idx += 1)
      {
        post[idx+1]->setEnabled(msg.getArgAsBool(idx));
      }
    }
  }
}