#version 330 core
//There's not much reason to change anything here, I'd move on to flat.frag.

layout(location = 0) in vec2 Position;
layout(location = 1) in vec2 InputUV;

out vec2 UV;

uniform float ScreenWidth;
uniform float ScreenHeight;

void main(){
	
	vec2 VertexPosition = Position - vec2(ScreenWidth,ScreenHeight)/2;
	VertexPosition /= vec2(ScreenWidth,ScreenHeight);
	
	gl_Position =  vec4(VertexPosition*2,0,1);

	UV = InputUV;
	
}