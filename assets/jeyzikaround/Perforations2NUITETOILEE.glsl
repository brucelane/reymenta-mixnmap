// https://www.shadertoy.com/view/MdSGDd

#define TAU 6.283185307179586

float scale(float l0, float r0, float l1, float r1, float x) {
	return (x - l0) / (r0 - l0) * (r1 - l1) + l1;
}

vec2 polar(vec2 p) {
	return vec2(atan(p.y, p.x), length(p));
}

vec2 cartesian(vec2 p) {
	return vec2(p.y * cos(p.x), p.y * sin(p.x))	;
}

vec2 rotate(float theta, vec2 p) {
	vec2 p_polar = polar(p);
	p_polar.x += theta;
	return cartesian(p_polar);
}

vec2 shear(float theta, vec2 p) {
	return vec2(p.x - p.y / tan(theta), p.y / sin(theta));
}

vec2 unshear(float theta, vec2 p) {
	float y = p.y * sin(theta);
	float x = p.x + y / tan(theta);
	return vec2(x, y);	
}

vec2 target(float theta, float delta, vec2 p) {
	return unshear(theta, floor(shear(theta, p) + delta) - delta + 0.5);
}

float perforations(float theta, float rot, float scale, float r, vec2 p0) {
	vec2 p1 = scale * rotate(rot, p0);
	return distance(p1, target(theta, 0.5, p1)) - r;
}

vec3 blend(float k, vec3 c0, vec3 c1) {
	float k_clamp = clamp(k, 0.0, 1.0);
	return (1.0 - k) * c0 + k * c1;
}

void main(void) {

	vec2 p0 = iZoom * 2.0*(gl_FragCoord.xy - 0.5 * iResolution.xy) / iResolution.xx;
	p0.x -= iRenderXY.x;
	p0.y -= iRenderXY.y;
	vec2 p0_1 = vec2(p0.x, p0.y);
	vec2 p0_2 = vec2(p0.x, p0.y);
	
	vec2 p1_polar = polar(p0);
	p1_polar.y = p1_polar.y * pow(cos(0.6 * length(p0)), 1.5);
	vec2 p1 = cartesian(p1_polar);
	
	// mess with this for all sorts of fun patterns
	float theta = TAU / 6.0;
	
	float rot1 = 0.004 * TAU * iGlobalTime;
	float rot2 = rot1 + TAU / 12.0 + TAU / 13.0 * sin(0.2 * iGlobalTime);
	
	float scale1 = 16.0;
	float scale2 = 16.0;
	
	float r1 = 0.25;
	float r2 = 0.25;
	
	float i1 = perforations(theta, rot1, scale1, r1, p1);
	float i2 = perforations(theta, rot2, scale2, r2, p1);
	
	vec3 bg = blend(length(p0), iBackgroundColor, vec3(0.0, 0.0, 0.3));
	vec3 fg = blend(length(p0), iColor, vec3(0.5, 0.0, 1.0));
	
	float satan = 0.03;
	float k = scale(-satan, satan, 0.0, 1.0, max(i1, i2));
	
	vec3 color = bg;
	if(k < 0.0) {
		color = fg;
	} else if(k < 1.0) {
		color = blend(0.5, fg, bg);	
	}
	
	
	gl_FragColor = vec4(color, 1.0);
}
