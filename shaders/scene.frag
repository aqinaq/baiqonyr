#version 330 core
in vec3 worldPos,normal;in vec2 uv;
out vec4 fragColor;
uniform vec3 baseColor,eye,sunDir,enginePos;
uniform float shininess,emission,enginePower,night,altitude;
uniform int material,lightsOn;
uniform sampler2D concrete;
// Original Blinn-Phong implementation. See CREDITS and report for model references.
vec3 illuminate(vec3 n,vec3 v,vec3 l,vec3 color,float strength){
 float d=max(dot(n,l),0.0);
 float s=d>0.0?pow(max(dot(n,normalize(l+v)),0.0),shininess):0.0;
 return strength*color*(baseColor*d+vec3(.34)*s);
}
void main(){
 vec3 n=normalize(normal),v=normalize(eye-worldPos);
 vec3 albedo=baseColor;
 if(material==1)albedo*=texture(concrete,uv*8.0).rgb;
 if(material==2){float grain=sin(worldPos.x*.34+sin(worldPos.z*.14))*sin(worldPos.z*.27);albedo*=.88+.12*grain;}
 vec3 lit=albedo*mix(.23,.07,night);
 lit+=illuminate(n,v,normalize(sunDir),mix(vec3(1.0,.78,.52),vec3(.24,.36,.65),night),.92)*(albedo/max(baseColor,vec3(.01)));
 vec3 delta=enginePos-worldPos;float d=length(delta);
 lit+=illuminate(n,v,normalize(delta+vec3(0,.001,0)),vec3(1,.3,.045),enginePower/(1+.045*d+.015*d*d));
 if(lightsOn==1){vec3 lamp=vec3(-8,17,5)-worldPos;float r=length(lamp);lit+=illuminate(n,v,normalize(lamp),vec3(.52,.8,1),4.0/(1+.05*r+.025*r*r));}
 lit+=albedo*emission;
 float fog=1-exp(-length(eye-worldPos)*.0014*(1-altitude));
 lit=mix(lit,mix(vec3(.57,.44,.3),vec3(.025,.04,.075),night),fog);
 fragColor=vec4(pow(max(lit,vec3(0)),vec3(1.0/2.2)),1);
}
