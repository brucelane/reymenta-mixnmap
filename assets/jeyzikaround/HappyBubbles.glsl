// 
// Created by inigo quilez - iq/2013 : https://www.shadertoy.com/view/4dl3zn
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// Messed up by Weyland
void main(void)
{
   vec2 uv = iZoom * gl_TexCoord[0].st;
   uv.x -= iRenderXY.x;
   uv.y -= iRenderXY.y;
	uv *= 2.0;
	uv -= 1.0;
	uv.x *=  iResolution.x / iResolution.y;

	// background	 
	vec3 color = iBackgroundColor;

    // bubbles	
	for( int i=0; i<iSteps; i++ )
	{
        // bubble seeds
		float pha =      sin(float(i)*546.13+1.0)*0.5 + 0.5;
		float siz = pow( sin(float(i)*651.74+5.0)*0.5 + 0.5, 4.0 );
		float pox =      sin(float(i)*321.55+4.1) * iResolution.x / iResolution.y;

        // buble size, position and color
		float rad = 0.1 + 0.5*siz+sin(iGlobalTime/60.+pha*500.+siz)/20.;
		vec2  pos = vec2( pox+sin(iGlobalTime/40.+pha+siz), -1.0-rad + (2.0+2.0*rad)
						 *mod(pha+0.1*(iGlobalTime/5.)*(0.2+0.8*siz),1.0));
		float dis = length( uv - pos );
		vec3  col = mix( vec3(0.194*sin(iGlobalTime/6.0),0.3,0.0), 
						vec3(1.1*sin(iGlobalTime/9.0),0.4,0.8), 
						0.5+0.5*sin(float(i)*1.2+1.9));
		      //col+= 8.0*smoothstep( rad*0.95, rad, dis );
		
        // render
		float f = length(uv-pos)/rad;
		f = sqrt(clamp(1.0-f*f,0.0,1.0));
		color -= col.zyx *(1.0-smoothstep( rad*0.95, rad, dis )) * f;
	}
  gl_FragColor = vec4(color,1.0);
}
