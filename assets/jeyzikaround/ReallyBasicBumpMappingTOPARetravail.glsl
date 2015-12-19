// https://www.shadertoy.com/view/Mdl3WH



void main(void)
{

	vec2 lightPos = vec2(
		(1.2 + sin(iGlobalTime)) * 0.4 * iResolution.x,
		(1.2 + cos(iGlobalTime)) * 0.4 * iResolution.y
	);
	
	// Uncomment this line to control with the mouse.
	//lightPos = iMouse.xy;
	
	float bumpStrength = 16.0*iSteps;
	float bumpRadius1 = 0.1; // anything closer than this to the light has no bump effect
	float bumpRadius2 = 0.25; // anything beyond this has full bump effect
	float lightStrength = 1.6;
	float lightRadius = 0.7;
	float refDist = 1.2 * iResolution.x / 500.0;
		
	
	vec2 vecToLight = gl_FragCoord.xy - lightPos;
	float distToLight = length(vecToLight.xy / iResolution.xy);
	vec2 dirToLight = normalize(vecToLight);
	//vec2 curPos = gl_FragCoord.xy / iResolution.xy;
	vec2 curPos = iZoom * gl_TexCoord[0].st;//* vec2(1.0,1.0);
	vec2 refPos = curPos.xy - (refDist * dirToLight.xy / iResolution.xy);
	
	vec4 curSample = texture2D(iChannel1, curPos);
	vec4 refSample = texture2D(iChannel1, refPos);
	
	float curLumin = (curSample.x + curSample.y + curSample.z) * 0.33;
	float refLumin = (refSample.x + refSample.y + refSample.z) * 0.33;
	
	// Attenaute brightness based on how much this fragment seems to face the light
	float directionBrightness =
		lightStrength - // start at maximum brightness
		(
			step(bumpRadius1, distToLight) * // leave a bump-free area round the light
			clamp((distToLight - bumpRadius1) / (bumpRadius2 - bumpRadius1), 0.0, 1.0) *
			step(curLumin, refLumin) * // only attenuate if facing away from light source
			clamp((refLumin - curLumin) * bumpStrength, 0.0, 1.0)
		);
	
	// Attenuate brightness based on distance from the light
	float distanceBrightness = 1.0 - (distToLight / lightRadius);
	
	gl_FragColor.xyz = directionBrightness * distanceBrightness * curSample.xyz;
	gl_FragColor.w = curSample.w;
}

	