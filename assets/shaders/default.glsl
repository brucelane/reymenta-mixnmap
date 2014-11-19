
// https://www.shadertoy.com/view/MdBSzV

const float FALL_OFF = 0.5;
const float HALF_PI = 1.57079632679;

void main(void)
{
    float aspect = iResolution.x / iResolution.y;
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
    uv.x *= aspect;
    vec2 uv2 = uv;
    
    vec2 mp = iMouse.xy / iResolution.xy;
    mp.x *= aspect;

    vec2 dif = mp - uv;
    float d = length(dif);
    float dist = clamp(d, 0.0, FALL_OFF);
    float power = pow(cos(dist * HALF_PI / FALL_OFF), 2.0);
    power *= FALL_OFF;
    dif *= power;
    uv += dif;
    uv2 -= dif;
    
    vec2 offset = vec2(iResolution.x, iResolution.y * aspect) / 10.0;
    mp *= offset;
    uv *= offset;
    uv2 *= offset;
    
    float bars = max(pow(cos(uv.x + iGlobalTime), 5.0), pow(cos(uv.y), 5.0));
    float bars2 = max(pow(cos(uv2.x - iGlobalTime), 5.0), pow(cos(uv2.y), 5.0));

    vec3 light = vec3(mp, 1.0);
    float topBarsLight = inversesqrt(distance(vec3(uv, power), light) * 0.05) * 0.5;
    float btmBarsLight = inversesqrt(distance(vec3(uv2, power), light) * 0.05) * 0.5;

    vec3 col = vec3(topBarsLight) * max(bars * topBarsLight, bars2 * btmBarsLight - 0.25);

	gl_FragColor = vec4(col ,1.0);
}

