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
#include "Walker.h"
#include "triangleEdge.h"
#include "CinderFreenect.h"
#define SIZE 900
using namespace ci;
using namespace ci::app;
using namespace std;

// FIGURE OUT GSR_DFRAG SHADER ALPHA CHANNEL SHITTT

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
    void    loopEdges(int cur);
    void    keyPressed( KeyEvent event );
    void    testMode();
    params::InterfaceGlRef	mParams;
    KinectRef		mKinect;
	int				mCurrentFBO;
	int				mOtherFBO;
	gl::Fbo			mFBOs[2];
	gl::GlslProgRef	mShader;
    gl::GlslProgRef	mShaderRefraction;
    const Vec2i	kWindowSize	= Vec2i( SIZE, SIZE );
    Surface myPicture;
    Surface myPicture2;
    Surface mySurface;
	gl::Texture		mTexture;
    
	gl::VboMeshRef mWrapper;
    vector<uint16_t> datapoint;
    std::shared_ptr<uint16_t> depth;
    int			mMouseX = 0;
    int  mMouseY = 0;
    int x = 640;
    int y = 480;
    int cur;
    int mFarThresh,mCloseThresh;
    vector<int> counter;
    vector<int> totDepth;
    vector<float> aveDepth;
    float col;
	Vec2f			mMouse;
	bool			mMousePressed;
	bool            textFeed  = true;
    bool            textFeed2 = true;
    bool            textFeed3 = true;
    bool            loop = false;
	float			mReactionU;
	float			mReactionV;
	float			mReactionK;
	float			mReactionF;
    float			mScale;
    float           mTilt;
    int             mSize;
    int             mSpeed;
    int             rate;
    int				mCurrentFrame;
    WalkerRef w;
    WalkerRef w2;
    TriangleEdge t1;
    TriangleEdge t2;
    TriangleEdge t3;
    TriangleEdge t4;
    TriangleEdge t5;
    TriangleEdge t6;
    TriangleEdge t7;
    TriangleEdge t8;
    TriangleEdge t9;
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
    
    settings->setWindowSize(SIZE+640,SIZE);
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
    w = WalkerRef(new Walker());
    w2 = WalkerRef(new Walker());
	// in order for this to work, you must run simple server which is a syphon test application
    // feel free to change the app and server name for your specific case
    mClientSyphon.set("", "Simple Server");
    
    mClientSyphon.bind();

    mSpeed = 3;
    mTilt = 0;
    mReactionU = 0.28f;
	mReactionV = 0.03f;
	mReactionK = 0.048f;
	mReactionF = 0.072f;
    mScale =14.0;
    rate= 2;
    mSize = 2;
    mColorValR = 1.0;
    mColorValG = .0;
    mColorValB = .8;
    mColorHeight = -7.5;
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
    mParams->addParam( "rate", &rate, "min=0 max=50 step=1" );

    mParams->addParam( "R", &mColorValR, "min=.0 max=10. step=.01 keyIncr=f keyDecr=F" );
    mParams->addParam( "G", &mColorValG, "min=.0 max=1. step=.01 keyIncr=f keyDecr=F" );
    mParams->addParam( "B", &mColorValB, "min=.0 max=1. step=.01 keyIncr=f keyDecr=F" );
    mParams->addParam( "ColorHeight", &mColorHeight, "min=-100.0 max=.0 step=.5 keyIncr=f keyDecr=F" );
	mParams->addParam( "FarThreshold", &mFarThresh, "min=0 max=80000 step=100" );
	mParams->addParam( "CloseThreshold", &mCloseThresh, "min=0 max=80000 step=100" );
    mySurface = Surface( 640, 480,false);
    
    mKinect = Kinect::create();
    console() << "There are " << Kinect::getNumDevices() << " Kinects connected." << std::endl;
    mFarThresh = 21400;
    mCloseThresh = 80000;
    gl::Fbo::Format format;
	format.enableDepthBuffer( false );
    format.setColorInternalFormat(GL_RGBA32F_ARB);
    mCurrentFBO =0;
	mOtherFBO = 1;
	mFBOs[0] = gl::Fbo( FBO_WIDTH, FBO_HEIGHT, format );
	mFBOs[1] = gl::Fbo( FBO_WIDTH, FBO_HEIGHT, format );
    //gl::enableAlphaBlending();
    mShaderRefraction = gl::GlslProg::create( loadResource( RES_REFRACTION_VERT  ), loadResource( PASS_FRAG   ) );
	mShader = gl::GlslProg::create( loadResource( RES_PASS_THRU_VERT ), loadResource( RES_GSRD_FRAG ) );
    
    createVBOMesh();
    
    mFBOs[0].bindFramebuffer();
    gl::clear();
    
    mFBOs[0].unbindFramebuffer();
    resetFBOs();
    
    mCam.setPerspective(60, getWindowAspectRatio(), 1, 10000);
    mCam.lookAt(Vec3f(0,mTilt,0),Vec3f(0,0,0),Vec3f::yAxis());
//    myPicture = Surface(SIZE/mSize,SIZE/mSize, true);
//    myPicture = loadImage( loadResource( RES_STARTER_IMAGE ) );
//    myPicture2 = Surface(SIZE/mSize,SIZE/mSize, true);
//    myPicture2 = loadImage( loadResource( RES_STARTER_IMAGE2 ) );
    //mySurface = Surface( mFBOs[0].getBounds(), true);
  
    //mySurface.copyFrom(myPicture, mFBOs[0].getBounds()); // Pass original image RGB data to
    t1 = TriangleEdge(Vec2f(159.11,705.72), Vec2f(230.56,37.08),Vec2f(378.99,509.02) );
    t2 = TriangleEdge(Vec2f(230.56,37.08),Vec2f(378.99,509.02), Vec2f(507.78,485.49));
    t3 = TriangleEdge(Vec2f(230.56,37.08), Vec2f(507.78,485.49), Vec2f(791.81,543.37));
    t4 = TriangleEdge(Vec2f(230.56,37.08), Vec2f(791.81,543.37), Vec2f(804.56,69.25 ));
    t5 = TriangleEdge(Vec2f(804.56,69.25 ),Vec2f(791.81,543.37), Vec2f(1021.12,0.2 ));
    t6 = TriangleEdge(Vec2f(791.81,543.37), Vec2f(1021.12,0.2 ), Vec2f(1098.41,267.96));
    t7 = TriangleEdge(Vec2f(791.81,543.37),Vec2f(1098.41,267.96), Vec2f(941.21,483.61 ));
    t8 = TriangleEdge(Vec2f(1021.12,0.2 ), Vec2f(1128.58,102.51), Vec2f(1098.41,267.96));
    t9 = TriangleEdge(Vec2f(941.21,483.61 ), Vec2f(1082.44,518.38), Vec2f(1098.41,267.96));
}

void RDIFFSYPHONApp::update(){
    counter.resize(18);
    totDepth.resize(18);
    aveDepth.resize(18);

    //KINECT DATA LOGGING
    for(int i =0; i<19; i++){
        counter[i] = 1;
        totDepth[i] = 1;
    }
  depth = mKinect->getDepthData();
    
    for (int i = 0; i<x; i+=3){
        for (int j = 0;j<y;j+=3){
            col = lmap((float) depth.get()[i+j*x], 0.f, 55000.f, 1.0f, 0.f);
            
            if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>6 && i<39){
                mySurface.setPixel(Vec2i(i,j),ColorA(2*col/3,col,col));
                counter[0]++;
                totDepth[0]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>40 && i<72){
                mySurface.setPixel(Vec2i(i,j),ColorA(col/2,col/2,col));
                counter[1]++;
                totDepth[1]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>73 && i<105){
                mySurface.setPixel(Vec2i(i,j),ColorA(col,col/2,col/2));
                counter[2]++;
                counter[2]+=depth.get()[i+j*x];
            }else  if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>106 && i<138){
                mySurface.setPixel(Vec2i(i,j),ColorA(2*col/3,col,col));
                counter[3]++;
                totDepth[3]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>139 && i<171){
                mySurface.setPixel(Vec2i(i,j),ColorA(col/2,col/2,col));
                counter[4]++;
                totDepth[4]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>172 && i<204){
                mySurface.setPixel(Vec2i(i,j),ColorA(col,col/2,col/2));
                counter[5]++;
                totDepth[5]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>205 && i<237){
                mySurface.setPixel(Vec2i(i,j),ColorA(2*col/3,col,col));
                counter[6]++;
                totDepth[6]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>238 && i<270){
                mySurface.setPixel(Vec2i(i,j),ColorA(col/2,col/2,col));
                counter[7]++;
                totDepth[7]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>271 && i<303){
                mySurface.setPixel(Vec2i(i,j),ColorA(col,col/2,col/2));
                counter[8]++;
                totDepth[8]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>304 && i<336){
                mySurface.setPixel(Vec2i(i,j),ColorA(col,col/2,col/2));
                counter[9]++;
                totDepth[9]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>337 && i<369){
                mySurface.setPixel(Vec2i(i,j),ColorA(col,col/2,col/2));
                counter[10]++;
                totDepth[10]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>370 && i<402){
                mySurface.setPixel(Vec2i(i,j),ColorA(col,col/2,col/2));
                counter[11]++;
                totDepth[11]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>403 && i<435){
                mySurface.setPixel(Vec2i(i,j),ColorA(col,col/2,col/2));
                counter[12]++;
                totDepth[12]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>436 && i<468){
                mySurface.setPixel(Vec2i(i,j),ColorA(col,col/2,col/2));
                counter[13]++;
                totDepth[13]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>469 && i<501){
                mySurface.setPixel(Vec2i(i,j),ColorA(col,col/2,col/2));
                counter[14]++;
                totDepth[14]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>502 && i<534){
                mySurface.setPixel(Vec2i(i,j),ColorA(col,col/2,col/2));
                counter[15]++;
                totDepth[15]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>535 && i<567){
                mySurface.setPixel(Vec2i(i,j),ColorA(col,col/2,col/2));
                counter[16]++;
                totDepth[16]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>568 && i<600){
                mySurface.setPixel(Vec2i(i,j),ColorA(col,col/2,col/2));
                counter[17]++;
                totDepth[17]+=depth.get()[i+j*x];
            }else if (depth.get()[i+j*x]>mFarThresh && depth.get()[i+j*x]<mCloseThresh && i>601 && i<633){
                mySurface.setPixel(Vec2i(i,j),ColorA(col,col/2,col/2));
                counter[18]++;
                totDepth[18]+=depth.get()[i+j*x];
            }
            else{
                 mySurface.setPixel(Vec2i(i,j),ColorA(.0,.0,.0));
            }
            
        }
        
    }
    for(int i = 0; i<19; i++){
    aveDepth[i] = lmap((float)(totDepth[i]/counter[i]), 0.f,55000.f,0.f,1.f);
    }
    
    mCam.lookAt(Vec3f(0,mTilt,700),Vec3f(0,0,0),Vec3f::yAxis());
    w->step();
     w2->step();
    //FBO PING PONGING
    const int ITERATIONS = mSpeed;
    gl::setMatricesWindow( mFBOs[0].getSize(), false );
	gl::setViewport( mFBOs[0].getBounds() );
	for( int i = 0; i < ITERATIONS; i++ ) {
		mCurrentFBO = ( mCurrentFBO + 1 ) % 2;
		mOtherFBO   = ( mCurrentFBO + 1 ) % 2;
		
		mFBOs[ mCurrentFBO ].bindFramebuffer();
        mFBOs[ mOtherFBO ].bindTexture();
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
        //mShader->uniform( "scale", mScale );
        
		gl::drawSolidRect( mFBOs[ mCurrentFBO ].getBounds() );
		mShader->unbind();
        mFBOs[ mOtherFBO ].unbindTexture();
        gl::pushMatrices();
        gl::translate(Vec2f(getWindowWidth()/2, getWindowHeight()/2));
        glColor4f( 1.0f, 1.0f, 1.0f, -.5f );
        RectMapping windowToFBO( getWindowBounds(), mFBOs[mCurrentFBO].getBounds() );
        gl::drawSolidCircle( windowToFBO.map( Vec2f(w->x,w->y) ), 5.0f, 32 );
        gl::drawSolidCircle( windowToFBO.map( Vec2f(w2->y,w2->x) ), 5.0f, 32 );
        gl::popMatrices();
        
     
        int highest;
        int hiIndex = 19;
        for(int i =0 ; i < 19; i++){
            if (counter[i]>highest){
                highest = counter[i];
                hiIndex = i;
            }
        }
        if      (hiIndex == 0){ t9.drawEdge1(mSize);}
        else if (hiIndex == 1){ t9.drawEdge2(mSize);}
        else if (hiIndex == 2){ t7.drawEdge3(mSize);}
        else if (hiIndex == 3){ t7.drawEdge2(mSize);}
        else if (hiIndex == 4){ t8.drawEdge2(mSize);}
        else if (hiIndex == 5){ t8.drawEdge1(mSize);}
        else if (hiIndex == 6){ t6.drawEdge2(mSize);}
        else if (hiIndex == 7){ t6.drawEdge3(mSize);}
        else if (hiIndex == 8){ t5.drawEdge2(mSize);}
        else if (hiIndex == 9){ t5.drawEdge3(mSize);}
        else if (hiIndex == 10){ t4.drawEdge2(mSize);}
        else if (hiIndex == 11){ t4.drawEdge3(mSize);}
        else if (hiIndex == 12){ t3.drawEdge3(mSize);}
        else if (hiIndex == 13){ t3.drawEdge2(mSize);}
        else if (hiIndex == 14){ t2.drawEdge3(mSize);}
        else if (hiIndex == 15){ t2.drawEdge2(mSize);}
        else if (hiIndex == 16){ t1.drawEdge2(mSize);}
        else if (hiIndex == 17){ t1.drawEdge3(mSize);}
        else if (hiIndex == 18){ t1.drawEdge1(mSize);}
        
        
        console()<<hiIndex<<std::endl;
        loopEdges(cur);
        if (textFeed==false){
            testMode();
        }
        mFBOs[ mCurrentFBO ].unbindFramebuffer();
        
	}
    
    float totDepth = 0.f;
    float countave = 0.f;
    for (int i = 0; i<19; i++){
        
        totDepth += aveDepth[i];
        countave +=counter[i];
    }
 
   // console() << totDepth << std::endl;
    console()<< "count: " <<std::endl;
    console() << countave<< std::endl;

    totDepth = totDepth/19;
    float countMap = 0;
    countMap = lmap((float)countave,500.f,30070.f, .06f,.03f) ;
    mReactionF = totDepth/6;
    if (countMap<1.00 && countMap>.0f){
        mReactionK = countMap;}

    console()<< "countMap: " <<std::endl;
    console() << countMap<< std::endl;
    if (countave< 18000 && countave>200){
    mSpeed = (int)lmap((float)countave, 2000.f, 18000.f, 25.f, 4.f);
    }
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
    gl::draw(mWrapper);
    mShaderRefraction->unbind();
    mFBOs[mCurrentFBO].unbindTexture();
    mScreenSyphon.publishScreen(); //publish the screen's output
	//mTextureSyphon.publishTexture(mLogo); //publish our texture without shader
    //p[prrgl::draw(mySurface,Vec2f(SIZE,0));
    gl::popMatrices();
   
    
    mParams->draw();
    
    //mClientSyphon.draw(Vec2f(16.f, 64.f)); //draw our client image
    
	
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
        
        
	}
    gl::Fbo::unbindFramebuffer();
    
}

void RDIFFSYPHONApp::loopEdges(int cur){
        if (getElapsedFrames() == (cur+rate)     ){t1.drawEdge1(mSize);}
        if (getElapsedFrames() == (cur+ rate*2)  ){t1.drawEdge3(mSize);}
        if (getElapsedFrames() == (cur+ rate*3)  ){t1.drawEdge2(mSize);}
        if (getElapsedFrames() == (cur+ rate*4)  ){t2.drawEdge2(mSize);}
        if (getElapsedFrames() == (cur+ rate*5)  ){t2.drawEdge3(mSize);}
        if (getElapsedFrames() == (cur+ rate*6)  ){t3.drawEdge2(mSize);}
        if (getElapsedFrames() == (cur+ rate*7)  ){t3.drawEdge3(mSize);}
        if (getElapsedFrames() == (cur+ rate*8)  ){t4.drawEdge3(mSize);}
        if (getElapsedFrames() == (cur+ rate*9)  ){t4.drawEdge2(mSize);}
        if (getElapsedFrames() == (cur+ rate*10) ){t5.drawEdge3(mSize);}
        if (getElapsedFrames() == (cur+ rate*11) ){t5.drawEdge2(mSize);}
        if (getElapsedFrames() == (cur+ rate*12) ){t6.drawEdge3(mSize);}
        if (getElapsedFrames() == (cur+ rate*13) ){t6.drawEdge2(mSize);}
        if (getElapsedFrames() == (cur+ rate*14) ){t8.drawEdge1(mSize);}
        if (getElapsedFrames() == (cur+ rate*15) ){t8.drawEdge2(mSize);}
        if (getElapsedFrames() == (cur+ rate*16) ){t7.drawEdge2(mSize);}
        if (getElapsedFrames() == (cur+ rate*17) ){t7.drawEdge3(mSize);}
        if (getElapsedFrames() == (cur+ rate*18) ){t9.drawEdge2(mSize);}
        if (getElapsedFrames() == (cur+ rate*19) ){t9.drawEdge1(mSize);}


        cur = cur + rate*9;
    
}

void RDIFFSYPHONApp::testMode(){
    t1.drawTriangle(mSize);
    t2.drawTriangle(mSize);
    t3.drawTriangle(mSize);
    t4.drawTriangle(mSize);
    t5.drawTriangle(mSize);
    t6.drawTriangle(mSize);
    t7.drawTriangle(mSize);
    t8.drawTriangle(mSize);
    t9.drawTriangle(mSize);
}


void RDIFFSYPHONApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'r' ) {
		resetFBOs();
	}
    if (event.getChar()== 'b'){
        textFeed = false;
        textFeed2 = false;
        textFeed3 = false;
        
    }
    if (event.getChar()== 'p'){
        cur = getElapsedFrames();
    }
    

    
}
void RDIFFSYPHONApp::keyUp( KeyEvent event ){
    if (event.getChar()== 'b'){
        textFeed = true;
        textFeed2 = true;
        textFeed3 = true;
    }
    
}
void RDIFFSYPHONApp::keyPressed( KeyEvent event ){
    
}


CINDER_APP_NATIVE( RDIFFSYPHONApp, RendererGl )
