#version 330 core
in vec3 worldPos,normal;in vec2 uv;
out vec4 fragColor;
uniform vec3 baseColor,eye,sunDir,enginePos;
uniform float shininess,emission,enginePower,night,altitude,time,flagReveal;
uniform int material,lightsOn;
uniform sampler2D concrete;
float groundHash(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
float groundNoise(vec2 p){
 vec2 i=floor(p),f=fract(p);f=f*f*(3.0-2.0*f);
 return mix(mix(groundHash(i),groundHash(i+vec2(1,0)),f.x),
            mix(groundHash(i+vec2(0,1)),groundHash(i+vec2(1,1)),f.x),f.y);
}
float lineMark(vec2 p,vec2 a,vec2 b,float width){
 vec2 d=b-a;return 1.0-smoothstep(width,width+.003,length(p-a-d*clamp(dot(p-a,d)/dot(d,d),0.0,1.0)));
}
vec3 flagColor(vec2 p){
 vec2 q=(p-vec2(.55,.60))*vec2(2,1);
 float angle=atan(q.y,q.x),radius=length(q);
 float sun=1.0-smoothstep(.085,.09,radius);
 float rays=(1.0-smoothstep(.12,.145,radius))*smoothstep(.098,.105,radius)*step(.55,cos(angle*32.0));
 float eagle=lineMark(p,vec2(.36,.35),vec2(.55,.29),.014)+lineMark(p,vec2(.55,.29),vec2(.74,.35),.014);
 for(int i=0;i<7;i++){
  float f=float(i)/7.0;
  eagle+=lineMark(p,vec2(.36+f*.17,.35-f*.055),vec2(.34+f*.18,.40-f*.07),.006);
  eagle+=lineMark(p,vec2(.74-f*.17,.35-f*.055),vec2(.76-f*.18,.40-f*.07),.006);
 }
 float ornament=(1.0-smoothstep(.012,.018,abs(p.x-.09-.018*sin(p.y*65.0))))*step(.05,p.y)*step(p.y,.95);
 return mix(vec3(.015,.60,.80),vec3(1,.78,.12),clamp(sun+rays+eagle+ornament,0.0,1.0));
}
float surfaceNoise(vec3 p){return (groundNoise(p.xy)+groundNoise(p.yz)+groundNoise(p.zx))/3.0;}
// Original Blinn-Phong implementation. See CREDITS and report for model references.
vec3 illuminate(vec3 n,vec3 v,vec3 l,vec3 color,float strength){
 float d=max(dot(n,l),0.0);
 float s=d>0.0?pow(max(dot(n,normalize(l+v)),0.0),shininess):0.0;
 return strength*color*(baseColor*d+vec3(.34)*s);
}
void main(){
 vec3 n=normalize(normal),v=normalize(eye-worldPos);
 if(material==5){
  float granules=groundNoise(uv*170.0)*.55+groundNoise(uv*47.0)*.45;
  float limb=pow(max(dot(n,v),0.0),.4);
  vec3 solar=mix(vec3(.65,.12,.015),vec3(1.0,.76,.3),granules*.6+limb*.4);
  fragColor=vec4(solar,1);return;
 }
 vec3 albedo=baseColor;
 if(material==6 || material==7){
  vec2 p=material==6 ? vec2((worldPos.x-0.0)/1400.0,(worldPos.z+250.0)/700.0) : uv;
  fragColor=vec4(flagColor(p),1);return;
 }
 if(material==19){
  if((uv.y>.48 && uv.y<.54) || uv.y>.985) discard;
  float bands=.65+.2*sin(uv.y*190.0)+.15*groundNoise(uv*vec2(300,80));
  albedo*=bands;
  if(!gl_FrontFacing)n=-n;
 }
 if(material>=10 && material<=18){
  float broad=surfaceNoise(n*7.0),detail=surfaceNoise(n*85.0);
  if(material==10 || material==11 || material==16){
   albedo*=.55+broad*.65+detail*.22;
   for(int i=0;i<28;i++){
    float f=float(i)+float(material)*17.0;
    vec3 center=normalize(vec3(sin(f*12.3),cos(f*7.13),sin(f*4.79)));
    float r=.025+.075*groundHash(vec2(f,2));
    float d=length(n-center)/r;
    albedo*=1.0-.38*(1.0-smoothstep(.65,.95,d));
    albedo+=vec3(.13)*exp(-pow((d-1.0)*8.0,2.0));
   }
   if(material==11){
    float canyon=1.0-smoothstep(.015,.045,abs(n.y+.12*sin(n.x*15.0)+.15));
    albedo*=1.0-canyon*.35;
    albedo=mix(albedo,vec3(.85,.83,.74),smoothstep(.88,.97,abs(n.y)));
   }
  } else {
   float bands=sin(n.y*65.0+surfaceNoise(n*12.0)*5.0+time*.015);
   albedo*=.65+.3*broad+.18*bands+.16*detail;
   if(material==14){
    float storm=length((n.xy-vec2(.45,-.22))*vec2(4,9));
    albedo=mix(albedo,vec3(.53,.17,.07),1.0-smoothstep(.6,1.0,storm));
   }
   if(material==13)albedo=mix(albedo,vec3(.9,.72,.36),smoothstep(.4,.7,broad)*.55);
   if(material==18)albedo=mix(albedo,vec3(.5,.7,.95),pow(max(0.0,bands),20.0)*.35);
  }
 }

 if(material==1)albedo*=texture(concrete,uv*8.0).rgb;
 if(material==2){

  vec2 p=worldPos.xz;
  float broad=groundNoise(p*.025),patch=groundNoise(p*.16);
  float detailFade=1.0-smoothstep(100.0,450.0,length(eye-worldPos));
  float grain=groundNoise(p*2.5)*detailFade;
  albedo*=.72+.32*broad+.16*patch+.08*grain;
  albedo=mix(albedo,albedo*vec3(.73,.86,.58),smoothstep(.52,.76,broad)*.42);
 }
 if(material==3){
  float land=groundNoise(uv*vec2(22,12))*.65+groundNoise(uv*vec2(49,27))*.35;
  albedo=mix(vec3(.035,.16,.37),vec3(.19,.30,.16),smoothstep(.48,.58,land));
  albedo=mix(albedo,vec3(.85,.89,.91),smoothstep(.78,.94,abs(n.y)));
  float clouds=smoothstep(.58,.76,groundNoise(uv*vec2(55,30)));
  albedo=mix(albedo,vec3(.8,.85,.9),clouds*.65);
 }
 if(material==4){
  float bands=sin(uv.y*90.0+groundNoise(uv*17.0)*4.0)*.12;
  albedo*=.7+groundNoise(uv*90.0)*.35+bands;
 }
 vec3 lit=albedo*mix(.23,.07,night);
 lit+=illuminate(n,v,normalize(sunDir),mix(vec3(1.0,.78,.52),vec3(.24,.36,.65),night),.92)*(albedo/max(baseColor,vec3(.01)));
 vec3 delta=enginePos-worldPos;float d=length(delta);
 lit+=illuminate(n,v,normalize(delta+vec3(0,.001,0)),vec3(1,.3,.045),enginePower/(1+.045*d+.015*d*d));
 if(lightsOn==1){vec3 lamp=vec3(-8,17,5)-worldPos;float r=length(lamp);lit+=illuminate(n,v,normalize(lamp),vec3(.52,.8,1),4.0/(1+.05*r+.025*r*r));}
 lit+=albedo*emission;
 float fog=1-exp(-length(eye-worldPos)*.0014*(1-altitude));
 lit=mix(lit,mix(vec3(.57,.44,.3),vec3(.025,.04,.075),night),fog);
 vec3 color=pow(max(lit,vec3(0)),vec3(1.0/2.2));
 // Reveal the country-sized flag only on the map's top surface during ascent.
 if(material==2 && normal.y>.9 && flagReveal>0.0){
  vec2 p=vec2((worldPos.x+4126.2167)/10000.0, (worldPos.z+3459.1282)/5380.5747);
  p.y=1.0-p.y;
  color=mix(color,flagColor(p),flagReveal);
 }
 fragColor=vec4(color,1);
}
