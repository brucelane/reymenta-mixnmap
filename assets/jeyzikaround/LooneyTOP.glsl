// https://www.shadertoy.com/view/XssXz2

//Looney by nimitz (stormoid.com) (twitter: @stormoid)

#define NUM_OBJ 35.
#define SPEED 5.

//undefine for a more standard "tunnel"
#define ACID

#define time iGlobalTime

mat2 mm2(in float theta){float c = cos(theta);float s = sin(theta);return mat2(c,-s,s,c);}

//2d primitive
float prim(vec2 p, float d, float n) 
{
	p.x += cos(p.y*4.5)*0.04;
	p.y += sin(p.x*4.5)*0.04;
	#ifdef ACID
	p.x += n*0.01+sin(time*0.3)*0.5;
	p.y += n*0.01+sin(time*0.3)*0.5;
	#endif
	
	vec2 q = abs(p);
	float tri = max(q.x*0.866025+p.y*.5, -p.y)-d;
	float sq = max(abs(p.x), abs(p.y)) - d;
	return mix(tri,sq,sin(time*0.27)*.75+.25);
}

float f (const in float x)
{
	return mod(time*SPEED-x,NUM_OBJ);
}

vec4 draw(in vec2 p, in float n, in float x)
{
	float size = exp(x*.15)*.02;
	#ifdef ACID
	float rot = n*0.2; 
	#else
	float rot = x*0.1;
	#endif
	float t2 = log(size)*sin(time*0.3)*.5;
	
	
	float a = smoothstep(0.0, -.04, prim(p*mm2(-rot * t2), size,x));
	//vec3 col2 = (sin(vec3(.98,0.5,.85)*x*.5)*0.1+.9) * a;
	vec3 col2 = (sin(vec3(.45,0.42+sin(time*0.12-2.5)*0.05,.39)*x*.5)*.1+.9)*a;
	float ofst = mix(0.25, 0.5, n/NUM_OBJ)*1.1;
	float b = smoothstep(0., -.02, prim(p*mm2(-rot*t2), size-(ofst),x ));
	col2 = mix(col2, vec3(.0), b);
	return vec4(col2,a);
}


void main(void)
{
	vec2 p = (gl_FragCoord.xy/iResolution.xy-0.5)*2.;
	p.x *= iResolution.x / iResolution.y;
	p *= mm2(time*0.01);
	
	vec3 col = vec3(0.);
	for (float i = NUM_OBJ; i > 0.; i--)
	{
		float num = floor(f(i));
		float x = f(num);
		vec4 col2 = draw(p,num,x);
		
		col = mix(col,col2.rgb,col2.a*.9);
	}
	
	col = pow(abs(col),vec3(18.))*5.;
	gl_FragColor = vec4(col, 1.0);
}