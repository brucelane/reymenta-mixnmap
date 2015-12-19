
// noise & fbm by iq

float hash( float n )
{
    return fract( sin(n)* 43758.5453123 );
}

float noise1( float x )
{
    float p = floor(x);
    float f = fract(x);

    //f = f*f*(3.0-2.0*f);
     f = f*f*( ( iZoom/1.0 ) - iRatio/50.0 * f );

    return mix( hash(p+0.0), hash(p+1.0), f );
}

float fbm( float p )
{
    float f = 0.0;

    f += 0.5000*noise1( p ); p = p*2.02;
    f += 0.2500*noise1( p ); p = p*2.03;
    f += 0.1250*noise1( p ); p = p*2.01;
    f += 0.0625*noise1( p );

    return f/0.9375;
}

void main(void)
{

	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	float c = dot( vec3( fbm( uv.x * 5.134 + iGlobalTime * 2.013 ),
			             fbm( uv.x * 15.002 + iGlobalTime * 3.591 ),
						 fbm( uv.x * 25.922 + iGlobalTime * 4.277 ) ),
				   vec3( .85, .35, .17 ) );

	gl_FragColor = vec4( c*iColor.r, c*iColor.g, c*iColor.b, 1.);

}