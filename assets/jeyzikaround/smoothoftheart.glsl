// https://www.shadertoy.com/view/Msl3zN

void main(void)
{
	float i, j;
	vec2 circ1, circ2;
	
	circ1.x = gl_FragCoord.x-((sin(iGlobalTime)*iResolution.x)/4.0 + iResolution.x/2.0);
	circ1.y = gl_FragCoord.y-((cos(iGlobalTime)*iResolution.x)/4.0 + iResolution.y/2.0);

	circ2.x = gl_FragCoord.x-((sin(iGlobalTime*0.92+1.2)*iResolution.x)/4.0 + iResolution.x/2.0);
	circ2.y = gl_FragCoord.y-((cos(iGlobalTime*0.43+0.3)*iResolution.x)/4.0 + iResolution.y/2.0);
	
	circ1.xy /= 4.0;
	circ2.xy /= 4.0;
	
	i = sin(sqrt(circ1.x*circ1.x+circ1.y*circ1.y))*0.5+0.5;
	j = sin(sqrt(circ2.x*circ2.x+circ2.y*circ2.y))*0.5+0.5;

	gl_FragColor = vec4(j*1.5,i*1.5,(j+i)/4.0,1.0);
}