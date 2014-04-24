#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"
#include "cinder/gl/Vbo.h"
#include "Resources.h"
#include "cinderSyphon.h"

#define SIZE 700
using namespace ci;
using namespace ci::app;
using namespace std;


class RDIFFSYPHONApp : public AppNative {
public:
    void	prepareSettings( Settings *settings );
	void	setup();
	void	update();
	void	draw();
	void	keyDown( KeyEvent event );
    void	keyUp( KeyEvent event );
	void	mouseMove( MouseEvent event );
	void	mouseDown( MouseEvent event );
	void	mouseDrag( MouseEvent event );
	void	mouseUp( MouseEvent event );
	void	resetFBOs();
    void    createVBOMesh();
    params::InterfaceGlRef	mParams;
    
	int				mCurrentFBO;
	int				mOtherFBO;
	gl::Fbo			mFBOs[2];
	gl::GlslProgRef	mShader;
    gl::GlslProgRef	mShaderRefraction;
    const Vec2i	kWindowSize	= Vec2i( SIZE, SIZE );
    
	gl::Texture		mTexture;
    
	gl::VboMeshRef mWrapper;
	Vec2f			mMouse;
	bool			mMousePressed;
    bool            keyPressed;
	bool            textFeed = false;
	float			mReactionU;
	float			mReactionV;
	float			mReactionK;
	float			mReactionF;
    float			mScale;
    float           mTilt;
    int             mSize;
    int             mSpeed;
    int				mCurrentFrame;

    float          mColorValR;
    float          mColorValG;
    float          mColorValB;
    float          mColorHeight;
	static const int		FBO_WIDTH = SIZE, FBO_HEIGHT = SIZE;
    
    CameraPersp mCam;
    syphonServer mScreenSyphon; //each item to publish requires a different server
	syphonServer mTextureSyphon;
	
	syphonClient mClientSyphon; //our syphon client
    
};
void RDIFFSYPHONApp::prepareSettings(Settings *settings)
{
    //        settings->enableHighDensityDisplay();
    //        settings->enableMultiTouch( false );
    
    settings->setWindowSize(SIZE,SIZE);
    settings->setFrameRate( 60.0f );
    
}

void RDIFFSYPHONApp::mouseDown( MouseEvent event )
{
    mMousePressed = true;
}

void RDIFFSYPHONApp::mouseUp(MouseEvent event)
{
    mMousePressed = false;
}


void RDIFFSYPHONApp::mouseMove( MouseEvent event )
{
	mMouse = event.getPos();
}

void RDIFFSYPHONApp::mouseDrag( MouseEvent event )
{
	mMouse = event.getPos();
}
void RDIFFSYPHONApp::createVBOMesh(){
    int trisize =1;
    int  NUMX = SIZE/trisize;
    int NUMY = SIZE/trisize;
    int numVertices = NUMX*NUMY;
    int totalTriangles = ( NUMX - 1 ) * ( NUMY - 1 );
    
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticColorsRGBA();
    layout.setStaticPositions();
    layout.setStaticTexCoords2d();
    layout.setStaticNormals();
    
    vector<Vec3f> positions;
    vector<Vec3f> normals;
    vector<Vec2f> texCoords;
    vector<uint32_t>   indices;
    vector<ColorA> colors;
    
    mWrapper = gl::VboMesh::create(numVertices, totalTriangles * 6, layout, GL_TRIANGLES);
    for(int x=0;x<NUMX;x+=trisize){
        for(int y=0;y<NUMY;y+=trisize){
            positions.push_back( Vec3f(x,y,0. ) );
            if( ( x + trisize < NUMX ) && ( y + trisize < NUMY) ) {
                indices.push_back( (x+0) * NUMY + (y+0) );
                indices.push_back( (x+0) * NUMY + (y+trisize) );
                indices.push_back( (x+trisize) * NUMY + (y+trisize) );
                indices.push_back( (x+0) * NUMY + (y+0) );
                indices.push_back( (x+trisize) * NUMY + (y+trisize) );
                indices.push_back( (x+trisize) * NUMY + (y+0) );
            }
            
            Vec3f normal = cross(Vec3f(x, y+trisize,0.f)-Vec3f(x, y,0.f), Vec3f(x+trisize, y,0.f)-Vec3f(x, y,0.f)).normalized();
            normals.push_back(normal);
            texCoords.push_back( Vec2f( (x / (float)NUMX), (y / (float)NUMY) ) );
            colors.push_back(ColorA(1.,1.,1.,.8));
        }
        
    }
    mWrapper->bufferIndices(indices);
    mWrapper->bufferPositions(positions);
    mWrapper->bufferColorsRGBA(colors);
    mWrapper->bufferTexCoords2d(0, texCoords);
    mWrapper->bufferNormals(normals);
    mWrapper->unbindBuffers();
    //    indices.clear();
    //    texCoords.clear();
    //    positions.clear();
}

void RDIFFSYPHONApp::setup()
{
    mScreenSyphon.setName("Screen Output"); // set a name for each item to be published
	mTextureSyphon.setName("Texture Output");
	
	mClientSyphon.setup();
    
	// in order for this to work, you must run simple server which is a syphon test application
    // feel free to change the app and server name for your specific case
    mClientSyphon.set("", "Simple Server");
    
    mClientSyphon.bind();

    mSpeed = 3;
    mTilt = -300;
    mReactionU = 0.28f;
	mReactionV = 0.03f;
	mReactionK = 0.048f;
	mReactionF = 0.072f;
    mScale =39.0;
    mSize = 3;
    mColorValR = .6;
    mColorValG = .0;
    mColorValB = .8;
    mColorHeight = -15.;
	mMousePressed = false;
    mParams = params::InterfaceGl::create( "Parameters", Vec2i( 175, 300 ) );
	mParams->addParam( "Reaction u", &mReactionU, "min=0.0 max=0.4 step=0.01 keyIncr=u keyDecr=U" );
	mParams->addParam( "Reaction v", &mReactionV, "min=0.0 max=0.4 step=0.01 keyIncr=v keyDecr=V" );
	mParams->addParam( "Reaction k", &mReactionK, "min=0.0 max=1.0 step=0.001 keyIncr=k keyDecr=K" );
	mParams->addParam( "Reaction f", &mReactionF, "min=0.0 max=1.0 step=0.001 keyIncr=f keyDecr=F" );
	mParams->addParam( "Scale", &mScale, "min=0.0 max=300.0 step=0.5 keyIncr=f keyDecr=F" );
    mParams->addParam( "speed", &mSpeed, "min = 1 max= 25 step=1 ");
    mParams->addParam( "Tilt", &mTilt, "min=-1000. max=1000. step=1.0 keyIncr=f keyDecr=F" );
    mParams->addParam( "Size", &mSize, "min=1. max=16. step=1.0 keyIncr=f keyDecr=F" );
    mParams->addParam( "R", &mColorValR, "min=.0 max=10. step=.01 keyIncr=f keyDecr=F" );
    mParams->addParam( "G", &mColorValG, "min=.0 max=1. step=.01 keyIncr=f keyDecr=F" );
    mParams->addParam( "B", &mColorValB, "min=.0 max=1. step=.01 keyIncr=f keyDecr=F" );
    mParams->addParam( "ColorHeight", &mColorHeight, "min=-100.0 max=.0 step=.5 keyIncr=f keyDecr=F" );

    gl::Fbo::Format format;
	format.enableDepthBuffer( false );
    format.setColorInternalFormat(GL_RGBA32F_ARB);
    mCurrentFBO =0;
	mOtherFBO = 1;
	mFBOs[0] = gl::Fbo( FBO_WIDTH, FBO_HEIGHT, format );
	mFBOs[1] = gl::Fbo( FBO_WIDTH, FBO_HEIGHT, format );
	
    mShaderRefraction = gl::GlslProg::create( loadResource( RES_REFRACTION_VERT  ), loadResource( PASS_FRAG   ) );
	mShader = gl::GlslProg::create( loadResource( RES_PASS_THRU_VERT ), loadResource( RES_GSRD_FRAG ) );
    
    createVBOMesh();
    
    mFBOs[0].bindFramebuffer();
    gl::clear();
    
    mFBOs[0].unbindFramebuffer();
    
	resetFBOs();
    
    //create Texture
    mTexture = gl::Texture( loadImage( loadResource( RES_STARTER_IMAGE ) ) );
    mTexture.setWrap( GL_REPEAT, GL_REPEAT );
    mTexture.setMinFilter( GL_LINEAR );
    mTexture.setMagFilter( GL_LINEAR );
    
    mCam.setPerspective(60, getWindowAspectRatio(), 1, 10000);
    mCam.lookAt(Vec3f(0,mTilt,0),Vec3f(0,0,0),Vec3f::yAxis());
    
}

void RDIFFSYPHONApp::update(){
    mCam.lookAt(Vec3f(0,mTilt,700),Vec3f(0,0,0),Vec3f::yAxis());
    
    //FBO PING PONGING
    const int ITERATIONS = mSpeed;
    gl::setMatricesWindow( mFBOs[0].getSize(), false );
	gl::setViewport( mFBOs[0].getBounds() );
	for( int i = 0; i < ITERATIONS; i++ ) {
		mCurrentFBO = ( mCurrentFBO + 1 ) % 2;
		mOtherFBO   = ( mCurrentFBO + 1 ) % 2;
		
		mFBOs[ mCurrentFBO ].bindFramebuffer();
        if (textFeed == false){
            mTexture.bind(0);
            textFeed = true;
        }else{
            mFBOs[ mOtherFBO ].bindTexture();
        }
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        
		mShader->bind();
		mShader->uniform( "texture", 0 );
		mShader->uniform( "srcTexture", 1 );
		mShader->uniform( "width", (float)FBO_WIDTH );
		mShader->uniform( "ru", mReactionU );
		mShader->uniform( "rv", mReactionV );
		mShader->uniform( "k", mReactionK );
		mShader->uniform( "f", mReactionF );
        mShader->uniform( "f", mReactionF );
        //mShader->uniform( "scale", mScale );
        
		gl::drawSolidRect( mFBOs[ mCurrentFBO ].getBounds() );
        
		mShader->unbind();
        
        
        
		if( mMousePressed ){
			glColor4f( 1.0f, 1.0f, 1.0f, -.5f );
			RectMapping windowToFBO( getWindowBounds(), mFBOs[mCurrentFBO].getBounds() );
			gl::drawSolidCircle( windowToFBO.map( (mMouse/2) ), 5.0f, 32 );
		}
        
        mFBOs[ mCurrentFBO ].unbindFramebuffer();
        
        
        
	}
    if (keyPressed){
        writeImage( getHomeDirectory() / "cinder" / "saveImage_" / ( toString( mCurrentFrame ) + ".png" ), copyWindowSurface() );
        mCurrentFrame++;}
    
}

void RDIFFSYPHONApp::draw()
{


    gl::clear( ColorA( 0, 0, 0, 0 ) );
	gl::setMatrices( mCam );
	gl::setViewport( getWindowBounds() );
    
    
    gl::pushMatrices();
    gl::rotate(Vec3f(180,0,.0));
    gl::translate(Vec2f(-getWindowWidth()/2, -getWindowHeight()/2));
    glEnable (GL_BLEND);
    
    mFBOs[mCurrentFBO].bindTexture(0);
    mShaderRefraction->bind();
    mShaderRefraction->uniform("displacementMap", 0);
    mShaderRefraction->uniform("scale", mScale);
    mShaderRefraction->uniform("size", mSize);
    mShaderRefraction->uniform("colorValR", mColorValR);
    mShaderRefraction->uniform("colorValG", mColorValG);
    mShaderRefraction->uniform("colorValB", mColorValB);
    mShaderRefraction->uniform("colorHeight", mColorHeight);
//    mShaderRefraction->uniform("randomNumber", randFloat(-1.,1.));

    gl::draw(mWrapper);
    mShaderRefraction->unbind();
    mFBOs[mCurrentFBO].unbindTexture();
    gl::popMatrices();
    
    
    mParams->draw();
    //mClientSyphon.draw(Vec2f(16.f, 64.f)); //draw our client image
    
	mScreenSyphon.publishScreen(); //publish the screen's output
	//mTextureSyphon.publishTexture(mLogo); //publish our texture without shader
	
	//anything that we draw after here will not be published
   // gl::drawStringCentered("This text will not be published to Syphon.", Vec2f(getWindowCenter().x, 20.f), ColorA::black());
    
}

void RDIFFSYPHONApp::resetFBOs()
{
    // mTexture.bind( 0 );
	gl::setMatricesWindow( mFBOs[0].getSize(), false );
	gl::setViewport( mFBOs[0].getBounds() );
	for( int i = 0; i < 2; i++ ){
		mFBOs[i].bindFramebuffer();
        gl::clear();
        textFeed = true;
        
	}
    gl::Fbo::unbindFramebuffer();
    
}

void RDIFFSYPHONApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r' ) {
		resetFBOs();
	}
    if (event.getChar()== 'b'){
        keyPressed = true;
    }
}
void RDIFFSYPHONApp::keyUp( KeyEvent event ){
    if (event.getChar()== 'b'){
        keyPressed = false;
    }
    
    
}


CINDER_APP_NATIVE( RDIFFSYPHONApp, RendererGl )
