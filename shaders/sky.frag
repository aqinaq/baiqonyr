#version 330 core
in vec2 uv;out vec4 fragColor;
uniform mat4 invVP;uniform vec3 eye;uniform float altitude,night,time;
float hash(vec3 p){p=fract(p*.1031);p+=dot(p,p.yzx+33.33);return fract((p.x+p.y)*p.z);}
void main(){
 vec4 w=invVP*vec4(uv*2-1,1,1);vec3 ray=normalize(w.xyz/w.w-eye);
 float h=pow(max(ray.y+.06,0.0),.5);
 vec3 day=mix(vec3(.91,.61,.34),vec3(.18,.34,.52),clamp(h,0,1));
 vec3 dark=mix(vec3(.032,.045,.09),vec3(.003,.007,.024),clamp(h,0,1));
 vec3 col=mix(day,dark,max(altitude,night*.9));
 vec3 sun=normalize(vec3(-.6,.22,-.8));float s=max(dot(ray,sun),0);
 col+=vec3(1,.7,.35)*pow(s,1400)*(1-altitude)*(1-night);
 col+=vec3(.28,.12,.04)*pow(s,16)*(1-altitude)*(1-night);
 vec3 grid=ray*420;float star=hash(floor(grid));vec3 local=fract(grid)-.5;
 float sparkle=pow(max(0.,1-length(local)*2),6.0)*step(.991,star);
 col+=vec3(.75,.86,1)*sparkle*6*smoothstep(.35,.8,max(altitude,night));
 fragColor=vec4(col,1);
}
