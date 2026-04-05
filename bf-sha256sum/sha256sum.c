/*
 * sha256sum.c — BOF: compute SHA-256 hash
 * Usage: sha256sum <file> [file2 ...]
 * Pure C SHA-256 (no openssl)
 */
#include "bofdefs.h"

static const uint32_t sha256_k[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

#define RR(x,n) (((x)>>(n))|((x)<<(32-(n))))
#define CH(x,y,z) (((x)&(y))^(~(x)&(z)))
#define MAJ(x,y,z) (((x)&(y))^((x)&(z))^((y)&(z)))
#define EP0(x) (RR(x,2)^RR(x,13)^RR(x,22))
#define EP1(x) (RR(x,6)^RR(x,11)^RR(x,25))
#define SIG0(x) (RR(x,7)^RR(x,18)^((x)>>3))
#define SIG1(x) (RR(x,17)^RR(x,19)^((x)>>10))

typedef struct { uint32_t h[8]; uint64_t count; unsigned char buf[64]; } sha256_ctx;

static void sha256_transform(sha256_ctx *c, const unsigned char *data) {
    uint32_t w[64], a,b,cc,d,e,f,g,h;
    for(int i=0;i<16;i++) w[i]=((uint32_t)data[i*4]<<24)|((uint32_t)data[i*4+1]<<16)|((uint32_t)data[i*4+2]<<8)|(uint32_t)data[i*4+3];
    for(int i=16;i<64;i++) w[i]=SIG1(w[i-2])+w[i-7]+SIG0(w[i-15])+w[i-16];
    a=c->h[0];b=c->h[1];cc=c->h[2];d=c->h[3];e=c->h[4];f=c->h[5];g=c->h[6];h=c->h[7];
    for(int i=0;i<64;i++){
        uint32_t t1=h+EP1(e)+CH(e,f,g)+sha256_k[i]+w[i];
        uint32_t t2=EP0(a)+MAJ(a,b,cc);
        h=g;g=f;f=e;e=d+t1;d=cc;cc=b;b=a;a=t1+t2;
    }
    c->h[0]+=a;c->h[1]+=b;c->h[2]+=cc;c->h[3]+=d;c->h[4]+=e;c->h[5]+=f;c->h[6]+=g;c->h[7]+=h;
}

static void sha256_init(sha256_ctx *c) {
    c->h[0]=0x6a09e667;c->h[1]=0xbb67ae85;c->h[2]=0x3c6ef372;c->h[3]=0xa54ff53a;
    c->h[4]=0x510e527f;c->h[5]=0x9b05688c;c->h[6]=0x1f83d9ab;c->h[7]=0x5be0cd19;
    c->count=0;
}

static void sha256_update(sha256_ctx *c, const void *data, size_t len) {
    const unsigned char *p=data;
    size_t off=c->count%64;
    c->count+=len;
    for(size_t i=0;i<len;i++){c->buf[off++]=p[i];if(off==64){sha256_transform(c,c->buf);off=0;}}
}

static void sha256_final(sha256_ctx *c, unsigned char digest[32]) {
    size_t off=c->count%64;
    c->buf[off++]=0x80;
    if(off>56){memset(c->buf+off,0,64-off);sha256_transform(c,c->buf);off=0;}
    memset(c->buf+off,0,56-off);
    uint64_t bits=c->count*8;
    for(int i=7;i>=0;i--) c->buf[56+(7-i)]=(bits>>(i*8))&0xff;
    sha256_transform(c,c->buf);
    for(int i=0;i<8;i++){digest[i*4]=(unsigned char)((c->h[i]>>24)&0xff);digest[i*4+1]=(unsigned char)((c->h[i]>>16)&0xff);digest[i*4+2]=(unsigned char)((c->h[i]>>8)&0xff);digest[i*4+3]=(unsigned char)(c->h[i]&0xff);}
}

void go(char *args, int alen) {
    if (!args || alen <= 0) BOF_ERROR("Usage: sha256sum <file> [file2 ...]");

    datap parser;
    BeaconDataParse(&parser, args, alen);
    char *argv_str = BeaconDataExtract(&parser, NULL);
    if (!argv_str || !*argv_str) BOF_ERROR("Usage: sha256sum <file> [file2 ...]");

    char *saveptr;
    char *file = strtok_r(argv_str, " ", &saveptr);
    while (file) {
        FILE *fp = fopen(file, "rb");
        if (!fp) { BeaconPrintf(CALLBACK_ERROR, "sha256sum: %s: %s\n", file, strerror(errno)); file = strtok_r(NULL, " ", &saveptr); continue; }
        sha256_ctx ctx; sha256_init(&ctx);
        unsigned char buf[4096]; size_t n;
        while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) sha256_update(&ctx, buf, n);
        fclose(fp);
        unsigned char digest[32]; sha256_final(&ctx, digest);
        char hex[65];
        for (int i=0;i<32;i++) snprintf(hex+i*2, 3, "%02x", digest[i]);
        BeaconPrintf(CALLBACK_OUTPUT, "%s  %s\n", hex, file);
        file = strtok_r(NULL, " ", &saveptr);
    }
}
