// hexler https://vimeo.com/54385033
#define specx iFreq0/1000.0
#define specy iFreq1/1000.0
#define specz iFreq2/1000.0
void main(void)
{
   vec2 uv = iZoom * gl_TexCoord[0].st;
   uv.x -= iRenderXY.x;
   uv.y -= iRenderXY.y;
  uv = 2.0 * (uv) - 1.0;
  float r = length(uv); float p = atan(uv.y/uv.x); uv = abs(uv);
  float col = 0.0;float amp = (specx+specy+specz)/3.0;
  uv.y += sin(uv.y*3.0*specx-iGlobalTime/5.0*specy+r*10.);
  uv.x += cos((iGlobalTime/5.0)+specx*30.0*uv.x);
  col += abs(1.0/uv.y/30.0) * (specx+specz)*15.0;
  col += abs(1.0/uv.x/60.0) * specx*8. ;
     
  gl_FragColor = vec4(vec3( col ),1.0);
}
