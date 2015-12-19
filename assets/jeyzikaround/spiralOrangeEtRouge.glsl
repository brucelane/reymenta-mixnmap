//  https://www.shadertoy.com/view/lds3WB

void main(void) {
	float aspectRatio = iResolution.x / iResolution.y;
	vec2 p = iZoom * 2.0 * gl_FragCoord.xy / iResolution.y - vec2(aspectRatio/3.0, 1.0);

	p.x -= iRenderXY.x;
	p.y -= iRenderXY.y;

	vec2 uv = 0.4 * p;
	float distSqr = dot(uv, uv);
	float vignette = 1.0 - distSqr;
	float angle = atan(p.y, p.x);
	float shear = sqrt(distSqr);
	float blur = 0.5;
	//float stripes = smoothstep(-blur, blur, cos(8.0 * angle + 12.0 * iGlobalTime - 12.0 * shear));
	float stripes = smoothstep(-blur, blur, cos(8.0 * angle + iGlobalTime * iRatio /12.0 - 12.0 * shear));
	//float stripes = smoothstep(-blur, blur, cos(8.0 * angle + iGlobalTime * iRotationSpeed /12.0 - 12.0 * shear));
	gl_FragColor = vec4(vignette * mix(iBackgroundColor, iColor, stripes), 1.0);
}