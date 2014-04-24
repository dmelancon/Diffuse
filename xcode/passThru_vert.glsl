
void main() {
    vec4 v = vec4(gl_Vertex);
    gl_TexCoord[0] = gl_MultiTexCoord0;
//    v.y += .05;
//    v.y -=.1;
gl_Position = gl_ModelViewProjectionMatrix * v;
}

