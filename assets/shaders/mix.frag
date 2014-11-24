// uniforms begin
#version 120
uniform vec3        iResolution;         	// viewport resolution (in pixels)
uniform sampler2D   iChannel0;				// input channel 0
uniform sampler2D   iChannel1;				// input channel 1 
uniform float       iCrossfade;          	// CrossFade 2 shaders
uniform float       iAlpha;          	  	// alpha

vec3 shaderLeft(vec2 uv)
{
	vec4 left = texture2D(iChannel0, uv);
	return vec3( left.r, left.g, left.b );
}

vec3 shaderRight(vec2 uv)
{
	vec4 right = texture2D(iChannel1, uv);
	return vec3( right.r, right.g, right.b );
}

vec3 mainFunction( vec2 uv )
{
   return mix( shaderLeft(uv), shaderRight(uv), iCrossfade );
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
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
	gl_FragColor = iAlpha * vec4( col, 1.0 );
}
// main end