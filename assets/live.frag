// uniforms begin
#version 130
uniform vec3        iResolution;         	// viewport resolution (in pixels)
uniform float       iChannelTime[4];     	// channel playback time (in seconds)
uniform vec3        iChannelResolution[4];	// channel resolution (in pixels)
uniform sampler2D   iChannel0;				// input channel 0 (TODO: support samplerCube)
uniform sampler2D   iChannel1;				// input channel 1 
uniform sampler2D   iChannel2;				// input channel 2
uniform sampler2D   iChannel3;				// input channel 3
uniform sampler2D   iChannel4;				// input channel 4
uniform sampler2D   iChannel5;				// input channel 5
uniform sampler2D   iChannel6;				// input channel 6
uniform sampler2D   iChannel7;        // input channel 7
uniform sampler2D   iAudio0;				// input channel 0 (audio)
uniform vec4        iMouse;              	// mouse pixel coords. xy: current (if MLB down), zw: click
uniform float       iGlobalTime;         	// shader playback time (in seconds)
uniform vec3        iBackgroundColor;    	// background color
uniform vec3        iColor;              	// color
uniform int         iSteps;              	// steps for iterations
uniform int         iFade;               	// 1 for fade out
uniform int         iToggle;             	// 1 for toggle
uniform float       iRatio;
uniform vec2        iRenderXY;           	// move x y 
uniform float       iZoom;               	// zoom
uniform int        	iBlendmode;          	// blendmode for channels
uniform float		iRotationSpeed;	  		// Rotation Speed
uniform float       iCrossfade;          	// CrossFade 2 shaders
uniform float       iPixelate;           	// pixelate
uniform int         iGreyScale;          	// 1 for grey scale mode
uniform float       iAlpha;          	  	// alpha
uniform int         iLight;   			  	// 1 for light
uniform int         iLightAuto;          	// 1 for automatic light
uniform float       iExposure;           	// exposure
uniform float       iDeltaTime;          	// delta time between 2 tempo ticks
uniform int         iTransition;   			// transition type
uniform float       iAnim;          		// animation
uniform int         iRepeat;           		// 1 for repetition
uniform int         iVignette;           	// 1 for vignetting
uniform int         iInvert;           		// 1 for color inversion
uniform int         iDebug;           		// 1 to show debug
uniform int         iShowFps;           	// 1 to show fps
uniform float       iFps;          			// frames per second
uniform float       iTempoTime;
uniform vec4		iDate;					// (year, month, day, time in seconds)
uniform int         iGlitch;           		// 1 for glitch
const 	float 		PI = 3.14159265;
// uniforms end
void rect(vec4 _p,vec3 _c)
{
	vec2 p=gl_FragCoord.xy;
    if((_p.x<p.x&&p.x<_p.x+_p.z&&_p.y<p.y&&p.y<_p.y+_p.w))gl_FragColor=vec4(_c,0.);
}

void print(float _i,vec2 _f,vec2 _p,vec3 _c)
{
    bool n=(_i<0.)?true:false;
    _i=abs(_i);
    if(gl_FragCoord.x<_p.x-5.-(max(ceil(log(_i)/log(10.)),_f.x)+(n?1.:0.))*30.||_p.x+6.+_f.y*30.<gl_FragCoord.x||gl_FragCoord.y<_p.y||_p.y+31.<gl_FragCoord.y)return;
    
    if(0.<_f.y){rect(vec4(_p.x-5.,_p.y,11.,11.),vec3(1.));rect(vec4(_p.x-4.,_p.y+1.,9.,9.),_c);}
    
    float c=-_f.y,m=0.;
    for(int i=0;i<16;i++)
    {
        float x,y=_p.y;
        if(0.<=c){x=_p.x-35.-30.*c;}
        else{x=_p.x-25.-30.*c;}
        if(int(_f.x)<=int(c)&&_i/pow(10.,c)<1.&&0.<c)
        {
            if(n){rect(vec4(x,y+10.,31.,11.),vec3(1.));rect(vec4(x+1.,y+11.,29.,9.),_c);}
            break;
        }
        float l=fract(_i/pow(10.,c+1.));
        if(l<.1){rect(vec4(x,y,31.,31.),vec3(1.));rect(vec4(x+1.,y+1.,29.,29.),_c);rect(vec4(x+15.,y+10.,1.,11.),vec3(1.));}
        else if(l<.2){rect(vec4(x+5.,y,21.,31.),vec3(1.));rect(vec4(x,y,31.,11.),vec3(1.));rect(vec4(x,y+20.,6.,11.),vec3(1.));rect(vec4(x+6.,y+1.,19.,29.),_c);rect(vec4(x+1.,y+1.,29.,9.),_c);rect(vec4(x+1.,y+21.,5.,9.),_c);}
        else if(l<.3){rect(vec4(x,y,31.,31.),vec3(1.));rect(vec4(x+1.,y+1.,29.,29.),_c);rect(vec4(x+15.,y+10.,15.,1.),vec3(1.));rect(vec4(x+1.,y+20.,15.,1.),vec3(1.));}
        else if(l<.4){rect(vec4(x,y,31.,31.),vec3(1.));rect(vec4(x+1.,y+1.,29.,29.),_c);rect(vec4(x+1.,y+10.,15.,1.),vec3(1.));rect(vec4(x+1.,y+20.,15.,1.),vec3(1.));}
        else if(l<.5){rect(vec4(x,y+5.,15.,26.),vec3(1.));rect(vec4(x+15.,y,16.,31.),vec3(1.));rect(vec4(x+1.,y+6.,14.,24.),_c);rect(vec4(x+16.,y+1.,14.,29.),_c);rect(vec4(x+15.,y+6.,1.,10.),_c);}
        else if(l<.6){rect(vec4(x,y,31.,31.),vec3(1.));rect(vec4(x+1.,y+1.,29.,29.),_c);rect(vec4(x+1.,y+10.,15.,1.),vec3(1.));rect(vec4(x+15.,y+20.,15.,1.),vec3(1.));}
        else if(l<.7){rect(vec4(x,y,31.,31.),vec3(1.));rect(vec4(x+1.,y+1.,29.,29.),_c);rect(vec4(x+10.,y+10.,11.,1.),vec3(1.));rect(vec4(x+10.,y+20.,20.,1.),vec3(1.));}
        else if(l<.8){rect(vec4(x,y+10.,15.,21.),vec3(1.));rect(vec4(x+15.,y,16.,31.),vec3(1.));rect(vec4(x+1.,y+11.,14.,19.),_c);rect(vec4(x+16.,y+1.,14.,29.),_c);rect(vec4(x+15.,y+20.,1.,10.),_c);}
        else if(l<.9){rect(vec4(x,y,31.,31.),vec3(1.));rect(vec4(x+1.,y+1.,29.,29.),_c);rect(vec4(x+10.,y+10.,11.,1.),vec3(1.));rect(vec4(x+10.,y+20.,11.,1.),vec3(1.));}
        else{rect(vec4(x,y,31.,31.),vec3(1.));rect(vec4(x+1.,y+1.,29.,29.),_c);rect(vec4(x+1.,y+10.,20.,1.),vec3(1.));rect(vec4(x+10.,y+20.,11.,1.),vec3(1.));}
        c+=1.;
    }
}

void main(void)
{
	vec2 uv = iZoom * gl_FragCoord.xy / iResolution.xy-vec2(0.5);
	uv.x -= iRenderXY.x;
	uv.y -= iRenderXY.y;

  const float eps = 0.1;
  const float r = 0.8;
  const float l = 0.5;
    float roundness = 0.6*iGlobalTime/80.0;

  vec4 color = vec4(iBackgroundColor.r,iBackgroundColor.g,iBackgroundColor.b, 0.0);
  vec4 linecol = vec4(iColor.r,iColor.g,iColor.b,0.0);
vec4 outbuff;

    float s = smoothstep(-0.1,0.1,sin(20.0*(uv.x+uv.y)))-0.5;
    color.xyz = clamp(color.xyz+color.xyz*s,vec3(0),vec3(1));


  float d   = pow(abs(uv.x/l),roundness)+pow(abs(uv.y/l),roundness)-r;
  float sdc = step(0.0,d);
  float sdl = smoothstep(eps-0.1,eps+0.1,abs(d));

  outbuff = mix(color,vec4(0),sdc);
  gl_FragColor = mix(linecol,outbuff,sdl);
  //gl_FragColor = vec4(1.0,0.0,0.0,1.0);
print(iFps,vec2(2.,0.),vec2(520.,20.),vec3(1.,.3,.0));
}


