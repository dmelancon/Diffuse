#version 120


uniform sampler2D displacementMap;
varying vec3 color;
void main(void)
{

    gl_FragColor.xyz = color;
    gl_FragColor.a = 1.;
    // gl_FragColor = texture2D(displacementMap,gl_TexCoord[0].st);
}
