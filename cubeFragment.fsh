#version 100
uniform highp vec4 colour;
uniform bool useTexture;
uniform sampler2D texture;
varying highp vec2 texCoord;

void main(void)
{
    if (!useTexture)
	gl_FragColor.rgba = colour;
    else
	gl_FragColor = texture2D(texture, texCoord);
}
