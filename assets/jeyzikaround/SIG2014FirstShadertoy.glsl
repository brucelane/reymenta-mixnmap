// https://www.shadertoy.com/view/4dsXWs
// The min and max sizes of the circles (in pixels) over time.
#define MIN_SIZE 4.0
#define MAX_SIZE 150.0

// Compute the relative distance to the circle, where < 0.0 is outside the feathered border,
// and > 1.0 is inside the feathered border.
float ComputeCircle(vec2 pos, vec2 center, float radius, float feather)
{
    // Determine the distance to the center of the circle.
	float dist = length(center - pos);
    
    // Use the distance and the specified feather factor to determine where the distance lies
    // relative to the circle border.
    float start = radius - feather;
    float end   = radius + feather;
    return smoothstep(start, end, dist);
}

// The main function, which is executed once per pixel.
void main(void)
{
    // Prepare the circle parameters, cycling the circle size over time.
    float cycle = clamp(cos(iGlobalTime / 2.0) * 0.6 + 0.7, 0.0, 1.0);
    float diameter = mix(MAX_SIZE, MIN_SIZE, cycle);
    float radius = diameter / 2.0;
    vec2  center = vec2(0.0);
    
    // Compute the relative distance to the circle, using mod() to repeat the circle across the display.
    // A feather value (in pixels) is used to reduce aliasing artifacts when the circles are small.
    // The position is adjusted so that a circle is in the center of the display.
    vec2 screenPos = gl_FragCoord.xy - (iResolution.xy / 2.0) - vec2(radius);
    vec2 pos = mod(screenPos, vec2(diameter)) - vec2(radius);
    float d = ComputeCircle(pos, center, radius, 0.5);
    
    // Compute "pixelated" (stepped) texture coordinates using the floor() function.
    // The position is adjusted to match the circles, i.e. so a pixelated block is at the center of the
    // display.
    vec2 count = iResolution.xy / diameter;
    vec2 shift = vec2(0.5) - fract(count / 2.0);
    vec2 uv = floor(count * gl_FragCoord.xy + shift) / count;
    
	// Sample the texture, using an offset to the center of the pixelated block.
    // NOTE: Use a large negative bias to effectively disable mipmapping, which would otherwise lead
    // to sampling artifacts where the UVs change abruptly at the pixelated block boundaries.
    uv += vec2(0.5) / count;
    uv = clamp(uv, 0.0, 0.99);
    uv.y = 1.0 - uv.y;
    vec3 texColor = iColor;    
    
	// Calculate the color based on the circle shape, mixing between that color and a background color.
    vec3 col = mix(texColor, iBackgroundColor, d);
        
    // Set the final fragment color.
	gl_FragColor = vec4(col, 1.0);
}
