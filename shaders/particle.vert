#version 330 core
layout(location=0) in vec2 corner;
layout(location=1) in vec4 positionSize;
layout(location=2) in vec4 color;
uniform mat4 view,projection;
out vec2 uv;out vec4 tint;
void main(){vec4 p=view*vec4(positionSize.xyz,1);p.xy+=corner*positionSize.w;gl_Position=projection*p;uv=corner;tint=color;}
