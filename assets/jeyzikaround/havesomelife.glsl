// https://www.shadertoy.com/view/XslGR7
float density( vec3 p, float distortion )
{
	vec3 tiled = ( ( p + vec3( 2.0 ) ) * 0.25 );
	tiled = fract( tiled );
	tiled = 4.0 * tiled - vec3( 2.0 );
	float variation = distortion * ( 
		sin( 24.0 * atan( tiled.x / tiled.y ) ) + 
		sin( 24.0 * atan( tiled.y / tiled.z ) ) + 
		sin( 24.0 * atan( tiled.z / tiled.x ) ) );
	return 1.0 + variation - length( tiled );
}
vec2 rotate( vec2 inVec, float angle )
{
	vec2 sincos = vec2( sin(angle), cos(angle) );
	return vec2( inVec.x * sincos.y - inVec.y * sincos.x, 
				inVec.x * sincos.x + inVec.y * sincos.y );
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy - vec2( .5, .5 );
	uv *= vec2( 1.0, .6 );
	vec3 dir = normalize( vec3( uv.x, uv.y, -1.0 ) );
	
	float progress = .03 * iGlobalTime;
	
	dir.yz = rotate( dir.yz, .3 * sin( .5 * progress ) );
	dir.zx = rotate( dir.zx, .7 * progress );
	
	float mask = .5 + .5 * sin( length( sin( uv * 7.0) + vec2( 3.0 * progress, progress ) ) * 6.28  + 10.0 * progress );
	mask = pow( mask, 6.0)* .4 +.3;
	float x = progress * 4.0;
	float z = 10.0 * sin( .3 * progress );
	float y = 2.0 + 1.0 * sin( 2.0 * progress );
	
	
	vec3 pos = vec3(x,y,z);
	
	
	float stepLen = 0.3;
	vec3 step = dir * stepLen;
	float acumulate = .0;
	// slow float bluredAcumulate = .0;
	float dist = .0;
	
	/* slow for( int i = 0; i < 100; ++i )
	{
		pos += step;
		if( density(pos, .05) > .0 )
		{
			bluredAcumulate = (50.0 - dist ) * .02;
			break;
		}
		dist += stepLen;
	}*/
	dist = .0;
	pos = vec3(x,y,z);	
	for( int i = 0; i < 100; ++i )
	{
		pos += step;
		if( density(pos, .2) > .0 )
		{
			acumulate = (50.0 - dist ) * .02;
			break;
		}
		dist += stepLen;
	}
	
	acumulate = pow(acumulate, 2.5);
	// slow bluredAcumulate = pow(bluredAcumulate, 2.5);
	vec3 colorHard =( 1.0 - mask )* vec3( 1.2, 1.6, 0.6 ) * ( 1.0 - acumulate);
	// slow vec3 colorChill = mask * vec3( 0.6, 1.5, 1.5) * ( 1.0 - bluredAcumulate);
	// slow gl_FragColor = vec4( colorChill + colorHard, 1.0 );
	gl_FragColor = vec4( colorHard, 1.0 );
}