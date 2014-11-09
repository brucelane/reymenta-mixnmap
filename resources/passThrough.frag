#version 110
uniform vec3			iResolution;           // viewport resolution (in pixels)
uniform sampler2D		iChannel0;
uniform float			iGlobalTime;           // shader playback time (in seconds)

// main
void main()
{
	// pass through
	vec2 uv = gl_TexCoord[0].st * vec2(iResolution.x,iResolution.y);
    gl_FragColor = texture2D(iChannel0, uv);
	//gl_FragColor.a = 1.0;
}