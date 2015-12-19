// https://www.shadertoy.com/view/Xs2GDR
// Quasicrystal based on Matt Henderson's blog post:
// http://blog.matthen.com/post/51566631087/quasicrystals-are-highly-structured-patterns-which	
// click and drag the mouse for interactivity.
// Left/Right: scale
// Up/Down: number of waves

// normalized mouse position. Default values
float QuasiCrystalMouseX = 0.8;
float QuasiCrystalMouseY = 0.4;
void main(void)
{
   vec2 uv = 2.0 * iZoom * (gl_TexCoord[0].st- 0.5);
   uv.x -= iRenderXY.x;
   uv.y -= iRenderXY.y;
	if( iMouse.x != 0.0 ) { // once mouse pos is initialized use that:
		QuasiCrystalMouseX = float(iMouse.x) / float(iResolution.x);
		QuasiCrystalMouseY = float(iMouse.y) / float(iResolution.y);
	}
	
	float speed = 2.0;
	float scale = 0.1;
	const int N_MAX = 20;
	float n = 7.0;
	
	// click and drag up and down to change the number of symmetries (up is more)
	// n is the number of waves. we allow a fractional number and fade out the final one below
	n = QuasiCrystalMouseY * float(N_MAX) +1.0;
	
	// click and drag left and right to adjust scale (left zooms out)
	scale = 0.1 + (1.0-QuasiCrystalMouseX);
	
	const float pi = 3.1415926;
	
	// accumulate n waves
	float S = 0.0;
	for( int i=0; i < N_MAX; ++i){
	
		// allow for n+1 waves
		if( i >= (int(n)+1) )
			break;
		
		float theta = pi * float(i)/float(n);
			
		// center coordinates in viewport
		float x = gl_FragCoord.x - (iResolution.x*0.5);
		float y = gl_FragCoord.y - (iResolution.y*0.5);
		
		float wp = x*cos(theta) + y*sin(theta);
		float w = sin(wp*scale + iGlobalTime*speed);
		
		// fade out the final wave in proportion to n-floor(n)
		if( i == int(n) )
			w *= n-floor(n);		
		
		// accumulate 
		S += 3.0 * w / n;
	}
	// greyscale version:
	//gl_FragColor = vec4(S,S,S,1.0);

	// sin/cos hackery to get some colours
  gl_FragColor = vec4(vec3(sin(S-0.1)*iColor.r,cos(S+.3)*iColor.g,cos(S+.1)*iColor.b),1.0);
}
