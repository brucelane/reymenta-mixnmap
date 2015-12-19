// functions begin
// https://www.shadertoy.com/view/lsS3DK

float Wave00100100(float x, float s) {
	return sin(x + mod(iGlobalTime * s, 3.1415 * 2.))/2. + .5;
}
// https://www.shadertoy.com/view/lsS3DK
void main(void)
{
	vec2 uv = iZoom * gl_FragCoord.xy / iResolution.xy;
	uv.x -= iRenderXY.x;
	uv.y -= iRenderXY.y;
	uv.x -= 0.5;
	uv.y -= 0.5;
	uv.x *= 2.;
	uv.y *= 2.;
	
	float t = mod(iGlobalTime* 2., 3.1415 * 2.);
	float a = 0.;
	for(float i = 1.; i <= 3.; ++i) {	
		a += sin(t * (i * 2. - 1.)) / (i * 2. - 1.);
	}
	a = a * 1.15 / 2. + .5;
	
	float power = 2. / (1. - min(a, .98));
	float x = pow((1.+2.*a)*abs(uv.x), power);
	float y = pow(abs(uv.y), power);
	float r = iResolution.y / 2.;
	float v = pow(x+y, 1./power);
	float l = (1. - v) * r;
	float l2 = (r/2. - l*(1.-a));
	float s = clamp(min(l, l2), 0., 1.);
	vec3 col = vec3(1.,1.,1.); // vec4(Wave00100100(uv.x,3.), Wave00100100(uv.y,5.), Wave00100100(uv.x*uv.y,7.),1.0);
	gl_FragColor = vec4(col * s,1.0);
}
