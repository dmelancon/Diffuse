//
//  triangleEdge.h
//  RDIFFSYPHON
//
//  Created by Dan on 4/29/14.
//
//
#pragma once
#include "cinder/gl/gl.h"
#include "cinder/app/AppBasic.h"
using namespace ci;

class TriangleEdge {
    
public:
    TriangleEdge(Vec2f A1,Vec2f A2,Vec2f A3);
    TriangleEdge();
    void drawTriangle(float mSize);
    void drawEdge1(float mSize);
    void drawEdge2(float mSize);
    void drawEdge3(float mSize);
    Vec2f mA1, mA2, mA3;
};