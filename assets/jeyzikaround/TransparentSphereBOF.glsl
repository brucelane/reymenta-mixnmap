// https://www.shadertoy.com/view/4dj3RG
#ifdef GL_ES
precision highp float;
#endif

#define FULLBODY

#define DELTA 0.02
#define MAXDISTANCE 2.0
#define RAYITERATIONS 100

#define CASTITERATIONS 20
#define CASTDELTA 0.01
#define CASTDISTANCE 20.0

#define SPHERERADIUS 1.0

//uniform float time;
//uniform vec2 mouse;
//uniform vec2 resolution;

#define time iGlobalTime
#define resolution iResolution

vec4 getAmbientSample( vec3 p )
{
	return vec4( 0.0, 0.05, 0.1, 0.010 );
}

float getSphereDistance( vec3 p, float r )
{
	return max( 0.0, length(p)-r );
//	return length(p)-r;
}
	
vec4 getSphereSample( vec3 p, float r )
{
	if( getSphereDistance(p,r)<=0.0 )
	{
		p = normalize(p);
		p.x = sin(p.x*10.0 + 1.0*time)/3.0;
		p.y = cos(p.y*11.0 + 2.0*time)/3.0;
		p.z = sin(p.z*12.0 + 1.0*time)/3.0;
		
		//return vec4(vec3(p), 0.015);
		return vec4( 5.0*p.z*p.y,3.0*abs(p.x),0.9 , abs(p.x+p.y+p.z)/20.0 );
	}
	else
	{
		return getAmbientSample(p);
	}
}

vec3 castRay( vec3 emitterP, vec3 rayV )
{
	float t = 0.0;
	vec3 rayP = emitterP;

	for(int i=0;i<CASTITERATIONS;i++)
	{
        	float h = getSphereDistance(rayP, SPHERERADIUS);
        	if( h<CASTDELTA )
			break;
		if( h>CASTDISTANCE )
			return emitterP + rayV*CASTDISTANCE; 
		rayP += rayV*h;
	}
	return rayP;
}

vec3 getWorldC( vec3 emitterP, vec3 rayV )
{
	vec3 sampleP = castRay( emitterP, rayV );
	//vec3 sampleP = emitterP + rayV;
	
	vec4 accumulatedS = getAmbientSample( MAXDISTANCE*rayV );

	#ifdef FULLBODY
	for( float depth = 0.0; depth <= MAXDISTANCE; depth += DELTA )
	#else
	for( int i = 0; i < RAYITERATIONS; i++ )
	#endif
	{
		//if( accumulatedS.a >= 1.0 )
		//	break;
		vec4 sampleS = getSphereSample( sampleP, SPHERERADIUS );
		//accumulatedS = vec4( vec3(mix(sampleS.rgb, accumulatedS.rgb,  accumulatedS.a)),
		//		     accumulatedS.a + sampleS.a); 
		accumulatedS = vec4( vec3(mix(accumulatedS.rgb, sampleS.rgb,  sampleS.a)),
				     accumulatedS.a + sampleS.a); 
		sampleP += DELTA*rayV;
	}
	return vec3( accumulatedS.rgb );
}

void main( void )
{
	vec2 scrP    = 2.0*gl_FragCoord.xy/resolution.xy - 1.0;
	     scrP.x *= resolution.x/resolution.y;	
	
	vec3 camP      = vec3(-2.0, 0.0, 0.0 );
	vec3 trgP      = vec3( 0.0, 0.0, 0.0 );
	vec3 upV       = vec3( 0.0, 1.0, 0.0 );
	vec3 camV      = normalize( trgP - camP );
	vec3 camRightV = cross( upV, camV );
	vec3 camUpV    = cross( camV, camRightV );
	
	vec3 lightP = vec3(-5.0, 5.0, 2.0 );
	vec3 lightV = normalize(lightP);
	vec3 lightC = vec3( 1.0, 0.9, 0.5 );
	
	vec3 rayV = normalize( camV + camRightV*scrP.x + camUpV*scrP.y );
	
	vec3 color = getWorldC( camP, rayV );
	//vec3 color = castRay(camP,rayV);
	
	gl_FragColor = vec4( color, 1.0 );

}
/*
	vec2 uv = iZoom * gl_TexCoord[0].st;
	vec2 uv = 2.0 * iZoom * (gl_TexCoord[0].st- 0.5);
	uv.x *= float(iResolution.x )/ float(iResolution.y);
	uv.x -= iRenderXY.x;
	uv.y -= iRenderXY.y;
*/
