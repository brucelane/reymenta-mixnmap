#version 120
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
uniform sampler2D   iChannel7;				// input channel 7
uniform sampler2D   iAudio0;				// input channel 0 (audio)
uniform vec4        iMouse;              	// mouse pixel coords. xy: current (if MLB down), zw: click
uniform float       iGlobalTime;         	// shader playback time (in seconds)
uniform float       iFreq0;              	// sound
uniform float       iFreq1;              	// sound
uniform float       iFreq2;              	// sound
uniform float       iFreq3;              	// sound
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
