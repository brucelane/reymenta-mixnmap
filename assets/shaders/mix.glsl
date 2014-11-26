
out vec4 oColor;

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
/*
void main(void)
{
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
   vec4 left = texture2D(iChannel0, uv);
    
   oColor = vec4( left.r, left.g, 1.0 * sin(iGlobalTime), 1.0 );
   
}
*/
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
	oColor = iAlpha * vec4( col, 1.0 );
}