// https://www.shadertoy.com/view/MtSGzW
//Right! It's 11:11 where I'm at, and I'm going to attempt to program a real-time raytracer.
//Because, um, it seems like a good idea.
//UNOPTIMIZED. LEARNING FROM THIS WILL MAKE YOUR CODE REALLY BAD.
//But it might be cool to look at!

//uses IQ's Distance Functions: http://iquilezles.org/www/articles/distfunctions/distfunctions.htm
//and heavily inspired by Fairlight's PHOTON and Feed Me Lies, which was itself inspired by Bot & Dolly's Box:
//https://vimeo.com/75260457

//OpenGL: z minus is into the screen
vec3 rotY(vec3 p, float rot){
    return vec3(p.x*cos(rot)-p.z*sin(rot),p.y,p.x*sin(rot)+p.z*cos(rot));
}

float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0)+length(max(d,0.0));
  //return length(max(d,0.0));
}

//distance, object ID (0=walls, 1=box)
vec2 df(in vec3 p){
    //back face:
    float back=2.0;
    float w2=1.78;
    float h2=1.0;
    float wall=0.1/2.0;
    //return sdBox(p,vec3(w2,h2,1));
    /*return opS(sdBox(p-vec3(0.0,0.0,-back*0.5),vec3(w2+wall,h2+wall,back*0.5)),
               sdBox(p-vec3(0.0,0.0,-back*0.5),vec3(w2-wall,h2-wall,back*0.5)));*/
    
    p=vec3(mod(p.x-2.0,4.0)-2.0,mod(p.y-1.1,2.2)-1.1,p.z);
    
    float d= max(-sdBox(p-vec3(0.0,0.0,-back*0.5+wall),vec3(w2-wall,h2-wall,back*0.5)),
                 sdBox(p-vec3(0.0,0.0,-back*0.5),vec3(w2+wall,h2+wall,back*0.5)));
    //float id=0.0;

    float v=sdBox(p-vec3(0.0,-0.51069,-1.6),vec3(0.319,0.417,0.319));
    //v=min(v,sdBox(p-vec3(1.0,-0.51069,-1.6),vec3(0.319,0.417,0.319)));
    //v=min(v,sdBox(p-vec3(-1.0,-0.51069,-1.6),vec3(0.319,0.417,0.319)));
    float id=0.5*sign(d-v)+0.5;
    d=min(d,v);
    /*if(v<d){
        d=v;
        id=1.0;
    }*/
    return vec2(d,id);
    //return opS(sdBox(p,vec3(w2+wall,h2+wall,back+wall)),sdBox(p,vec3(w2-wall,h2-wall,back-wall)));
}

vec2 intersect(vec3 co, vec3 ci, float tmin){
    float t=tmin;
    float dist=0.0;
    for(int i=0;i<48;i++){
        dist=df(co+t*ci).r;
        if(dist<0.001){ //iq uses something smaller
            break;
        }
        t+=dist;//max(dist,0.002);
    }
    return vec2(t,df(co+t*ci).g); //may actually be fast!
}

vec2 intersectFast(vec3 co, vec3 ci, float tmin, float tmax){
    float t=tmin;
    float dist=0.0;
    float minDist=1000.0;
    for(int i=0;i<14;i++){
        dist=df(co+t*ci).r;
        t+=dist;
    }
    return vec2(t,df(co+t*ci).g);
}

//NOT NORMALIZED
vec3 calcNormal(in vec3 p){
    float origin=df(p).r;
    vec3 eps=vec3(0.001,0.0,0.0);
    float sc=1.0/eps.x;
    return sc*vec3(df(p+eps.xyy).r-origin,
                df(p+eps.yxy).r-origin,
                df(p+eps.yyx).r-origin);
}

vec3 gamma(vec3 col){
    return pow(clamp(col,0.0,1.0),vec3(0.45));
}

vec3 degamma(vec3 col){
    return pow(clamp(col,0.0,1.0),vec3(2.2));
}

float degamma(float col){
    return pow(clamp(col,0.0,1.0),2.2);
}

vec2 cubemap(vec3 p, vec3 n){
    //Maps position, normal-> texture coordinate.
    //n=1,0,0: just y and z
    //This isn't a particularly nice way to do things, but it works.
    n=abs(n);
    if(n.x>n.y && n.x>n.z){
        return p.yz;
    }else if(n.y>n.x && n.y>n.z){
        return p.xz;
    }else{
        return p.xy;
    }
}

vec4 cubetex(sampler2D channel, vec3 p, vec3 n){
    return texture2D(channel,cubemap(p,n));
}

 
//HAAACK
vec3 texrot(vec3 dirzp, vec3 n){
    vec3 tn=abs(n);
    if(tn.z>tn.x && tn.z>tn.y){
        return dirzp.xyz*sign(n.z);
    }else if(tn.y>tn.x && tn.y>tn.z){
        return dirzp.xzy*sign(n.y);
    }else{
        return dirzp.zxy*sign(n.x);
    }
}

//Pretend the image's really a depth map, and extract a normal from it.
//We have a position. Right is +X, down is +Y, up is Z.
//Erm.
//We'd need tangent-space coordinates, which.... are those easy to find?
//Easier method: Make stuff up and hope you can get away with it.
//Okay. So we have the incoming light ray, the normal, and all sorts of other stuff.
//Per-wall? If +X, z->x, If +Y, z->y. Okay, that's simple! And then invert things if signs are inverted.
//The math here is obviously bogus.
vec3 tex2norm(sampler2D channel, vec2 uv, vec3 n){
    vec2 e=vec2(0.001,0.0); //obv too much, but oh well
    float center=degamma(texture2D(channel,uv-e).g);
    float dx=degamma(texture2D(channel,uv+e).g)-center;
    float dy=degamma(texture2D(channel,uv+e.yx).g)-center;
    //normal: n.p=d, so n.(e,0,dx)=0,  n.(0,e,dy)=0 - e*n.x+n.z*dx=0. Since we're normalizing...
    vec3 tn=normalize(clamp(vec3(-dx/e.x,-dy/e.x,1),-1.0,1.0));
    //maybe a mix helps?
    tn=normalize(mix(vec3(0,0,1),tn,tn.z));
    return texrot(tn,n);
}


//Evaluates a simple BRDF. Doesn't account for stuff like reflections. That's for the raytracer.
float calcDirectLighting(vec3 p, vec3 norm){   
        vec3 lightpos=vec3((iMouse.x/iResolution.x)-0.5,0.0,0);
        vec3 lightdir=lightpos-p;
    float e=5.0;
        return e*dot(normalize(lightdir),norm)/(dot(lightdir,lightdir));
}

vec3 sampleMovie(vec2 p){
    p=vec2(mod(p.x-2.0,4.0)-2.0,mod(p.y-1.1,2.2)-1.1);
    return degamma(texture2D(iChannel1,(p-vec2(-1.5,-1.0))*vec2(0.325,0.5)).rgb);
}

vec3 walls(vec3 p, vec3 n, vec3 wallColor){
                //MEH!
            if(n.z>0.8 && p.z<0.0){ //back wall
                return sampleMovie(p.xy);
            }else{
                //col=degamma(cubetex(iChannel2,0.5*p,nor).rgb);
                return wallColor;
            }
}

float random(float p){return fract(cos(p)*123456.);} //[0,1]

vec3 noise(vec2 uv){
    return texture2D(iChannel0,uv).rgb;
}

vec3 noise2(vec2 uv, float r){
    return texture2D(iChannel0,uv+0.1*vec2(cos(r),sin(r))).rgb;
}

void main(void)
{
    vec2 ss = gl_FragCoord.xy / iResolution.x;
    //stochastic aa
    vec2 cs = (gl_FragCoord.xy-iResolution.xy*0.5)/iResolution.x; //lower-left=-1,-y/x
    
    float //time=iGlobalTime*0.1;
    time=0.4*cos(iGlobalTime*0.3);
    
    
    
    vec3 co;
    vec3 ci;
    if(iMouse.z>0.0){
         time=2.0*(iMouse.x/iResolution.x-0.5);
        
        float rotation=time;
        co=rotY(vec3(0.0,0.0,5.0),rotation);
        float zoom=0.5/0.6;
        ci=rotY(normalize(vec3(zoom*cs.x,zoom*cs.y,-1)),rotation);
    }else{
        //time=0.0; //STOP TIME!
        float camz=3.0+8.0*(exp(smoothstep(5.0,30.0,iGlobalTime))-1.0);
        float rotation=mix(time,-0.95,smoothstep(20.0,32.0,iGlobalTime));
        co=rotY(vec3(0.0,0.0,camz),rotation)+vec3(1.2*mix(0.0,iGlobalTime-26.0,smoothstep(26.0,35.0,iGlobalTime)),0.0,0.0);
        float zoom=0.5/0.6;
        ci=rotY(normalize(vec3(zoom*cs.x,zoom*cs.y,-1)),rotation);
    }
    
    vec2 res=intersect(co,ci,0.0);
    float t=res.r;
    
    vec3 col=vec3(0.0);
    
    if(t<50.0){
        vec3 p=co+t*ci;
        vec3 nor=calcNormal(p);
        nor=normalize(nor);
        vec2 uv=cubemap(p,nor);
        
        vec3 albedo=vec3(0.0);
        vec3 emissive=vec3(0.0);
        
        if(res.g<0.9){
            albedo=walls(p,nor,vec3(0.3));
            emissive=walls(p,nor,vec3(0.0));

        }else if(res.g<1.9){
            albedo=vec3(0.2,0.2,0.2);
        }
        
        float sampleScale=1.0/16.0;
        vec3 sum=vec3(0.0);
        float iter=0.0;
        for(int i=0;i<16;i++){
            //Choose a random direction along the normal.
            //First, random direction in hemisphere pointing up (this is very approximate, and wrong)
            vec3 rd=vec3(random(iter*20.0),
                         random(dot(p.xy,vec2(18.0,5.0))+iter*55.0),
                         random( dot(p.xy,vec2(-7.0,8.0))+iter*81.0));
            rd=2.0*(rd-vec3(0.5));
            rd.z=abs(rd.z);
            rd=normalize(texrot(rd,nor));
            vec2 res2=intersectFast(p,rd,0.01,10.0);
            vec3 p2=p+res2.r*rd;
            vec3 nor2=normalize(calcNormal(p2));
            if(res2.g<0.9){
                sum+=dot(nor,rd)*walls(p2,nor2,vec3(0.0));
            }
            iter+=1.0;
        }
        
        float gloss=pow(clamp(2.0*cubetex(iChannel2,p.xzy*0.5,nor.xzy).b-0.5,0.0,1.0),0.3);
        for(int i=0;i<4;i++){
            vec3 rd=vec3(random(iter*20.0),
                         random(dot(p.xy,vec2(18.0,5.0))+iter*55.0),
                         random( dot(p.xy,vec2(-7.0,8.0))+iter*81.0));
            rd=2.0*(rd-vec3(0.5));
            rd.z=abs(rd.z);
            rd=texrot(rd,nor);
            //do a hard mix
            rd=normalize(mix(rd,reflect(ci,nor),gloss));
            vec2 res2=intersectFast(p,rd,0.01,10.0);
            vec3 p2=p+res2.r*rd;
            vec3 nor2=normalize(calcNormal(p2));
            if(res2.g<0.9){
                sum+=gloss*walls(p2,nor2,vec3(0.0));
            }
            iter+=1.0;
        }
        
        col=emissive+albedo*sum*sampleScale;
        //col=emissive+4.0*albedo*sum*sampleScale;
        //col=vec3(gloss);
        //test...
        //col=vec3(0.2,0.2,0.2);
        //col=vec3(col.x,col.y,dot(nor,tex2norm(iChannel2,uv,nor)));
        
        col=col;//*calcDirectLighting(p,nor);
    }
    //fragColor=vec4(t/20.0,t/20.0,t/20.0,1.0);
    //return;
    //Nope, not gamma correcting right now because it makes things look bad. Sorry.
    //Okay, I take that back. Maybe. TODO: perform actual radiosity test with real object/camera
    gl_FragColor = vec4(gamma(2.0*col),1.0);
}