#include <stdio.h>
#include <assert.h>
#include "base64.h"

static char *base64 = 
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnoqtstuvwxyz0123456789+/";
/*
Base64将输入按照6bits分割,映射到ASCII段. ⚠️因为24bits是6bits和8bits的最小公倍数,就需要将输入数据填充到24bits的倍数,实际实现的时候这样会较为复杂,本书中采用另一种办法.
如果输入数据的最后一块是1Byte的话,就在后面添加两个’=‘字符.
如果输入数据的最后一块是2Bytes的话,就在后面添加一个‘=’.
如果是三个字节(24bits)的偶数倍(48bits的倍数)的话,就不添加任何字符.
 */
void base64_encode(const unsigned char *input, int len, unsigned char *output)
{
    do
    {
        //处理第一个8bits的高6bits
        //0xFC ： 1111 1100
        *output = base64[ (input[0] & 0xFC) >> 2 ];

        if(len == 1)
        {
            //输入数据长度为1的情况下
            //处理第一个8bits的低2bits
            *output++ = base64[((input[0] & 0x03 ) << 4) ];
            *output++ = '=';//填充‘=’
            *output++ = '=';//填充‘=’
            break;//跳出do while
        }
        //将第一个8bits的低2bits,和第二个8bits的高4bits作为一个6bits处理
        *output++ = base64[((input[0] & 0x03) << 4) | ((input[1] & 0xF0 ) >> 4)];

        if(len == 2)
        {
            //将第二个8bits的低4bits左移动2bits凑够6bits
            *output++ = base64[((input[1] & 0x0F) << 2)];
            *output++ = '=';//填充‘=
            break;
        }

        //常规非尾数据的处理
        *output++ = base64[((input[1] & 0x0F)<<2) | ((input[2] & 0xC0) >> 6 )];
        *output++ = base64[(input[2] & 0x3F)];
        input += 3;
    }
    while(len -= 3);//每次处理3Bytes数据

    *output = '\0';
    //PS: 输出数据的长度是输入数据长度的4/3
}

//–1 条目是非 base64 字符。
static int unbase64[] =
{
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52,
 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, 0, -1, -1, -1,
 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1,
 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1
};

int base64_decode(const unsigned char *input, int len, unsigned char *output)
{
    int out_len = 0, i;

    //输入长度必须是4的倍数
    assert( !(len & 0x03) );

    do 
    {
        for (i=0; i<=3; ++i){
            if(input[0] > 128 || unbase64[input[i]] == -1){
                fprintf(stderr, "invalid character for base64 encoding: %c\n", input[i]);
                return -1;
            }
        }

        *output++ = unbase64[ input [0] ] << 2 | (unbase64 [input[1]] & 0x30) >> 4;
        out_len++;

        if(input[2] != '='){
            *output++ = (unbase64[input[1]] & 0x0F ) << 4 | (unbase64[input[2]] & 0x3C) >> 2;
            out_len++;
        }

        if(input[3] != '='){
            *output++ = (unbase64[ input[ 2 ] ] & 0x03) << 6 | unbase64[ input[3] ];
            out_len++;
        }

        input += 4;

    }while(len -= 4);

    return out_len;
}
