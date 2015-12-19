// https://www.shadertoy.com/view/MdSGzG
//Thanks iq for the RippledDarknessHash functions

float RippledDarknessHash( float n )
{
    return fract(sin(n)*43758.5453);
}

vec2 RippledDarknessHash2( vec2 p )
{
    p = vec2( dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)) );
	return fract(sin(p)*43758.5453);
}

void main(void)
{
	float aspect = iResolution.y / iResolution.x;
	vec2 uv = iZoom * gl_FragCoord.xy / iResolution.xy; 
	uv.x /= aspect;
	uv.x -= iRenderXY.x;
	uv.y -= iRenderXY.y;
	float mdist = 1000.0;
	for (int x = 0; x < 5; x++)
	{
		for (int y = 0; y < 5; y++)
		{
			vec2 p = RippledDarknessHash2(vec2(x, y)) / vec2(aspect, 1);
			
			if (distance(p, uv) < mdist)
			{
				mdist = distance(p, uv) * cos(uv.x + iGlobalTime* ( iRotationSpeed + 1.01 ) * 3.0 ) * sin(uv.x + iGlobalTime);
				gl_FragColor = vec4(RippledDarknessHash(float(x) + float(y))) * cos(400.0 * mdist);
			}
		}
	}
}


