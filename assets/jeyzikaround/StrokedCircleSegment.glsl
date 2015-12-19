// https://www.shadertoy.com/view/Mll3D4
const float kInvPi = 1.0 / 3.141592;

float modulo( in float x, in float y )
{
	if ( y == 0.0 )	return x;
	return x - y * floor( x / y );
}

float wrap( in float x, in float lower, in float upper )
{
	return modulo( x - lower, upper - lower ) + lower;
}

void main(void)
{
    // Define how blurry the circle should be. 
    // A value of 1.0 means 'sharp', larger values
    // will increase the bluriness.
    float bluriness = 1.0;
    
    // In the range (0, 1].
    float radius = 0.75;
    
    // In the range (0, 1].
    float thickness = 0.2;
    
    // In the range (0, 1].
    float len = 0.6 + 0.4 * cos( iGlobalTime );
    
    // Optional offset.
    float offset = 0.1 * iGlobalTime;
    
    // Calculate texture coordinates. Normally you would
    // simply pass them from the vertex shader.
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
    
    // Convert from range [0,1] to [-1,1]
    uv = 2.0 * uv - 1.0;
    
    // Adjust for the aspect ratio. Not necessary if
    // you supplied texture coordinates yourself.
    uv.x *= (iResolution.x / iResolution.y);
    
    // Calculate distance to (0,0).
    float d = length( uv );
    
    // Calculate angle, so we can draw segments, too.
	float angle = atan( uv.x, uv. y ) * kInvPi * 0.5;
	angle = wrap( angle - offset, 0.0, 1.0 );
    
    // Create an anti-aliased circle.
    float w = bluriness * fwidth( d );
    float circle = smoothstep( radius + w, radius - w, d );
    
    // Optionally, you could create a hole in it:
    float inner = radius - thickness;
    circle -= smoothstep( inner + w, inner - w, d );
    
    // Or only draw a portion (segment) of the circle.
    float segment = smoothstep( len + 0.002, len, angle );
	segment *= smoothstep( 0.0, 0.002, angle );    
    circle = mix( circle * segment, circle, step( 1.0, len ) );
        
    // Let's define the circle's color now.
    vec3 rgb = vec3( 0.5, 0.0, 0.5 ) * circle;     
    
    // Output final color.
    gl_FragColor = vec4( rgb, 1);    
}