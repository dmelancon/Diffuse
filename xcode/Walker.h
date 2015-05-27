//
//  Walker.h
//  RDIFFSYPHON
//
//  Created by Dan on 4/24/14.
//
//

#pragma once
#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "cinder/Perlin.h"
#include "cinder/CinderMath.h"

typedef std::shared_ptr<class Walker> WalkerRef;
class Walker {
    
public:
    Walker(float x, float y);
    Walker();
    
    void step();
    void draw();
    float x, y;
private:
    
    float tx, ty;
    int radius;
    ci::Perlin noise;
    
};