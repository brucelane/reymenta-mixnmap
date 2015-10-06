//for audio input
in Vertex
{
	vec2 	uv;
} vertex;
out vec4 fragColor;
//void main(void)
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	//vec2 uv = gl_FragCoord.xy / iResolution.xy;
	// flip horizontally
	if (iFlipH)
	{
		vertex.uv.x = 1.0 - vertex.uv.x;
	}
	// flip vertically
	if (iFlipV)
	{
		vertex.uv.y = 1.0 - vertex.uv.y;
	}
   	vec4 tex = texture2D(iChannel4, vertex.uv);
   	fragColor = vec4(vec3( tex.r, tex.g, tex.b ),1.0);
}
