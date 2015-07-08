#version 100
attribute highp vec4 vertex;
attribute highp vec2 texCoords;
//attribute highp vec4 qt_MultiTexCoord0;
//uniform highp mat4 qt_ModelViewProjectionMatrix;
uniform highp mat4 projection, rotate;
uniform bool useTexture;
//varying highp vec4 qt_TexCoord0;
varying highp vec2 texCoord;

void main(void)
{
    gl_Position = projection * rotate * vertex;
    gl_PointSize = 1.0;
    if (useTexture)
        texCoord = texCoords;
    //gl_Position = qt_ModelViewProjectionMatrix * qt_Vertex;
    //qt_TexCoord0 = qt_MultiTexCoord0;
}
