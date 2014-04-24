#version 120
uniform float scale;
uniform int size;
uniform sampler2D displacementMap;
uniform float colorValR;
uniform float colorValG;
uniform float colorValB;
uniform float colorHeight;
varying vec3 color;


void main(void)
{
vec4 newVertexPos;
vec4 dv;
float df;
    
gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;

dv = texture2D( displacementMap, gl_MultiTexCoord0.xy/size );

df = .1*dv.x + 0.1*dv.y + 0.1*dv.z;

newVertexPos = vec4(gl_Normal * dv.b * scale, 0.0) + gl_Vertex;
    
    if (newVertexPos.z > colorHeight){
        color = mix(vec3(0., 0.0, 0.),vec3(colorValG,colorValR,colorValB), dv.b);}
    else{
       color = mix(vec3(1.0, 1.0, 1.),vec3(colorValR,colorValG,colorValB), dv.b );}


gl_Position = gl_ModelViewProjectionMatrix * newVertexPos;
}
