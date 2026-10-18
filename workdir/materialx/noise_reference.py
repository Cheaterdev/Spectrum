# Independent port of MaterialX/OSL noise (genglsl/lib/mx_noise.glsl) to Python.
import math, struct
M = 0xFFFFFFFF
def rotl(x,k): return ((x<<k)|(x>>(32-k))) & M
def bjfinal(a,b,c):
    c^=b; c=(c-rotl(b,14))&M
    a^=c; a=(a-rotl(c,11))&M
    b^=a; b=(b-rotl(a,25))&M
    c^=b; c=(c-rotl(b,16))&M
    a^=c; a=(a-rotl(c,4))&M
    b^=a; b=(b-rotl(a,14))&M
    c^=b; c=(c-rotl(b,24))&M
    return c
def hash_int(*v):
    n=len(v); s=(0xdeadbeef+(n<<2)+13)&M
    a=b=c=s
    a=(a+(v[0]&M))&M
    if n>1: b=(b+(v[1]&M))&M
    if n>2: c=(c+(v[2]&M))&M
    return bjfinal(a,b,c)
def f32(x): return struct.unpack('f',struct.pack('f',x))[0]
def floorfrac(x):
    i=math.floor(x); return i, f32(x-i)
def fade(t): return t*t*t*(t*(t*6-15)+10)
def grad2(h,x,y):
    h&=7; u=x if h<4 else y; v=2*(y if h<4 else x)
    return (-u if h&1 else u)+(-v if h&2 else v)
def grad3(h,x,y,z):
    h&=15; u=x if h<8 else y
    v=y if h<4 else (x if h in (12,14) else z)
    return (-u if h&1 else u)+(-v if h&2 else v)
def perlin2(x,y):
    X,fx=floorfrac(x); Y,fy=floorfrac(y); u=fade(fx); v=fade(fy)
    g=lambda dx,dy: grad2(hash_int(X+dx,Y+dy), fx-dx, fy-dy)
    s1=1-u
    r=(1-v)*(g(0,0)*s1+g(1,0)*u)+v*(g(0,1)*s1+g(1,1)*u)
    return 0.6616*r
def perlin3(x,y,z):
    X,fx=floorfrac(x); Y,fy=floorfrac(y); Z,fz=floorfrac(z)
    u=fade(fx); v=fade(fy); w=fade(fz)
    g=lambda dx,dy,dz: grad3(hash_int(X+dx,Y+dy,Z+dz), fx-dx, fy-dy, fz-dz)
    s1=1-u; t1=1-v; r1=1-w
    r=(r1*(t1*(g(0,0,0)*s1+g(1,0,0)*u)+v*(g(0,1,0)*s1+g(1,1,0)*u))+
       w*(t1*(g(0,0,1)*s1+g(1,0,1)*u)+v*(g(0,1,1)*s1+g(1,1,1)*u)))
    return 0.9820*r
def cell2(x,y): return hash_int(math.floor(x),math.floor(y))/4294967295.0
def cell3(x,y,z): return hash_int(math.floor(x),math.floor(y),math.floor(z))/4294967295.0
pts2=[(0.3,0.7),(1.5,2.25),(-0.8,3.1),(10.2,-4.6),(0.5,0.5)]
pts3=[(0.3,0.7,0.1),(1.5,2.25,-3.5),(-0.8,3.1,0.9),(10.2,-4.6,7.7),(0.5,0.5,0.5)]
print("perlin2",[round(perlin2(*p),6) for p in pts2])
print("perlin3",[round(perlin3(*p),6) for p in pts3])
print("cell2",[round(cell2(*p),6) for p in pts2])
print("cell3",[round(cell3(*p),6) for p in pts3])

def cell_vec3_2(x,y):
    ix,iy=math.floor(x),math.floor(y)
    return [hash_int(ix,iy,k)/4294967295.0 for k in range(3)]
def worley2(px,py,jitter=1.0):
    X,fx=floorfrac(px); Y,fy=floorfrac(py)
    best=1e6
    for x in (-1,0,1):
        for y in (-1,0,1):
            off=cell_vec3_2(x+X,y+Y)[:2]
            cx=x+(off[0]-0.5)*jitter+0.5; cy=y+(off[1]-0.5)*jitter+0.5
            d=(cx-fx)**2+(cy-fy)**2
            best=min(best,d)
    return math.sqrt(best)
def fractal2(x,y,octaves=3,lac=2.0,dim=0.5):
    r=0;a=1
    for _ in range(octaves):
        r+=a*perlin2(x,y); a*=dim; x*=lac; y*=lac
    return r
def fractal3(x,y,z,octaves=3,lac=2.0,dim=0.5):
    r=0;a=1
    for _ in range(octaves):
        r+=a*perlin3(x,y,z); a*=dim; x*=lac; y*=lac; z*=lac
    return r
print("worley2",[round(worley2(*p),6) for p in pts2])
print("fractal2",[round(fractal2(*p),6) for p in pts2])
print("fractal3",[round(fractal3(*p),6) for p in pts3])
