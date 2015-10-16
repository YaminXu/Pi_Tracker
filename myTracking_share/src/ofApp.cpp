//--------------------------------------------------------------------------------
//  The idea comes from CMUcam, which is a small size video tracking system.
//  My goal is to make a simple and flexible video tracking system.
//  I believe It could be powerful and funny because of openFrameworks and RaspberryPI.
//  
//  The video is here: https://www.youtube.com/watch?v=tXOxcmbyjSg
//  
//  Yamin Xu  
//  2015/7/26 
//--------------------------------------------------------------------------------


#include "ofApp.h"
#include <opencv2/opencv.hpp>
#include "ofxGPIO.h"


I2CBus * bus;

GPIO* gpio17;
GPIO* gpio18;
string state_button0;
string state_button1;
string state_button2;
uint8_t state = 0;

//using namespace ofxCv;//rpi
using namespace cv;

Mat frame, frameProcessed;

int w = 320; int h = 240;
//--------------------------------------------------------------
void testApp::setup() {

  ofBackgroundHex(0x000000);
  bPressedInside = false;

  colorImage.allocate(w, h);//
  grayImage.allocate(w, h);//

  cam.setup(w, h, false); //setup camera (w,h,color = true,gray = false);//rpi

  confidenceMin = 80;
  cropRect.set(120, 90, 80, 60);

  bus = new I2CBus("/dev/i2c-1");
  bus->addressSet(0x04);


  gpio17  = new GPIO("17");
  gpio18  = new GPIO("18");


  gpio17->export_gpio();
  gpio18->export_gpio();


  gpio17->setdir_gpio("in");
  gpio18->setdir_gpio("in");

  usleep(200);
}

//--------------------------------------------------------------
void testApp::update() {

  frame = cam.grab();//rpi


  if (!frame.empty()) {

    grayImage.setFromPixels((unsigned char *) IplImage(frame).imageData, w, h);


    // clear out all the old matches
    matches.clear();

    // look for any of the objects in the source image
    for (int i = 0; i < objects.size(); i++) {

      int nFound = matcher.findMatches(grayImage, objects[i], confidenceMin);

      // copy over the matches and store the object id
      for (int j = 0; j < matcher.matches.size(); j++) {
        matches.push_back(matcher.matches[j]);
        matches.back().objectID = i;
      }
    }

    if (matcher.matches.size())
    {

      cropRect.set(matcher.matches[0].x, matcher.matches[0].y, matcher.matches[0].width, matcher.matches[0].height);
      objects.back().allocate(cropRect.width, cropRect.height);
      grayImage.setROI(cropRect.x, cropRect.y, cropRect.width, cropRect.height);
      objects.back().setFromPixels(grayImage.getRoiPixels(), cropRect.width, cropRect.height);
      grayImage.resetROI();
    }
  }

  //printf("Tracking Object X: %i; Y: %i\n", (int)cropRect.x, (int)cropRect.y);


  bus->writeByte(0x04, 255);
  //usleep(20);
  bus->writeByte(0x04, (uint8_t)(cropRect.x * 0.75));
  //usleep(20);
  bus->writeByte(0x04, (uint8_t)(cropRect.y * 0.75));
  //usleep(20);
  bus->writeByte(0x04, state);

  gpio18->getval_gpio(state_button2);
  if (state_button2 == "1") {

    cropRect.set(120, 90, 80, 60);
    objects.clear();
    objects.push_back(MatchObject());
    objects.back().allocate(cropRect.width, cropRect.height);
    grayImage.setROI(cropRect.x, cropRect.y, cropRect.width, cropRect.height);
    objects.back().setFromPixels(grayImage.getRoiPixels(), cropRect.width, cropRect.height);
    grayImage.resetROI();

    printf("state_button2\n");

  }
  gpio17->getval_gpio(state_button1);
  if (state_button1 == "1") {

    matcher.matches.clear();
    objects.clear();

    printf("state_button1\n");

  }

  if (state_button0 == "1") {

    printf("state_button0\n");
  }

}

void testApp::draw() {
  ofSetColor(255);
  grayImage.draw(10, 10);
  ofNoFill();

  if (objects.size())
  {
    if (matches.size())
    {
      ofSetHexColor(0x00FF00);
      ofRect(cropRect);
      state = 1;
    }
    else
    {
      ofSetHexColor(0xFF0000);
      ofRect(cropRect);
      state = 2;
    }

    ofPushMatrix();
    ofTranslate(w + 30, 10);
    ofSetColor(255);
    objects[0].draw(0, 0);

    ofPopMatrix();

  }

  else
  {
    ofSetHexColor(0x0000FF);
    ofRect(cropRect);
    state = 0;
  }

  ofLine(160, 115, 160, 125);
  ofLine(155, 120, 165, 120);
  ofSetHexColor(0xF5ECCF);
  ofDrawBitmapString(("Tracking Object X:" + ofToString((int)cropRect.x) + "; Y:" + ofToString((int)cropRect.y)), 10, h + 30);
  //ofDrawBitmapString("Draw a bounding box around object\nPress spacebar to capture object\nUp/Down to change confidence threshold\nFPS: "+ofToString(ofGetFrameRate(),0)+"\nConfidence Min:"+ofToString(confidenceMin)+"%", 10, h+30);
  //ofDrawBitmapString("Green->tracking object; Blue->ready to select; Red->lost target\nspacebar->start to capture object\nUp/Down to change confidence threshold\nFPS: "+ofToString(ofGetFrameRate(),0)+"\nConfidence Min:"+ofToString(confidenceMin)+"%", 10, h+70);
  ofDrawBitmapString("Green->tracking object; Blue->ready to select; Red->lost target\nspacebar->start to capture object\nFPS: " + ofToString(ofGetFrameRate(), 0), 10, h + 70);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key) {

  if (key == ' ') {
    objects.clear();
    objects.push_back(MatchObject());
    objects.back().allocate(cropRect.width, cropRect.height);
    grayImage.setROI(cropRect.x, cropRect.y, cropRect.width, cropRect.height);
    objects.back().setFromPixels(grayImage.getRoiPixels(), cropRect.width, cropRect.height);
    grayImage.resetROI();
  }
  if (key == 'c') {
    matcher.matches.clear();
    objects.clear();
  }
  if (key == OF_KEY_UP) {
    if (confidenceMin < 100) confidenceMin ++;
  }
  if (key == OF_KEY_DOWN) {
    if (confidenceMin > 0) confidenceMin --;
  }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ) {

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button) {
  if (bPressedInside) {
    if (x > downPos.x + 5 && y > downPos.y + 5) {
      cropRect.width  = x - downPos.x;
      cropRect.height = y - downPos.y;
      if (cropRect.height + cropRect.y > h) cropRect.height = (h - cropRect.y) + 10;
      if (cropRect.width + cropRect.x > w) cropRect.width = (w - cropRect.x) + 10;
    }
  }
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button) {
  if (ofRectangle(10, 10, w, h).inside(x, y)) {
    cropRect.setPosition(x, y);
    if (cropRect.width + cropRect.x > w) cropRect.width = (w - x) + 10;
    if (cropRect.height + cropRect.y > h) cropRect.height = (h - y) + 10;
    downPos.set(x, y);
    bPressedInside = true;
  }
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button) {
  bPressedInside = false;
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h) {

}
//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo) {

}
