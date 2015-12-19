// https://www.shadertoy.com/view/ld2Gzz

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float t = iGlobalTime * 5.0;	
	vec2 p = 2.0 * uv - 1.0;
	vec2 op = p;
	p.x *= iResolution.x/iResolution.y;
	vec3 color = vec3(0.0);
	float v = length(2.0*uv-1.0);
	
	p *= 10.0;
	
	float movex = sin(t+p.x+p.y*v);
	float movey = sin(t*0.5);
	float grid = sin(p.x+movex)*sin(p.y+movey);
	float grid2 = grid + 0.2;
	float grid3 = grid + 0.3;
	
	float inner = 1.0-ceil(grid+grid2);
	float outer = 1.0-ceil(grid+grid3);
	
	inner = max(0.0,min(1.0,inner));
	outer = max(0.0,min(1.0,outer));
	float stencil = inner-outer;
	
	color.r = inner - outer;
	p *= 0.5;
	p = abs(p);
	p *= v;
	p.x += sin(p.y*sin(p.x));
	float beam = sin(p.x+t);
	beam = max(0.0,min(1.0,beam));
	
	color.r += beam;
	color.r *= beam*grid;
	color *= 2.0;
	color.r += stencil*beam;
	color.r = max(0.0,min(1.0,color.r));
	color.g = color.r*0.5;
	
	
	// high constrast and vinj
	color *= color * 10.0;
	color *= 1.0-v;
	
	//output final color
	gl_FragColor = vec4(color,1.0);
}