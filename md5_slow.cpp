#include<iostream>
#include<string>
#include<fstream>
#include<sstream>

#define LEFT_ROTATE_SHIFT(x, n)     (((x) << (n)) | ((x) >> (32-(n))))  
#define F(x, y, z)                  (((x) & (y)) | (~(x) & (z)))        // (X AND Y) OR ((NOT X) AND Z)
#define G(x, y, z)                  (((x) & (z)) | ((y) & ~(z)))        // (X AND Z)
#define H(x, y, z)                  ( (x) ^ (y) ^ (z))
#define I(x, y, z)                  ( (y) ^ ((x) | ~(z)))
#define A                           0x67452301
#define B                           0xefcdab89
#define C                           0x98badcfe
#define D                           0x10325476
#define HEX_NUM_CHAR_MAX            16
#define HEX_NUM_UPPER_CHAR_SET      "0123456789ABCDEF"
#define HEX_NUM_LOWER_CHAR_SET      "0123456789abcdef"
#define HEX_NUM_CHAR_SET            HEX_NUM_LOWER_CHAR_SET

const static char msc_hexChars[] = HEX_NUM_CHAR_SET;

// 常量ti unsigned int(abs(sin(i+1))*(2pow32)), 优先记录结果加快运算速度
const unsigned int k[] = {
        0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee, 0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
        0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be, 0x6b901122,0xfd987193,0xa679438e,0x49b40821,
        0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa, 0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
        0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed, 0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
        0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c, 0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
        0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05, 0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
        0xf4292244,0x432aff97,0xab9423a7,0xfc93a039, 0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
        0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1, 0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391,
    };

//向左位移数
const unsigned int s[] = {
        7,  12, 17, 22,     7,  12, 17, 22,     7,  12, 17, 22,     7, 12,  17, 22,
        5,  9,  14, 20,     5,  9,  14, 20,     5,  9,  14, 20,     5,  9,  14, 20,
        4,  11, 16, 23,     4,  11, 16, 23,     4,  11, 16, 23,     4,  11, 16, 23,
        6,  10, 15, 21,     6,  10, 15, 21,     6,  10, 15, 21,     6,  10, 15, 21,
    };

void mainLoop(unsigned int M[], unsigned int temp[]) {
    unsigned int f, g, tmp;
    unsigned int a = temp[0];
    unsigned int b = temp[1];
    unsigned int c = temp[2];
    unsigned int d = temp[3];
    for (unsigned int i = 0; i < 64; ++i) {
        if (i < 16) {
            f = F(b, c, d);
            g = i;
        } else if (i < 32) {
            f = G(b, c, d);
            g = (5 * i + 1) % 16;
        } else if (i < 48) {
            f = H(b, c, d);
            g = (3 * i + 5) % 16;
        } else {
            f = I(b, c, d);
            g = (7 * i) % 16;
        }
        tmp = d;
        d = c;
        c = b;
        b = b + LEFT_ROTATE_SHIFT((a + f + k[i] + M[g]), s[i]);
        a = tmp;
    }
    temp[0] += a;
    temp[1] += b;
    temp[2] += c;
    temp[3] += d;
}

/*
 * 填充函数
 * 处理后应满足bits≡448(mod512),字节就是bytes≡56（mode64)
 * 填充方式为先加一个1,其它位补零
 * 最后加上64位的原来长度
 */
unsigned int* add(std::string& str, unsigned int * out_len) {
    unsigned int len = str.length();
    unsigned int num = ((len + 8) / 64) + 1;
    const int strlength = num * 16;
    unsigned int *strByte = new unsigned int[strlength];
    for (unsigned int i = 0; i < strlength; ++i) {
        strByte[i] = 0;
    }
    for (unsigned int i = 0; i < len; ++i) {
        strByte[i >> 2] |= (str[i]) << ((i & 0x03) << 3);
    }
    strByte[len >> 2] |= 0x80 << ((len & 0x03) << 3);
    strByte[strlength - 2]=len * 8;
    *out_len = strlength;
    return strByte;
}

void DecToHex(unsigned int num, char * out) {
    out[8] = 0;
    for(int i = 0; i < 4; ++i) {
        unsigned int byte_val = (num >> (i << 3)) & 0xff;
        for (int j = 0; j < 2; ++j) {
            out[i << 1 | (1 - j)] = msc_hexChars[byte_val & 0x0f];
            byte_val >>= 4;
        }
    }
}

std::string getMD5(std::string& source) {
    unsigned int temp[] = { A, B, C, D };
    unsigned int strlength = 0;
    unsigned int *strByte = add(source, &strlength);
    for (unsigned int i = 0, len = strlength / 16; i < len; ++i){
        unsigned int num[16];
        for (unsigned int j = 0; j < 16; ++j){
            num[j] = strByte[(i << 4) + j];
            //std::cout<<(num[j])<<' ';
        }
        mainLoop(num, temp);
    }
    delete[] strByte;

    char hex[32 + 1] = { 0 };
    for (int i = 0; i < 4; ++ i) {
        DecToHex(temp[i], hex + (i << 3));
    }
    return std::string(hex);
}


// 测试代码
using namespace std;
int main(int argc, char *argv[]) {
    if (argc == 1) {
      std::cout<<"No input file"<<std::endl;
      return -1;
    }
    
    std::ifstream ifile(argv[1]);
    
    std::ostringstream tmp;
    tmp << ifile.rdbuf();
    std::string val = tmp.str();
    cout<<val.length()<<endl;
    //std::string content = ifile.read()
    std::string md5 = getMD5(val);
    
    std::cout << md5 << " : " << argv[1] << std::endl;
    return 0;
}
