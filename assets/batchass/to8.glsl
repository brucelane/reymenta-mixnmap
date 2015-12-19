// https://www.shadertoy.com/view/Xss3zr

// @simesgreen

const int VideoHeightFieldSteps = 64;
const vec3 VideoHeightFieldLightDir = vec3(0.577, 0.577, 0.577);

// transforms
vec3 VideoHeightFieldRotateX(vec3 p, float a)
{
    float sa = sin(a);
    float ca = cos(a);
    vec3 r;
    r.x = p.x;
    r.y = ca*p.y - sa*p.z;
    r.z = sa*p.y + ca*p.z;
    return r;
}

vec3 VideoHeightFieldRotateY(vec3 p, float a)
{
    float sa = sin(a);
    float ca = cos(a);
    vec3 r;
    r.x = ca*p.x + sa*p.z;
    r.y = p.y;
    r.z = -sa*p.x + ca*p.z;
    return r;
}

bool VideoHeightFieldIntersectBox(vec3 ro, vec3 rd, vec3 boxmin, vec3 boxmax, out float tnear, out float tfar)
{
	// compute intersection of ray with all six bbox planes
	vec3 invR = 1.0 / rd;
	vec3 tbot = invR * (boxmin - ro);
	vec3 ttop = invR * (boxmax - ro);
	// re-order intersections to find smallest and largest on each axis
	vec3 tmin = min (ttop, tbot);
	vec3 tmax = max (ttop, tbot);
	// find the largest tmin and the smallest tmax
	vec2 t0 = max (tmin.xx, tmin.yz);
	tnear = max (t0.x, t0.y);
	t0 = min (tmax.xx, tmax.yz);
	tfar = min (t0.x, t0.y);
	// check for hit
	bool hit;
	if ((tnear > tfar)) 
		hit = false;
	else
		hit = true;
	return hit;
}

float VideoHeightFieldLuminance(sampler2D tex, vec2 uv)
{
	vec3 c = texture2D(tex, uv).xyz;
	return dot(c, vec3(0.33, 0.33, 0.33));
}

vec2 gradient(sampler2D tex, vec2 uv, vec2 texelSize)
{
	float h = VideoHeightFieldLuminance(tex, uv);
	float hx = VideoHeightFieldLuminance(tex, uv + texelSize*vec2(1.0, 0.0));	
	float hy = VideoHeightFieldLuminance(tex, uv + texelSize*vec2(0.0, 1.0));
	return vec2(hx - h, hy - h);
}

vec2 VideoHeightFieldWorldToTex(vec3 p)
{
	vec2 uv = p.xz*0.5+0.5;
	uv.y = 1.0 - uv.y;
	return uv;
}

float VideoHeightFieldHeightField(vec3 p)
{
	return VideoHeightFieldLuminance(iChannel0, VideoHeightFieldWorldToTex(p))*iFreq0/250.0;
}

bool VideoHeightFieldTraceHeightField(vec3 ro, vec3 rayStep, out vec3 hitPos)
{
	vec3 p = ro;
	bool hit = false;
	float pH = 0.0;
	vec3 pP = p;
	for(int i=0; i<VideoHeightFieldSteps; i++) {
		float h = VideoHeightFieldHeightField(p);
		if ((p.y < h) && !hit) {
			hit = true;
			//hitPos = p;
			// interpolate based on height
            hitPos = mix(pP, p, (pH - pP.y) / ((p.y - pP.y) - (h - pH)));
		}
		pH = h;
		pP = p;
		p += rayStep;
	}
	return hit;
}

vec3 VideoHeightFieldBackground(vec3 rd)
{
     // blue return mix(vec3(1.0, 1.0, 1.0), vec3(0.0, 0.5, 1.0), abs(rd.y));
     return mix(iBackgroundColor, iBackgroundColor, abs(rd.y));
}

void main(void)
{
 	vec2 uv = 0.5 * iZoom * vec2( (gl_FragCoord.x / iResolution.x)*2.4-1.2, (gl_FragCoord.y / iResolution.y)*-2.4+1.2 );
    //vec2 uv = 2.0 * iZoom * (gl_TexCoord[0].st- 0.5);
    uv.x *= float(iResolution.x )/ float(iResolution.y);
    uv.x -= iRenderXY.x;
    uv.y -= iRenderXY.y;    



    // compute ray origin and direction
    float asp = iResolution.x / iResolution.y;
    vec3 rd = normalize(vec3(asp*uv.x, uv.y, -2.0));
    vec3 ro = vec3(0.0, 0.0, 2.0);
		
	vec2 mouse = iMouse.xy / iResolution.xy;

	// rotate view
    float ax = -0.7;
	if (iMouse.x > -0.1) {
    	ax = -(1.0 - mouse.y)*2.0 - 1.0;
	}
    rd = VideoHeightFieldRotateX(rd, ax);
    ro = VideoHeightFieldRotateX(ro, ax);
		
	float ay = sin(iGlobalTime*0.04);
    rd = VideoHeightFieldRotateY(rd, ay);
    ro = VideoHeightFieldRotateY(ro, ay);
	
	// intersect with bounding box
    bool hit;	
	const vec3 boxMin = vec3(-1.0, -0.01, -1.0);
	const vec3 boxMax = vec3(1.0, 0.5, 1.0);
	float tnear, tfar;
	hit = VideoHeightFieldIntersectBox(ro, rd, boxMin, boxMax, tnear, tfar);

	tnear -= 0.0001;
	vec3 pnear = ro + rd*tnear;
    vec3 pfar = ro + rd*tfar;
	
    float stepSize = length(pfar - pnear) / float(VideoHeightFieldSteps);
	
    vec3 rgb = VideoHeightFieldBackground(rd);
    if(hit)
    {
    	// intersect with heightfield
		ro = pnear;
		vec3 hitPos;
		hit = VideoHeightFieldTraceHeightField(ro, rd*stepSize, hitPos);
		if (hit) {
			vec2 uv = VideoHeightFieldWorldToTex(hitPos);
			rgb = texture2D(iChannel4, uv).xyz;
		}
     }

    gl_FragColor=vec4(rgb, 1.0);
}
