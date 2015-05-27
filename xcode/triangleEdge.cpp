//
//  triangleEdge.cpp
//  RDIFFSYPHON
//
//  Created by Dan on 4/29/14.
//
//

#include "triangleEdge.h"

TriangleEdge::TriangleEdge(){}

TriangleEdge::TriangleEdge(Vec2f A1,Vec2f A2,Vec2f A3) {
    mA1 = A1;
    mA2 = A2;
    mA3 = A3;
 
}

void TriangleEdge::drawTriangle(float mSize){
    drawEdge1(mSize);
    drawEdge2(mSize);
    drawEdge3(mSize);
    }

void TriangleEdge::drawEdge1(float mSize){
    glBegin(GL_LINES);
    glVertex2f(mA1.x/(mSize*1.7),mA1.y/(mSize*1.7));
    glVertex2f(mA2.x/(mSize*1.7),mA2.y/(mSize*1.7));
    glEnd();
    
}

void TriangleEdge::drawEdge2(float mSize){
    glBegin(GL_LINES);
    glVertex2f(mA2.x/(mSize*1.7),mA2.y/(mSize*1.7));
    glVertex2f(mA3.x/(mSize*1.7),mA3.y/(mSize*1.7));
    glEnd();
    
}

void TriangleEdge::drawEdge3(float mSize){
    glBegin(GL_LINES);
    glVertex2f(mA3.x/(mSize*1.7),mA3.y/(mSize*1.7));
    glVertex2f(mA1.x/(mSize*1.7),mA1.y/(mSize*1.7));
    glEnd();
    
}