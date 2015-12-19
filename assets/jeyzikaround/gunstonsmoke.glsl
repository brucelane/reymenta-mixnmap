// https://www.shadertoy.com/view/XsX3D4
#define numLegs 3.0	//non integers look terrible
#define wibblewobble 6.5

void main(void) {

	vec2 p = -1.0+2.0*gl_FragCoord.xy/iResolution.xy;
	
	float w = sin(iGlobalTime+wibblewobble*sqrt(dot(p,p))*cos(p.x)); 	//part 2
	float x = cos(numLegs*atan(p.y,p.x) + 1.8*w);	//part 1
	
	//vec3 col = vec3(0.1,0.2,0.82)*15.0;
	vec3 col = iBackgroundColor*15.0;

	gl_FragColor = vec4(col*x,1.0);
	
}