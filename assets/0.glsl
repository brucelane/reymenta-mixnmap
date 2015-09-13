//for spout input
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
   	//vec4 tex = texture2D(iChannel4, 1-uv);
   	vec4 tex = texture2D(iChannel4, uv);
   	gl_FragColor = vec4(vec3( tex.r, tex.g, tex.b ),1.0);
}

