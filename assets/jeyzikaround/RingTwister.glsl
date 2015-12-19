// https://www.shadertoy.com/view/Xt23z3
#define NUM_FACES 4
#define ANTI_ALIAS
float tau = atan(1.0) * 8.0;
float pi = atan(1.0) * 4.0;

vec4 slice(float x0, float x1, vec2 uv)
{
    float u = (uv.x - x0)/(x1 - x0);
    float w = (x1 - x0);
    vec3 col = vec3(0);
    
    //Orange-yellow gradient
    col = mix(vec3(1, 0.6, 0), vec3(1, 0.8, 0), u);
    
    //Lighting 
    col *= w * 0.5;
    
    //Edges
    col *= smoothstep(0.05, 0.10, u) * smoothstep(0.95, 0.90, u) + 0.5;
    
    //Checker board
    
    #ifdef ANTI_ALIAS
    	col *= (-1.0+2.0*smoothstep(-0.05,0.05,sin(u*pi*4.0) * cos(uv.y*16.0))) * (1.0/16.0) + 0.7;
    #else
    	col *= sign(sin(u * pi * 4.0) * cos(uv.y * 16.0)) * (1.0/16.0) + 0.7;
    #endif
    
    float clip = 0.0;
    
    #ifdef ANTI_ALIAS
    	clip = (1.0-smoothstep(0.5-0.05/w,0.5+0.05/w,abs(u-0.5))) * step(x0,x1);
    #else
    	clip = float((u >= 0.0 && u <= 1.0) && (x0 < x1));
    #endif
    
    return vec4(col, clip);
}

void main(void)
{
    vec2 res = iResolution.xy / iResolution.y;
	vec2 uv = gl_FragCoord.xy / iResolution.y;
    uv -= res / 2.0;
    uv *= 8.0;
    
    //Polar coordinates
    vec2 uvr = vec2(length(uv), atan(uv.y, uv.x) + pi);
    uvr.x -= 3.0;
    
    vec3 col = vec3(0.05);
    
    //Twisting angle
    float angle = uvr.y + iGlobalTime + sin(uvr.y) * sin(iGlobalTime) * pi;
    
    for(int i = 0;i < NUM_FACES;i++)
    {
        float x0 = sin(angle + tau * (float(i) / float(NUM_FACES)));
        float x1 = sin(angle + tau * (float(i + 1) / float(NUM_FACES)));
        
        vec4 face = slice(x0, x1, uvr);
        
        col = mix(col, face.rgb, face.a); 
    }
    
	gl_FragColor = vec4(col, 1.0);
}

