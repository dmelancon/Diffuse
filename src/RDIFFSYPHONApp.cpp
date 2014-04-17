#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class RDIFFSYPHONApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void RDIFFSYPHONApp::setup()
{
}

void RDIFFSYPHONApp::mouseDown( MouseEvent event )
{
}

void RDIFFSYPHONApp::update()
{
}

void RDIFFSYPHONApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( RDIFFSYPHONApp, RendererGl )
