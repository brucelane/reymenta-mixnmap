// https://www.shadertoy.com/view/XssGRj
//uncomment for a more intense color scheme suited for ourpithyator, etc
//comment for a calmer color scheme suited for 8 bit mentality, etc
#define ALT_MODE

//up to your taste. Might go well with ALT_MODE?
//If you comment gamma but leave contrast you'll get a solarized effect.
//#define POST_PROC

#define BEAT_POINT 0.3
// functions end
// https://www.shadertoy.com/view/XssGRj
void main(void)
{
   vec2 uv = iZoom * gl_TexCoord[0].st;
   uv.x -= iRenderXY.x;
   uv.y -= iRenderXY.y;
	vec3 final_color, bg_color, wave_color;
	//establish a reasonable beat estimation
	float beat = texture2D(iAudio0, vec2(BEAT_POINT, 0.0)).x;
	beat = beat * beat;
	//sun.x is desaturation. (-1.0 - 1.0) is good
	//sun.y is hue-related (keep around 0.2 - 1.0)
	//sun.z is contrast-related (keep around 0.7 - 1.0)
	//base values are -0.64, 0.96, 0.99
	vec3 sun = vec3(-0.64, 0.96, 0.99);
	
	//forget static shading. let's get dynamic!
	sun.x = -0.8 + beat*1.5;
	sun.y = 0.7 + 0.3 * cos(uv.x*6.0+20.0*(uv.y-0.5)*(beat-0.2) + iGlobalTime);
	sun.z = 0.9 + 0.1 * cos(iGlobalTime);

	//crazy intense dubstep-ize the colors
	#ifdef ALT_MODE
		sun.x = -cos(iGlobalTime);
		sun.y = 0.7 + 0.3 * cos(uv.x*6.0+20.0*(uv.y-0.5)*(beat-0.2) + iGlobalTime);
		sun.z = 0.85 + 0.35 * cos(iGlobalTime);
	#endif

	sun.xy = 0.5 + 0.5 * sun.xy;
	sun = normalize(sun);
	
	//waveform at x-coord
	float hwf = texture2D(iAudio0, vec2(uv.x,1.0)).x;
	float vwf = texture2D(iAudio0, vec2(uv.y,1.0)).x;
	
	#ifdef ALT_MODE
	float noise = texture2D(iChannel0, vec2(vwf,1.0)).x;
	float distort = (vwf - 0.5) * beat * beat * beat;
	
	//distort and wrap, triangle mod
	uv.x = abs(uv.x + distort);
	if (uv.x > 1.0) uv.x = 2.0 - uv.x;
	#endif
	
	//four distortion patterns
	float coord1 = sin(uv.x + hwf) - cos(uv.y);
	float coord2 = sin(1.0 - uv.x + hwf) - sin(uv.y * 3.0);
	//float coord3 = cos(uv.x+sin(0.2*iGlobalTime)) - sin(2.5*uv.y - wf);
	float coord4 = mix(coord1, coord2,hwf);
	float coord5 = mix(coord1, coord2, beat);
	
	float coorda = mix(coord1, coord2, 0.5 + 0.5 *sin(iGlobalTime*0.7));
	float coordb = mix(coord5, coord4, 0.5 + 0.5 *sin(iGlobalTime*0.5));
	
	//mix distortions based on time
	float coord = mix(coorda, coordb, 0.5 + 0.5 *sin(iGlobalTime*0.3));

	//distort the spectrum image using the above coords
	float j = (texture2D(iAudio0, vec2(abs(coord),0.0)).x - 0.5) * 2.0;

	//greyscale -> color with a lot of dot products
	float fac1 = dot(vec3(uv.x,uv.y,j),sun);
	float fac2 = dot(vec3(uv.y,j,uv.x),sun);
	float fac3 = dot(vec3(j,uv.x,uv.y),sun);
	bg_color = vec3(fac1,fac2,fac3);

	//extract some info about wave shape
	float w1 = texture2D(iAudio0, vec2(0.1,0.0)).x;
	float w2 = texture2D(iAudio0, vec2(0.2,0.0)).x;
	float w3 = texture2D(iAudio0, vec2(0.4,0.0)).x;
	float w4 = texture2D(iAudio0, vec2(0.8,0.0)).x;
	
	//save screenspace for later
	vec2 ouv = uv;
	
	float wave_width = 0.0;
	float wave_amp = 0.0;
	
	//rescale to aspect-aware [-1, 1]
	uv  = -1.0 + 2.0 * uv;
	uv.y *= iResolution.y/iResolution.x;
	
	#ifdef ALT_MODE
	//bend-spin-stretch the coord system based on wave shape
	float theta = (w1 - w2 + w3 - w4) * 3.14;
	uv = uv*mat2(cos(theta * 0.2),-sin(theta * 0.3),sin(theta * 0.5),cos(theta * 0.7));
	#endif
	
	//for polar calculations
	float rad = length(uv) - 0.3;
	
	float h;
	
	//slightly based on Waves by bonniem 
	for(float i = 0.0; i < 10.0; i++) {
		//build a simple trig series based on wave shape
		//note that waves are in rectangular coords
		h = sin(uv.x*8.0+iGlobalTime)*0.2*w1+ 
			sin(uv.x*16.0-2.0*iGlobalTime)*0.2*w2 + 
			sin(uv.x*32.0+3.0*iGlobalTime)*0.2*w3 + 
			sin(uv.x*64.0-4.0*iGlobalTime)*0.2*w4;
		rad += h * 0.4;
		//find the wave section's width in polar coords
		wave_width = abs(1.0 / (100.0 * rad));
		wave_amp += wave_width;
	}
	
	//color waves based on sonar shape and screen position
	wave_color = vec3(0.2 + w2*1.5, 0.1 + w4*2.0 + uv.y*0.1, 0.1 + w3*2.0+uv.x*0.1);
	
	#ifdef ALT_MODE
	//fade with beat
	wave_amp *= beat * 3.0;
	#endif
	
	//modulate with waveform. doesn't do much.
	wave_amp *= hwf*2.0;
	
	
	//fade bg in from 0 to 5 sec
	final_color = bg_color*smoothstep(0.0,5.0,iChannelTime[0]);
	final_color += 0.12*wave_color*wave_amp;
	
	//postproc code from iq :)
	#ifdef POST_PROC
	 //gamma. remove this for a solarized look	
	final_color = pow( clamp( final_color, 0.0, 1.0 ), vec3(1.7) );

    //contrast
	final_color = final_color*0.3 + 0.7*final_color*final_color*(3.0-2.0*final_color);
	#endif
	
	//vignette
	final_color *= 0.5 + 0.5*pow( 16.0*ouv.x*ouv.y*(1.0-ouv.x)*(1.0-ouv.y), 0.2);

	#ifdef ALT_MODE
//	final_color = mix(final_color, vec3(noise), distort * 10.0);
	final_color += vec3(noise) * distort * 10.0;
	#endif
   gl_FragColor = vec4(final_color,1.0);
}
