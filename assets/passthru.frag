// #include "precision.glsl"

uniform sampler2D uSampler;

in Vertex
{
	vec2 	uv;
} vertex;

out vec4 oColor;

void main( void )
{
	vec4 color 	= vec4( 1.0 );
	color 	= vec4( texture( uSampler, vertex.uv ).xyz, 1.0 );

	oColor 		= color;
}
