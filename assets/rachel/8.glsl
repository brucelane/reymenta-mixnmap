// https://www.shadertoy.com/view/4dsSDn
void main(void)
{
	//vec2 uv = iZoom * (gl_FragCoord.xy / iResolution.xy);
	vec2 uv = iZoom * gl_TexCoord[0].st;
	uv.x -= iRenderXY.x;
	uv.y -= iRenderXY.y;
    float a = 1.0;
	if (iInvert==1)
	{
	    float theta = 1.0*(1.0/(3.14159/2.0))*atan(uv.x,uv.y);
	    float r = length(uv);
		a=1.0-r;//vignette
	    uv = vec2(theta,r);	
	}	
	else
	{
		uv=abs(2.0*(uv-0.5));
	}
		
	vec4 t1 = texture2D(iChannel0, vec2(uv[0],0.1) );
    vec4 t2 = texture2D(iChannel0, vec2(uv[1],0.1) );
   	float fft = t1[0]*t2[0]*a;
	gl_FragColor = vec4( sin(fft*3.141*2.5), sin(fft*3.141*2.0),sin(fft*3.141*1.0),1.0);
}


