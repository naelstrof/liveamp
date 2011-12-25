#version 330 core

in vec2 UV;

out vec4 FinalColor;

uniform vec3 Color; //Software generated transitioning colors.
uniform float Amp; //Amplitude of current system sounds playing. (0-1)
uniform sampler2D TextureSampler;
void main(){
	
	vec4 CheckColor = texture2D(TextureSampler,UV);
	float Multi = (CheckColor.r + CheckColor.g + CheckColor.b)/3;
	FinalColor = ((vec4(Amp*Color.r, Amp*Color.g, Amp*Color.b, 0) + CheckColor)/2)*Multi;

}
