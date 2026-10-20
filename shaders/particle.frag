#version 330 core
in vec2 uv;in vec4 tint;out vec4 fragColor;
void main(){float r=dot(uv,uv);if(r>1)discard;float a=pow(1-r,2);fragColor=vec4(tint.rgb,tint.a*a);}
