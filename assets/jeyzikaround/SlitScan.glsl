//https://www.shadertoy.com/view/4t2Gz3
void main(void)
{
 	float x = gl_FragCoord.x;
    float y = gl_FragCoord.y;
    
    float z = floor((x/20.0) + 0.5);
    
    float y2 = y + (sin(z + (iGlobalTime * 10.0)) * 10.5);
    
    vec2 uv2 = vec2(x / iResolution.x, y2/ iResolution.y);
    
    vec3 texSample 	= texture2D( iChannel1, uv2 ).rgb;
    
    gl_FragColor = vec4(texSample, 1.0); 

}
