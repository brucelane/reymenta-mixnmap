#define TRAN0 vec4(0.0, 0.0, 0.0, 0.0)
const float TIME_TRAN = 0.4;	// Transition time
const float TIME_INTR = 0.1;	// Intermission between in/out
const float TIME_PADN = 0.1;	// Padding time at the end of out.
const float TIME_TOTAL = (2.0 * TIME_TRAN) + TIME_INTR + TIME_PADN;

// https://www.shadertoy.com/view/Xsf3RN
float NVRadius = 0.4;
float NVBeat = 0.;

//utility
float NVRemap(float value, float inputMin, float inputMax, float outputMin, float outputMax)
{
    return (value - inputMin) * ((outputMax - outputMin) / (inputMax - inputMin)) + outputMin;
}
float NVRand(vec2 n, float time)
{
  return 0.5 + 0.5 * 
     fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453 + time);
}

struct NVCircle
{
	vec2 center;
	float radius;
};
	
vec4 NVCircleMaskColor(NVCircle circle, vec2 position)
{
	float d = distance(circle.center, position);
	if(d > circle.radius)
	{
		return vec4(0.0, 0.0, 0.0, 1.0);
	}
	
	float distanceFromCircle = circle.radius - d;
	float intencity = smoothstep(
								    0.0, 1.0, 
								    clamp(
									    NVRemap(distanceFromCircle, 0.0, 0.1, 0.0, 1.0),
									    0.0,
									    1.0
								    )
								);
	return vec4(intencity, intencity, intencity, 1.0);
}

vec4 NVCircleMaskBlend(vec4 a, vec4 b)
{
	vec4 one = vec4(1.0, 1.0, 1.0, 1.0);
	return one - (one - a) * (one - b);
}

float NVF1(float x)
{
	return -4.0 * pow(x - 0.5, 2.0) + 1.0;
}
	
void main(void)
{
	//OK but flipped vec2 uv = gl_FragCoord.xy / iResolution.xy;
	// OK not flipped but scaled vec2 uv = (gl_FragCoord.xy / iResolution.xy)*-2.0+2.0;
	//OK!
	vec2 uv = (gl_FragCoord.xy / iResolution.xy)*-1.0+1.0;
	
	float wide = iResolution.x / iResolution.y;
	float high = 1.0;
	
	vec2 position = vec2(uv.x * wide, uv.y);
	
	NVCircle circle_a = NVCircle(vec2(NVRadius, NVRadius+0.1), 0.43);
	NVCircle circle_b = NVCircle(vec2(wide - NVRadius, NVRadius+0.1), 0.43);
	vec4 mask_a = NVCircleMaskColor(circle_a, position);
	vec4 mask_b = NVCircleMaskColor(circle_b, position);
	vec4 mask = NVCircleMaskBlend(mask_a, mask_b);
	
	float greenness = 0.4;
	vec4 coloring = vec4(1.0 - greenness, 1.0, 1.0 - greenness, 1.0);
	
	float noise = NVRand(uv * vec2(0.1, 1.0), iGlobalTime * 5.0);
	float noiseColor = 1.0 - (1.0 - noise) * 0.3;
	vec4 noising = vec4(noiseColor, noiseColor, noiseColor, 1.0);
	
	float warpLine = fract(-iGlobalTime * 0.5);
	
	float warpLen = 0.1;
	float warpArg01 = NVRemap(clamp((position.y - warpLine) - warpLen * 0.5, 0.0, warpLen), 0.0, warpLen, 0.0, 1.0);
	float offset = sin(warpArg01 * 10.0)  * NVF1(warpArg01);
	
	
	vec4 lineNoise = vec4(1.0, 1.0, 1.0, 1.0);
	if(abs(uv.y - fract(-iGlobalTime * 19.0)) < 0.0005)
	{
		lineNoise = vec4(0.5, 0.5, 0.5, 1.0);
	}

	vec4 base = texture2D(iChannel0, uv + vec2(offset * 0.02, 0.0));

	vec4 col = base * mask * coloring * noising * lineNoise;
	if (iFade == 1) 
	{
		float t = iGlobalTime;
		float r = TIME_TOTAL - TIME_PADN - t;
		col = mix(TRAN0, col, smoothstep(0.0, TIME_TRAN, r));		
	}
	gl_FragColor=col;

}