// https://www.shadertoy.com/view/MdXGDr
// srtuss, 2013
// this pretty much was my first raymarching experience.
// big thanks to iq for his awesome artices!

float DataTransferSlices = cos(iGlobalTime * 0.8) * 0.3 + 0.4;

vec2 DataTransferrotate(vec2 k,float t)
{
	return vec2(cos(t) * k.x - sin(t) * k.y, sin(t) * k.x + cos(t) * k.y);
}

float DataTransferhexagon(vec3 p, vec2 h)
{
    vec3 q = abs(p);
    return max(q.z - h.y, max(q.x + q.y * 0.57735, q.y * 1.1547) - h.x);
}

float DataTransferscene(vec3 pi)
{
	// twist the wires
	float a = sin(pi.z * 0.3) * 2.0;
	vec3 pr = vec3(DataTransferrotate(pi.xy, a), pi.z);
	
	// move individual wires
	vec3 id = floor(pr);
	pr.z += sin(id.x * 10.0 + id.y * 20.0) * iGlobalTime * 2.0;
	
	// calculate distance
	vec3 p = fract(pr);
	p -= 0.5;
	
	// this makes hollow wires
	//return max(DataTransferhexagon(p, vec2(0.3, DataTransferSlices)), -DataTransferhexagon(p, vec2(0.2, DataTransferSlices + 0.05)));
	
	return DataTransferhexagon(p, vec2(0.3, DataTransferSlices));
}

vec3 DataTransfernorm(vec3 p)
{
	// the normal is simply the gradient of the volume
	vec4 dim = vec4(1, 1, 1, 0) * 0.01;
	vec3 n;
	n.x = DataTransferscene(p - dim.xww) - DataTransferscene(p + dim.xww);
	n.y = DataTransferscene(p - dim.wyw) - DataTransferscene(p + dim.wyw);
	n.z = DataTransferscene(p - dim.wwz) - DataTransferscene(p + dim.wwz);
	return normalize(n);
}
void main(void)
{
   vec2 uv = gl_FragCoord.xy / iResolution.xy;

	vec2 p = -1.0 + 2.0 * uv;
	vec3 dir = normalize(vec3(p * vec2(1.77, 1.0), 1.0));
	
	// camera
	dir.zx = DataTransferrotate(dir.zx, sin(iGlobalTime * 0.5) * 0.4);
	dir.xy = DataTransferrotate(dir.xy, iGlobalTime * 0.2);
	vec3 ray = vec3(0.0, 0.0, 0.0 - iGlobalTime * 0.9);
	
	// raymarching
	float t = 0.0;
	for(int i = 0; i < 90; i ++)
	{
		float k = DataTransferscene(ray + dir * t);
		t += k * 0.75;
	}
	vec3 hit = ray + dir * t;
	
	// fog
	float fogFact = clamp(exp(-distance(ray, hit) * 0.3), 0.0, 1.0);
	
	/*if(fogFact < 0.05)
	{
		//gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return vec3(0.0, 0.0, 0.0);
	}*/
	
	// diffuse & specular light
	vec3 sun = normalize(vec3(0.1, 1.0, 0.2));
	vec3 n = DataTransfernorm(hit);
	vec3 ref = reflect(normalize(hit - ray), n);
	float diff = dot(n, sun);
	float spec = pow(max(dot(ref, sun), 0.0), 32.0);
	//vec3 col = mix(vec3(0.0, 0.7, 0.9), vec3(0.0, 0.1, 0.2), diff);
	
	vec3 col = mix(iColor, iBackgroundColor, diff);

	// enviroment map
	//col += textureCube(iChannel0, ref).xyz * 0.2;
	col = fogFact * (col + spec);
	
	// iq's vignetting
	col *= 0.1 + 0.8 * pow(16.0 * uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y), 0.1);
	
  gl_FragColor = vec4(col,1.0);
}