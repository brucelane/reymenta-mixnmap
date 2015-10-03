// uniforms begin
//#version 130
uniform vec3        iResolution;         	// viewport resolution (in pixels)
uniform float       iChannelTime[4];     	// channel playback time (in seconds)
uniform vec3        iChannelResolution[4];	// channel resolution (in pixels)
uniform sampler2D   iChannel0;				// input channel 0 (TODO: support samplerCube)
uniform sampler2D   iChannel1;				// input channel 1 
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
uniform float       iChromatic;				// chromatic if > 0.
uniform float       iTrixels;           	// trixels if > 0.
uniform float       iGridSize;				// gridSize if > 0.
uniform bool        iFlipH;					// flip horizontally
uniform bool        iFlipV;					// flip vertically
uniform int         iBeat;					// measure from ableton
uniform float       iSeed;					// random 
uniform float       iRedMultiplier;			// red multiplier 
uniform float       iGreenMultiplier;		// green multiplier 
uniform float       iBlueMultiplier;		// blue multiplier 
uniform float       iParam1;				// slitscan (or other) Param1
uniform float       iParam2;				// slitscan (or other) Param2 
uniform bool        iXorY;					// slitscan (or other) effect on x or y
uniform float       iBadTv;					// badtv if > 0.

const 	float 		  PI = 3.14159265;
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
vec4 trixels( vec2 inUV, sampler2D tex )
{
	// trixels https://www.shadertoy.com/view/4lj3Dm
	vec4 rtn;

    float height = iResolution.x/(1.01 - iTrixels)/90.0;
    float halfHeight = height*0.5;
    float halfBase = height/sqrt(3.0);
    float base = halfBase*2.0;

    float screenX = gl_FragCoord.x;
    float screenY = gl_FragCoord.y;    

    float upSlope = height/halfBase;
    float downSlope = -height/halfBase;

    float oddRow = mod(floor(screenY/height),2.0);
    screenX -= halfBase*oddRow;
    
    float oddCollumn = mod(floor(screenX/halfBase), 2.0);

    float localX = mod(screenX, halfBase);
    float localY = mod(screenY, height);

    if(oddCollumn == 0.0 )
    {
        if(localY >= localX*upSlope)
        {
            screenX -= halfBase;
        }
    }
    else
    {
        if(localY <= height+localX*downSlope)
        {
            screenX -= halfBase;
        }
    }
    
    float startX = floor(screenX/halfBase)*halfBase;
    float startY = floor(screenY/height)*height;
    vec4 blend = vec4(0.0,0.0,0.0,0.0);
    for(float x = 0.0; x < 3.0; x += 1.0)
    {
        for(float y = 0.0; y < 3.0; y += 1.0)
        {
            vec2 screenPos = vec2(startX+x*halfBase,startY+y*halfHeight);
            vec2 uv1 = screenPos / iResolution.xy;
			blend += texture2D(tex, uv1);         
        }
    }
    rtn = (blend / 9.0);
   	return rtn;
}
// trixels end
// Squirclimation https://www.shadertoy.com/view/Ml23DW begin
vec4 grid( vec2 inUV, sampler2D tex )
{    
    vec2 uv = (floor(gl_FragCoord.xy/iGridSize)*iGridSize)/ iResolution.xy;
    vec3 texColor;

    texColor = texture2D(tex, uv).xyz;
    
    float diff = pow(distance(texColor,vec3(0.0,1.0,0.0)),8.0); 
    diff = smoothstep(0.0,1.5,diff);
    texColor = mix(iBackgroundColor,texColor,diff);
    
    float texLum = dot(vec3(0.2126,0.7152,0.0722),texColor);
    
    vec3 color = iBackgroundColor;
    
    vec2 ppos = (inUV - uv)/(vec2(iGridSize)/iResolution.xy);
  
    float power = texLum*texLum*16.0;
    float radius = 0.5;
    float dist = pow(abs(ppos.x-0.5),power) + pow(abs(ppos.y - 0.5),power);
    
    if( dist < pow(radius,power))
    {
      color = texColor;
    }
    
    return vec4( color.r, color.g, color.b, 1.0 ); 
}
// Squirclimation end
// global functions end

// left main lines begin
vec3 shaderLeft(vec2 uv)
{
	vec4 left = texture2D(iChannel0, uv);
	// chromatic aberation
	if (iChromatic > 0.0) 
	{
		vec2 offset = vec2(iChromatic/50.,.0);
		left.r = texture2D(iChannel0, uv+offset.xy).r;
		left.g = texture2D(iChannel0, uv          ).g;
		left.b = texture2D(iChannel0, uv+offset.yx).b;
	}
  // Trixels
  if (iTrixels > 0.0) 
  {
        left = trixels( uv, iChannel0 );
  }
  // Grid
  if (iGridSize > 0.0) 
  {
        left = grid( uv, iChannel0 );
  }
	return vec3( left.r, left.g, left.b );
}
// left main lines end

// right main lines begin
vec3 shaderRight(vec2 uv)
{
	vec4 right = texture2D(iChannel1, uv);
	// chromatic aberation
	if (iChromatic > 0.0) 
	{
		vec2 offset = vec2(iChromatic/50.,.0);
		right.r = texture2D(iChannel1, uv+offset.xy).r;
		right.g = texture2D(iChannel1, uv          ).g;
		right.b = texture2D(iChannel1, uv+offset.yx).b;
	}
	// Trixels
	if (iTrixels > 0.0) 
	{
      	right = trixels( uv, iChannel1 );
	}
  	// Grid
  	if (iGridSize > 0.0) 
  	{
        right = grid( uv, iChannel1 );
  	}

	return vec3( right.r, right.g, right.b );
}

// right main lines end

// Blend functions begin
vec3 multiply( vec3 s, vec3 d )
{
   return s*d;
}
vec3 colorBurn( vec3 s, vec3 d )
{
   return 1.0 - (1.0 - d) / s;
}
vec3 linearBurn( vec3 s, vec3 d )
{
   return s + d - 1.0;
}
vec3 darkerColor( vec3 s, vec3 d )
{
   return (s.x + s.y + s.z < d.x + d.y + d.z) ? s : d;
}
vec3 lighten( vec3 s, vec3 d )
{
   return max(s,d);
}
vec3 darken( vec3 s, vec3 d )
{
   return min(s,d);
}
vec3 screen( vec3 s, vec3 d )
{
   return s + d - s * d;
}

vec3 colorDodge( vec3 s, vec3 d )
{
   return d / (1.0 - s);
}

vec3 linearDodge( vec3 s, vec3 d )
{
   return s + d;
}

vec3 lighterColor( vec3 s, vec3 d )
{
   return (s.x + s.y + s.z > d.x + d.y + d.z) ? s : d;
}

float overlay( float s, float d )
{
   return (d < 0.5) ? 2.0 * s * d : 1.0 - 2.0 * (1.0 - s) * (1.0 - d);
}

vec3 overlay( vec3 s, vec3 d )
{
   vec3 c;
   c.x = overlay(s.x,d.x);
   c.y = overlay(s.y,d.y);
   c.z = overlay(s.z,d.z);
   return c;
}

float softLight( float s, float d )
{
   return (s < 0.5) ? d - (1.0 - 2.0 * s) * d * (1.0 - d) 
      : (d < 0.25) ? d + (2.0 * s - 1.0) * d * ((16.0 * d - 12.0) * d + 3.0) 
                : d + (2.0 * s - 1.0) * (sqrt(d) - d);
}

vec3 softLight( vec3 s, vec3 d )
{
   vec3 c;
   c.x = softLight(s.x,d.x);
   c.y = softLight(s.y,d.y);
   c.z = softLight(s.z,d.z);
   return c;
}

float hardLight( float s, float d )
{
   return (s < 0.5) ? 2.0 * s * d : 1.0 - 2.0 * (1.0 - s) * (1.0 - d);
}

vec3 hardLight( vec3 s, vec3 d )
{
   vec3 c;
   c.x = hardLight(s.x,d.x);
   c.y = hardLight(s.y,d.y);
   c.z = hardLight(s.z,d.z);
   return c;
}

float vividLight( float s, float d )
{
   return (s < 0.5) ? 1.0 - (1.0 - d) / (2.0 * s) : d / (2.0 * (1.0 - s));
}

vec3 vividLight( vec3 s, vec3 d )
{
   vec3 c;
   c.x = vividLight(s.x,d.x);
   c.y = vividLight(s.y,d.y);
   c.z = vividLight(s.z,d.z);
   return c;
}

vec3 linearLight( vec3 s, vec3 d )
{
   return 2.0 * s + d - 1.0;
}

float pinLight( float s, float d )
{
   return (2.0 * s - 1.0 > d) ? 2.0 * s - 1.0 : (s < 0.5 * d) ? 2.0 * s : d;
}

vec3 pinLight( vec3 s, vec3 d )
{
   vec3 c;
   c.x = pinLight(s.x,d.x);
   c.y = pinLight(s.y,d.y);
   c.z = pinLight(s.z,d.z);
   return c;
}

vec3 hardMix( vec3 s, vec3 d )
{
   return floor(s + d);
}

vec3 difference( vec3 s, vec3 d )
{
   return abs(d - s);
}

vec3 exclusion( vec3 s, vec3 d )
{
   return s + d - 2.0 * s * d;
}

vec3 subtract( vec3 s, vec3 d )
{
   return s - d;
}

vec3 divide( vec3 s, vec3 d )
{
   return s / d;
}

// rgb<-->hsv functions by Sam Hocevar
// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 rgb2hsv(vec3 c)
{
   vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
   vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
   vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
   
   float d = q.x - min(q.w, q.y);
   float e = 1.0e-10;
   return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
   vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
   vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
   return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 hue( vec3 s, vec3 d )
{
   d = rgb2hsv(d);
   d.x = rgb2hsv(s).x;
   return hsv2rgb(d);
}

vec3 color( vec3 s, vec3 d )
{
   s = rgb2hsv(s);
   s.z = rgb2hsv(d).z;
   return hsv2rgb(s);
}

vec3 saturation( vec3 s, vec3 d )
{
   d = rgb2hsv(d);
   d.y = rgb2hsv(s).y;
   return hsv2rgb(d);
}

vec3 luminosity( vec3 s, vec3 d )
{
   float dLum = dot(d, vec3(0.3, 0.59, 0.11));
   float sLum = dot(s, vec3(0.3, 0.59, 0.11));
   float lum = sLum - dLum;
   vec3 c = d + lum;
   float minC = min(min(c.x, c.y), c.z);
   float maxC = max(max(c.x, c.y), c.z);
   if(minC < 0.0) return sLum + ((c - sLum) * sLum) / (sLum - minC);
   else if(maxC > 1.0) return sLum + ((c - sLum) * (1.0 - sLum)) / (maxC - sLum);
   else return c;
}
// Blend functions end

vec3 mainFunction( vec2 uv )
{
   vec3 c = vec3(0.0);
   switch ( iBlendmode )
   {
   case 0: 
      c = mix( shaderLeft(uv), shaderRight(uv), iCrossfade );
      break;
   case 1: 
      c = multiply( shaderLeft(uv), shaderRight(uv) );
      break;
   case 2: 
      c = colorBurn( shaderLeft(uv), shaderRight(uv) );
      break;
   case 3: 
      c = linearBurn( shaderLeft(uv), shaderRight(uv) );
      break;
   case 4: 
      c = darkerColor( shaderLeft(uv), shaderRight(uv) );
      break;
   case 5: 
      c = lighten( shaderLeft(uv), shaderRight(uv) );
      break;
   case 6: 
      c = screen( shaderLeft(uv), shaderRight(uv) );
      break;
   case 7: 
      c = colorDodge( shaderLeft(uv), shaderRight(uv) );
      break;
   case 8: 
      c = linearDodge( shaderLeft(uv), shaderRight(uv) );
      break;
   case 9: 
      c = lighterColor( shaderLeft(uv), shaderRight(uv) );
      break;
   case 10: 
      c = overlay( shaderLeft(uv), shaderRight(uv) );
      break;
   case 11: 
      c = softLight( shaderLeft(uv), shaderRight(uv) );
      break;
   case 12: 
      c = hardLight( shaderLeft(uv), shaderRight(uv) );
      break;
   case 13: 
      c = vividLight( shaderLeft(uv), shaderRight(uv) );
      break;
   case 14: 
      c = linearLight( shaderLeft(uv), shaderRight(uv) );
      break;
   case 15: 
      c = pinLight( shaderLeft(uv), shaderRight(uv) );
      break;
   case 16: 
      c = hardMix( shaderLeft(uv), shaderRight(uv) );
      break;
   case 17: 
      c = difference( shaderLeft(uv), shaderRight(uv) );
      break;
   case 18: 
      c = exclusion( shaderLeft(uv), shaderRight(uv) );
      break;
   case 19: 
      c = subtract( shaderLeft(uv), shaderRight(uv) );
      break;
   case 20: 
      c = divide( shaderLeft(uv), shaderRight(uv) );
      break;
   case 21: 
      c = hue( shaderLeft(uv), shaderRight(uv) );
      break;
   case 22: 
      c = color( shaderLeft(uv), shaderRight(uv) );
      break;
   case 23: 
      c = saturation( shaderLeft(uv), shaderRight(uv) );
      break;
   case 24: 
      c = luminosity( shaderLeft(uv), shaderRight(uv) );
      break;
   case 25: 
      c = darken( shaderLeft(uv), shaderRight(uv) );
      break;
   case 26: 
      c = shaderLeft(uv);
      break;
   default: // in any other case.
      c = shaderRight(uv);
      break;
   }
   return c;
}
float BadTVResoRand(in float a, in float b) { return fract((cos(dot(vec2(a,b) ,vec2(12.9898,78.233))) * 43758.5453)); }

// main start
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv.x -= iRenderXY.x;
	uv.y -= iRenderXY.y;
	// flip horizontally
	if (iFlipH)
	{
		uv.x = 1.0 - uv.x;
	}
	// flip vertically
	if (iFlipV)
	{
		uv.y = 1.0 - uv.y;
	}
	// rotate
	//float rad = radians(360.0 * fract(iGlobalTime*iRotationSpeed));
	//mat2 rotate = mat2(cos(rad),sin(rad),-sin(rad),cos(rad));
	//uv = rotate * (uv - 0.5) + 0.5;

	// zoom centered
	float xZ = (uv.x - 0.5)*iZoom*2.0;
	float yZ = (uv.y - 0.5)*iZoom*2.0;
	vec2 cZ = vec2(xZ, yZ);

	// slitscan
	if (iRatio < 20.0)
	{
		float x = gl_FragCoord.x;
		float y = gl_FragCoord.y;
		float x2 = x;	  
		float y2 = y;
		if (iXorY)
		{
			float z1 = floor((x/iParam1) + 0.5);	   //((x/20.0) + 0.5)
			x2 = x + (sin(z1 + (iGlobalTime * 2.0)) * iRatio);
		}
		else
		{
			float z2 = floor((y/iParam2) + 0.5);	   //((x/20.0) + 0.5)
			y2 = y + (sin(z2 + (iGlobalTime * 2.0)) * iRatio);
		}

		vec2 uv2 = vec2(x2 / iResolution.x, y2/ iResolution.y);
		uv 	= texture2D( iChannel1, uv2 ).rg;
	}
	// glitch
	if (iGlitch == 1) 
	{
		// glitch the point around
		float s = iTempoTime * iRatio;//50.0;
		float te = iTempoTime * 9.0 / 16.0;//0.25 + (iTempoTime + 0.25) / 2.0 * 128.0 / 60.0;
		vec2 shk = (vec2(glitchNse(s), glitchNse(s + 11.0)) * 2.0 - 1.0) * exp(-5.0 * fract(te * 4.0)) * 0.1;
		uv += shk;		
	}
	// pixelate
	if ( iPixelate < 1.0 )
	{
		vec2 divs = vec2(iResolution.x * iPixelate / iResolution.y*60.0, iPixelate*60.0);
		uv = floor(uv * divs)/ divs;
	}

	vec3 col;
	if ( iCrossfade > 0.99 )
	{
		col = shaderRight(uv-cZ);
	}
	else
	{
		if ( iCrossfade < 0.01 )
		{
			col = shaderLeft(uv-cZ);
		}
		else
		{
			col = mainFunction( uv-cZ );

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
	col *= iExposure;
	if (iInvert == 1) col = 1.-col;
	if (iVignette == 1)
	{
		vec2 p = 1.0 + -2.0 * uv;
		col = mix( col, vec3( iBackgroundColor ), dot( p, p )*iRotationSpeed );
	}
	// badtv
	if (iBadTv > 0.0)
	{
		float c = 1.;
		if (iXorY)
		{
			c += iBadTv * sin(iGlobalTime * 2. + uv.y * 100. * iParam1);
			c += iBadTv * sin(iGlobalTime * 1. + uv.y * 80.);
			c += iBadTv * sin(iGlobalTime * 5. + uv.y * 900. * iParam2);
			c += 1. * cos(iGlobalTime + uv.x);
		}
		else
		{
			c += iBadTv * sin(iGlobalTime * 2. + uv.x * 100. * iParam1);
			c += iBadTv * sin(iGlobalTime * 1. + uv.x * 80.);
			c += iBadTv * sin(iGlobalTime * 5. + uv.x * 900. * iParam2);
			c += 1. * cos(iGlobalTime + uv.y);
		}	
	
		//vignetting
		c *= sin(uv.x*3.15);
		c *= sin(uv.y*3.);
		c *= .9;
	
		uv += iGlobalTime;
	
		float r = BadTVResoRand(uv.x, uv.y);
		float g = BadTVResoRand(uv.x * 9., uv.y * 9.);
		float b = BadTVResoRand(uv.x * 3., uv.y * 3.);
	
		col.x *= r*c*.35;
		col.y *= b*c*.95;
		col.z *= g*c*.35;
	}

	// grey scale mode
	if (iGreyScale == 1)
	{
	col = greyScale( col );
	}
	col.r *= iRedMultiplier;
	col.g *= iGreenMultiplier;
	col.b *= iBlueMultiplier;

	gl_FragColor = iAlpha * vec4( col, 1.0 );
}
// main end