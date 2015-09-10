void main(void)
{
   vec2 uv = iZoom * gl_TexCoord[0].st;
   uv.x -= iRenderXY.x;
   uv.y -= iRenderXY.y;
	float u = gl_FragCoord.x / iResolution.x;
	float fft = texture2D(iAudio0, vec2(u,.25)).x;  
	float wav = texture2D(iAudio0, vec2(u,.75)).x;
	
	uv = uv * 2. - 1.;
	vec2 wv = uv + vec2(0., wav - .5);

	float f = pow(abs(fft * tan(iGlobalTime - uv.y / wv.y)), .1);
	float h = pow(abs(wv.x - pow(abs(uv.y), cos(fft * 3.14159 * .25))), f);
	float g = abs(iRatio / 40.0 * .02 / (sin(wv.y) * h));

	vec3 c = g * clamp(vec3(fft, fract(fft) / fract(wav), g * wav), 0., 1.);
	
  gl_FragColor = vec4(c,1.0);
}
