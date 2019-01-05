#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;

out VS_OUT {
	vec3 pos;
	vec4 color;
} vs_out;

void main(){
	gl_Position = vec4(aPos, 1.0);
	vs_out.pos = aPos;
	vs_out.color = aColor;
}