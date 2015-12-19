void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	// aspect-ratio correction
   	uv.x*=iResolution.x/iResolution.y;
   	uv.x -= iRenderXY.x;
   	uv.y -= iRenderXY.y;

	float rad = radians(360.0 * fract(iGlobalTime*iRotationSpeed));
	mat2 rotate = mat2(cos(rad),sin(rad),-sin(rad),cos(rad));
	uv = rotate * (uv - 0.5) + 0.5;

	float x = (uv.x - 0.5)*iZoom*2.0;
	float y = (uv.y - 0.5)*iZoom*2.0;

	vec2 c = vec2(x, y);

   	vec4 tex = texture2D(iChannel1, uv-c);//-c);
   	gl_FragColor = vec4(vec3( tex.r, tex.g, tex.b ),1.0);
}

