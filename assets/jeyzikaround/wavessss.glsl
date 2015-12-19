// https://www.shadertoy.com/view/4lS3zV

void main(void)
{
    float amp = 8.0;
    float freq = 2.0; //* abs(sin(iGlobalTime));
    float slide = 1.0 - iGlobalTime * 1.0;
    float xx = 1.0 - abs((gl_FragCoord.y / iResolution.x - .3) * amp - sin((gl_FragCoord.x / iResolution.x - slide) * freq));
    float xz = 1.0 - abs((gl_FragCoord.y / iResolution.x - .3) * amp - sin((gl_FragCoord.x / iResolution.x - 1.0 - slide * -.5) * freq));
    float xr = 1.0 - abs((gl_FragCoord.y / iResolution.x - .3) * amp - sin((gl_FragCoord.x / iResolution.x - 2.4 - slide * 1.9) * freq));
	gl_FragColor.b = xz;
    gl_FragColor.r = xx;
    gl_FragColor.g = xr;
    gl_FragColor.a = 1.0;
}
