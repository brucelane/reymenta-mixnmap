// uniforms begin
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

// global functions begin
const float kCharBlank = 12.0;
const float kCharMinus = 11.0;
const float kCharDecimalPoint = 10.0;
//-----------------------------------------------------------------
// Digit drawing function by P_Malin (https://www.shadertoy.com/view/4sf3RN)
float SampleDigit(const in float n, const in vec2 vUV)
{		
	if(vUV.x  < 0.0) return 0.0;
	if(vUV.y  < 0.0) return 0.0;
	if(vUV.x >= 1.0) return 0.0;
	if(vUV.y >= 1.0) return 0.0;
	
	float data = 0.0;
	
	     if(n < 0.5) data = 7.0 + 5.0*16.0 + 5.0*256.0 + 5.0*4096.0 + 7.0*65536.0;
	else if(n < 1.5) data = 2.0 + 2.0*16.0 + 2.0*256.0 + 2.0*4096.0 + 2.0*65536.0;
	else if(n < 2.5) data = 7.0 + 1.0*16.0 + 7.0*256.0 + 4.0*4096.0 + 7.0*65536.0;
	else if(n < 3.5) data = 7.0 + 4.0*16.0 + 7.0*256.0 + 4.0*4096.0 + 7.0*65536.0;
	else if(n < 4.5) data = 4.0 + 7.0*16.0 + 5.0*256.0 + 1.0*4096.0 + 1.0*65536.0;
	else if(n < 5.5) data = 7.0 + 4.0*16.0 + 7.0*256.0 + 1.0*4096.0 + 7.0*65536.0;
	else if(n < 6.5) data = 7.0 + 5.0*16.0 + 7.0*256.0 + 1.0*4096.0 + 7.0*65536.0;
	else if(n < 7.5) data = 4.0 + 4.0*16.0 + 4.0*256.0 + 4.0*4096.0 + 7.0*65536.0;
	else if(n < 8.5) data = 7.0 + 5.0*16.0 + 7.0*256.0 + 5.0*4096.0 + 7.0*65536.0;
	else if(n < 9.5) data = 7.0 + 4.0*16.0 + 7.0*256.0 + 5.0*4096.0 + 7.0*65536.0;
	else if(n < 10.5) data = 2.0 + 0.0 * 16.0 + 0.0 * 256.0 + 0.0 * 4096.0 + 0.0 * 65536.0;// '.'
	else if(n < 11.5) data = 0.0 + 0.0 * 16.0 + 7.0 * 256.0 + 0.0 * 4096.0 + 0.0 * 65536.0;// '-'
		
	vec2 vPixel = floor(vUV * vec2(4.0, 5.0));
	float fIndex = vPixel.x + (vPixel.y * 4.0);
	
	return mod(floor(data / pow(2.0, fIndex)), 2.0);
}

float PrintValue(const in vec2 vStringCharCoords, const in float fValue, const in float fMaxDigits, const in float fDecimalPlaces)
{
	float fAbsValue = abs(fValue);
	
	float fStringCharIndex = floor(vStringCharCoords.x);
	
	float fLog10Value = log2(fAbsValue) / log2(10.0);
	float fBiggestDigitIndex = max(floor(fLog10Value), 0.0);
	
	// This is the character we are going to display for this pixel
	float fDigitCharacter = kCharBlank;
	
	float fDigitIndex = fMaxDigits - fStringCharIndex;
	if(fDigitIndex > (-fDecimalPlaces - 1.5))
	{
		if(fDigitIndex > fBiggestDigitIndex)
		{
			if(fValue < 0.0)
			{
				if(fDigitIndex < (fBiggestDigitIndex+1.5))
				{
					fDigitCharacter = kCharMinus;
				}
			}
		}
		else
		{		
			if(fDigitIndex == -1.0)
			{
				if(fDecimalPlaces > 0.0)
				{
					fDigitCharacter = kCharDecimalPoint;
				}
			}
			else
			{
				if(fDigitIndex < 0.0)
				{
					// move along one to account for .
					fDigitIndex += 1.0;
				}

				float fDigitValue = (fAbsValue / (pow(10.0, fDigitIndex)));

				// This is inaccurate - I think because I treat each digit independently
				// The value 2.0 gets printed as 2.09 :/
				//fDigitCharacter = mod(floor(fDigitValue), 10.0);
				fDigitCharacter = mod(floor(0.0001+fDigitValue), 10.0); // fix from iq
			}		
		}
	}

	vec2 vCharPos = vec2(fract(vStringCharCoords.x), vStringCharCoords.y);

	return SampleDigit(fDigitCharacter, vCharPos);	
}

float PrintValue(const in vec2 vPixelCoords, const in vec2 vFontSize, const in float fValue, const in float fMaxDigits, const in float fDecimalPlaces)
{
	return PrintValue((gl_FragCoord.xy - vPixelCoords) / vFontSize, fValue, fMaxDigits, fDecimalPlaces);
}

vec3 spotLight( vec3 curSample )
{

	vec2 lightPos = vec2(
		(1.2 + sin(iGlobalTime)) * 0.4 * iResolution.x,
		(1.2 + cos(iGlobalTime)) * 0.4 * iResolution.y
	);
	
	// control with the mouse.
	if (iLightAuto == 1) 
	{
		lightPos = iMouse.xy;
	}
	float lightStrength = 1.6;
	float lightRadius = iRotationSpeed +1.0;//0.7;
	float refDist = 1.2 * iResolution.x / 500.0;	
	
	vec2 vecToLight = gl_FragCoord.xy - lightPos;
	float distToLight = length(vecToLight.xy / iResolution.xy);
	vec2 dirToLight = normalize(vecToLight);
	
	// Attenaute brightness based on how much this fragment seems to face the light
	float directionBrightness = lightStrength - distToLight;
	
	// Attenuate brightness based on distance from the light
	float distanceBrightness = 1.0 - (distToLight / lightRadius);

	return directionBrightness * distanceBrightness * curSample.xyz;
}
vec3 greyScale( vec3 colored )
{
   return vec3( (colored.r+colored.g+colored.b)/3.0 );
}
float glitchHash(float x)
{
	return fract(sin(x * 11.1753) * 192652.37862);
}
float glitchNse(float x)
{
	float fl = floor(x);
	return mix(glitchHash(fl), glitchHash(fl + 1.0), smoothstep(0.0, 1.0, fract(x)));
}
// global functions end
// left main lines begin

vec3 shaderLeft(vec2 uv)
{
	vec4 left = texture2D(iChannel0, uv);
	return vec3( left.r, left.g, left.b );
}

// left main lines end
// right main lines begin

vec3 shaderRight(vec2 uv)
{
	vec4 right = texture2D(iChannel1, uv);
	return vec3( right.r, right.g, right.b );
}

// right main lines end
vec3 mainFunction( vec2 uv )
{
   return mix( shaderLeft(uv), shaderRight(uv), iCrossfade );
}
// main start
void main(void)
{
	vec2 uv = gl_TexCoord[0].st;
	uv.x -= iRenderXY.x;
	uv.y -= iRenderXY.y;
	uv *= iZoom;

	if (iGlitch == 1) 
	{
		// glitch the point around
		float s = iTempoTime * iRatio;//50.0;
		float te = iTempoTime * 9.0 / 16.0;//0.25 + (iTempoTime + 0.25) / 2.0 * 128.0 / 60.0;
		vec2 shk = (vec2(glitchNse(s), glitchNse(s + 11.0)) * 2.0 - 1.0) * exp(-5.0 * fract(te * 4.0)) * 0.1;
		uv += shk;		
	}
	// pixelate
	/*if ( iPixelate < 59.0 )
	{
		vec2 divs = vec2(iResolution.x * iPixelate / iResolution.y, iPixelate);
		uv = floor(uv * divs)/ divs;
	}*/
	if ( iPixelate < 1.0 )
	{
		vec2 divs = vec2(gl_TexCoord[0].s * iPixelate*60.0 / gl_TexCoord[0].t, iPixelate*60.0);
		uv = floor(uv * divs)/ divs;
	}
	vec3 col;
	if ( iCrossfade > 0.95 )
	{
		col = shaderRight(uv);
	}
	else
	{
		if ( iCrossfade < 0.05 )
		{
			col = shaderLeft(uv);
		}
		else
		{
			col = mainFunction( uv );

		}
	}
	if (iLight == 1) 
	{
		col = spotLight( col );
	}
	if (iToggle == 1) 
	{
		col.rgb = col.gbr;
	}
	// grey scale mode
	if (iGreyScale == 1)
	{
		col = greyScale( col );
	}
	col *= iExposure;
	if (iInvert == 1) col = 1.-col;
	if (iVignette == 1)
	{
		vec2 p = 1.0 + -2.0 * uv;
		col = mix( col, vec3( iBackgroundColor ), dot( p, p )*iRotationSpeed );
	}

	vec2 vFontSize = vec2(16.0, 20.0);
	vec2 vPixelCoord0 = vec2(5.0, 5.0);
	float fDigits = 3.0;
	float fDecimalPlaces = 0.0;
	// Show FPS
	if (iDebug == 1)
	{
		if (iShowFps == 1)
		{
			float fIsDigit4 = PrintValue(vPixelCoord0, vFontSize, iFps, fDigits, fDecimalPlaces);
			col = mix( col, iColor, fIsDigit4);
		}
		else
		{
			float fIsDigit5 = PrintValue(vPixelCoord0, vFontSize, iGlobalTime, fDigits, 2.0);
			col = mix( col, iColor, fIsDigit5);
		}
	}

	gl_FragColor = iAlpha * vec4( col, 1.0 );

}
// main end