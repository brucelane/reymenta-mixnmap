#version 110
uniform vec3         iResolution;         // viewport resolution (in pixels)
uniform float        iChannelTime[4];     // channel playback time (in seconds)
uniform vec3         iChannelResolution[4];  // channel resolution (in pixels)
uniform sampler2D    iChannel0;
uniform sampler2D    iChannel1;
uniform sampler2D    iChannel2;
uniform sampler2D    iChannel3;
uniform sampler2D    iChannel4;
uniform sampler2D    iChannel5;
uniform sampler2D    iChannel6;
uniform sampler2D    iAudio0;
uniform vec4         iMouse;              // mouse pixel coords. xy: current (if MLB down), zw: click
uniform float        iGlobalTime;         // shader playback time (in seconds)
uniform float        iFreq0;              // sound
uniform float        iFreq1;              // sound
uniform float        iFreq2;              // sound
uniform float        iFreq3;              // sound
uniform vec3         iBackgroundColor;    // background color
uniform vec3         iColor;              // color
uniform int          iSteps;              // steps for iterations
uniform int          iFade;               // 1 for fade out
uniform int          iToggle;             // 1 for toggle
uniform float        iRatio;
uniform vec2         iRenderXY;           // move x y 
uniform float        iZoom;               // zoom
uniform float        iBlendmode;          // blendmode for channels
uniform float		 iRotationSpeed;	  // Rotation Speed
uniform float        iCrossfade;          // CrossFade 2 shaders
uniform float        iPixelate;           // pixelate

// main
void main()
{
	// pass through
	//vec2 uv = iZoom * gl_TexCoord[0].st;//* vec2(1.0,1.0);
	vec2 uv = iZoom * gl_FragCoord.xy / iResolution.xy; 
	uv.x -= iRenderXY.x;
	uv.y -= iRenderXY.y;
	gl_FragColor = texture2D(iChannel0, uv);
	gl_FragColor.a = 1.0;
}