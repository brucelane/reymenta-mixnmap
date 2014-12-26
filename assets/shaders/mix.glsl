
out vec4 oColor;

vec3 shaderLeft(vec2 uv)
{
	vec4 left = texture2D(iChannel0, uv);
	return vec3( left.r, left.g, left.b );
}

vec3 shaderRight(vec2 uv)
{
	vec4 right = texture2D(iChannel1, uv);
	return vec3( right.r, right.g, right.b );
}

vec3 mainFunction( vec2 uv )
{
   return mix( shaderLeft(uv), shaderRight(uv), iCrossfade );
}
void pRect(vec4 _p,vec3 _c)
{
	vec2 p=gl_FragCoord.xy;
    if((_p.x<p.x&&p.x<_p.x+_p.z&&_p.y<p.y&&p.y<_p.y+_p.w)) oColor=vec4(_c,0.);
}

void print(float _i,vec2 _f,vec2 _p,vec3 _c)
{
    bool n=(_i<0.)?true:false;
    if(gl_FragCoord.x<_p.x-5.-(max(ceil(log(_i)/log(10.)),_f.x)+(n?1.:0.))*30.||_p.x+6.+_f.y*30.<gl_FragCoord.x||gl_FragCoord.y<_p.y||_p.y+31.<gl_FragCoord.y)return;
    
    _i=abs(_i);
    if(0.<_f.y){pRect(vec4(_p.x-5.,_p.y,11.,11.),vec3(1.));pRect(vec4(_p.x-4.,_p.y+1.,9.,9.),_c);}
    
    float c=-_f.y,m=0.;
    for(int i=0;i<16;i++)
    {
        float x,y=_p.y;
        if(0.<=c){x=_p.x-35.-30.*c;}
        else{x=_p.x-25.-30.*c;}
        if(int(_f.x)<=int(c)&&_i/pow(10.,c)<1.&&0.<c)
        {
            if(n){pRect(vec4(x,y+10.,31.,11.),vec3(1.));pRect(vec4(x+1.,y+11.,29.,9.),_c);}
            break;
        }
        float l=fract(_i/pow(10.,c+1.));
        if(l<.1){pRect(vec4(x,y,31.,31.),vec3(1.));pRect(vec4(x+1.,y+1.,29.,29.),_c);pRect(vec4(x+15.,y+10.,1.,11.),vec3(1.));}
        else if(l<.2){pRect(vec4(x+5.,y,21.,31.),vec3(1.));pRect(vec4(x,y,31.,11.),vec3(1.));pRect(vec4(x,y+20.,6.,11.),vec3(1.));pRect(vec4(x+6.,y+1.,19.,29.),_c);pRect(vec4(x+1.,y+1.,29.,9.),_c);pRect(vec4(x+1.,y+21.,5.,9.),_c);}
        else if(l<.3){pRect(vec4(x,y,31.,31.),vec3(1.));pRect(vec4(x+1.,y+1.,29.,29.),_c);pRect(vec4(x+15.,y+10.,15.,1.),vec3(1.));pRect(vec4(x+1.,y+20.,15.,1.),vec3(1.));}
        else if(l<.4){pRect(vec4(x,y,31.,31.),vec3(1.));pRect(vec4(x+1.,y+1.,29.,29.),_c);pRect(vec4(x+1.,y+10.,15.,1.),vec3(1.));pRect(vec4(x+1.,y+20.,15.,1.),vec3(1.));}
        else if(l<.5){pRect(vec4(x,y+5.,15.,26.),vec3(1.));pRect(vec4(x+15.,y,16.,31.),vec3(1.));pRect(vec4(x+1.,y+6.,14.,24.),_c);pRect(vec4(x+16.,y+1.,14.,29.),_c);pRect(vec4(x+15.,y+6.,1.,10.),_c);}
        else if(l<.6){pRect(vec4(x,y,31.,31.),vec3(1.));pRect(vec4(x+1.,y+1.,29.,29.),_c);pRect(vec4(x+1.,y+10.,15.,1.),vec3(1.));pRect(vec4(x+15.,y+20.,15.,1.),vec3(1.));}
        else if(l<.7){pRect(vec4(x,y,31.,31.),vec3(1.));pRect(vec4(x+1.,y+1.,29.,29.),_c);pRect(vec4(x+10.,y+10.,11.,1.),vec3(1.));pRect(vec4(x+10.,y+20.,20.,1.),vec3(1.));}
        else if(l<.8){pRect(vec4(x,y+10.,15.,21.),vec3(1.));pRect(vec4(x+15.,y,16.,31.),vec3(1.));pRect(vec4(x+1.,y+11.,14.,19.),_c);pRect(vec4(x+16.,y+1.,14.,29.),_c);pRect(vec4(x+15.,y+20.,1.,10.),_c);}
        else if(l<.9){pRect(vec4(x,y,31.,31.),vec3(1.));pRect(vec4(x+1.,y+1.,29.,29.),_c);pRect(vec4(x+10.,y+10.,11.,1.),vec3(1.));pRect(vec4(x+10.,y+20.,11.,1.),vec3(1.));}
        else{pRect(vec4(x,y,31.,31.),vec3(1.));pRect(vec4(x+1.,y+1.,29.,29.),_c);pRect(vec4(x+1.,y+10.,20.,1.),vec3(1.));pRect(vec4(x+10.,y+20.,11.,1.),vec3(1.));}
        c+=1.;
    }
}

/*
void main(void)
{
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
   vec4 left = texture2D(iChannel0, uv);
    
   oColor = vec4( left.r, left.g, 1.0 * sin(iGlobalTime), 1.0 );
   
}
*/
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec3 col;
	if ( iCrossfade > 0.95 )
	{
		col = shaderRight(uv);
	}
	else
	{
		if ( iCrossfade < 0.05 )
		{
			col = shaderLeft(uv);
		}
		else
		{
			col = mainFunction( uv );

		}
	}
	print(iGlobalTime,vec2(4.,2.),vec2(10.,10.),vec3(1.0));

	oColor = iAlpha * vec4( col, 1.0 );
}