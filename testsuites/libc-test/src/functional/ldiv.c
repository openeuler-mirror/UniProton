#include "test.h"
#include <stdio.h>
#include <stdlib.h>

static struct {
    long x, y, div, mod;
} t1[] = {
2147483647, 2147483646, 1, 1,
2147483647, 2147483647, 1, 0,
2147483647, 536870908, 4, 15,
-2147483648, 2147483647, -1, -1,
-2147483647, -1, 2147483647, 0,
2147483647, 1, 2147483647, 0,
};

static struct {
    long long x, y, div, mod;
} t2[] = {
// 正数
0x723f4114006c08c7LL, 0x817de530db2b43fcLL, 0x0LL, 0x723f4114006c08c7LL,
0x51ffcc7cdc989d43LL, 0x36be8bd6746b70e4LL, 0x1LL, 0x1b4140a6682d2c5fLL,
0x57bf9128512fe829LL, 0x197b3858155d498dLL, 0x3LL, 0xb4de82011180b82LL,
0x4ed5264cf7092ec5LL, 0xde40d1e15ef3e74LL, 0x5LL, 0x960e4b6895cf681LL,
0x6b93ffce49f1a4b3LL, 0x3583d1f9702ee03LL, 0x20LL, 0x8c5bdb6993e453LL,
0x138aefcc98ce5d19LL, 0x117002fa7600b11LL, 0x11LL, 0x103eca27b6da0f8LL,
0x71c4b06e463912b5LL, 0x1c286ad9e8f5229LL, 0x40LL, 0x1230506a2648875LL,
0x1428f04bd490ea11LL, 0x9d97f29a897c93LL, 0x20LL, 0x75f1f8836157b1LL,
0x35256c76832705a7LL, 0xa962f1a447dcd7LL, 0x50LL, 0x3680f32cb20277LL,
0x2969e82bd9347f2dLL, 0x723d68574d4156LL, 0x5cLL, 0x5bd6ac79710445LL,
0x695b8d33ef342e09LL, 0x3ed1fe1a998fe3LL, 0x1adLL, 0x15a6615bde0ea2LL,
0x46b4dd1e06367a5fLL, 0xa04e70622e4e8LL, 0x70eLL, 0x64750bc0b9dafLL,
0x1e59cc2ac508f85bLL, 0xeb15ae6d4d7f9LL, 0x210LL, 0xc00aeae0b86cbLL,
0x7036f5ad7cbc5e17LL, 0xa09d3bfcf72cfLL, 0xb2dLL, 0x72236db564ab4LL,
0x27a2e280bcf990cfLL, 0x389aa0c0b0cc0LL, 0xb34LL, 0x9d71d12eb9cfLL,
0x1e032f04a5372e95LL, 0x63c2a1d58710LL, 0x4d04LL, 0x154ce4414255LL,
0x3a1a5659908495cbLL, 0x279dcd85418aLL, 0x17775LL, 0x132c6f9c7bb9LL,
0x6700daeeb87a770dLL, 0xeca7ab1aa93LL, 0x6f6c5LL, 0x70d9466f1eeLL,
0x7a08fe1d98b4dae1LL, 0x6bced9c0c15LL, 0x121c89LL, 0x40c856617a4LL,
0x34435992a5c9c2f7LL, 0x4f4a94c109fLL, 0xa8bc9LL, 0x94c5d46120LL,
0x6fd0027468f1dcfdLL, 0x597186b0153LL, 0x140060LL, 0x16f26555dddLL,
0x4fe37c1db1619a73LL, 0x47a0c30bd15LL, 0x11d861LL, 0x5964fb3d7eLL,
0x77aa77f86d07c8d9LL, 0x3a39cf03d65LL, 0x20e21cLL, 0x37f7fede7cdLL,
0x2509ddea3a648dd1LL, 0xec762d81bcLL, 0x281955LL, 0xc0463d1e65LL,
0x1dd4fe261b4adeedLL, 0x2736e25406LL, 0xc2bfefLL, 0x1354c1f353LL,
0x480258f92fc38de3LL, 0x2599b52bb0LL, 0x1ea450cLL, 0x2879f11a3LL,
0x5a3257b1114109c9LL, 0x2978f9f1aaLL, 0x22cc30aLL, 0x1317311b25LL,
0x3c2c319ca8612a65LL, 0x73fc01eceLL, 0x84d0088LL, 0x3165accf5LL,
0x4f6034e74a16561bLL, 0x1f29d53707LL, 0x28c0daaLL, 0xd88e07075LL,
0x206665a7072f1cc1LL, 0xda87e7ceaLL, 0x25f48c1LL, 0xd3ddb2057LL,
0x100c559d7db417d7LL, 0xb907ebbc2LL, 0x1634188LL, 0xa2eae16c7LL,
0x64c5f83691b47cddLL, 0x5aced6ebbLL, 0x11c17fb7LL, 0x344109030LL,
0x32a812777eaf7d53LL, 0x1cb63fe4fLL, 0x1c3a9675LL, 0xb113f938LL,
0x67478d96865ca6b9LL, 0x142fa03aLL, 0x51dcb463dLL, 0x11359ce7LL,
0x71024e740deb428fLL, 0x142d3885LL, 0x599d9edd5LL, 0x13b1ae6LL,
0x52c78160b090b655LL, 0xd02101c6LL, 0x65d1b205LL, 0x1c0a0177LL,
0x74babc1be2ed9c47LL, 0x22eda9a6LL, 0x3578b1967LL, 0x189b247dLL,
0x7c5cbf2dfc1db6cdLL, 0x5f09c060LL, 0x14efd44d4LL, 0x5210e74dLL,
0x7c046071c1ac68c3LL, 0x3696c8e6LL, 0x24596d86bLL, 0x26060a1LL,
0x3f4c0514b0df5e45LL, 0xf2c3810LL, 0x42bf84d39LL, 0x3aa12b5LL,
0x23fb9839e8358cbdLL, 0x24deb54LL, 0xf9d714151LL, 0xb9c329LL,
0x2005d5de30015033LL, 0x47c06dbLL, 0x7240bccbaLL, 0x104d115LL,
0x67d59c29e076f499LL, 0x179f009LL, 0x465554ac22LL, 0x10b0767LL,
0x32d2dd34369c836fLL, 0x13d3fbfLL, 0x2902f2fb54LL, 0x7553c3LL,
0x3960c3c99fdc2235LL, 0x1b808baLL, 0x21618743cdLL, 0x11e7743LL,
0x343bad5adfa9726bLL, 0xeef444LL, 0x37f58c51a6LL, 0x3d8a53LL,
0x7a4aadd7b4e5f191LL, 0x129c9LL, 0x6921bb5a2a53LL, 0x6b66LL,
0x1b285999316afeadLL, 0x115477LL, 0x1912cf6611eLL, 0x801bbLL,
0x18e6a86b0473a589LL, 0x50a12LL, 0x4f0fabc67d4LL, 0x210a1LL,
0x76743abdfb91f081LL, 0xd5888LL, 0x8e0303c479cLL, 0x245a1LL,
0x63cc9c23f0ed0c9dLL, 0x6c1e5LL, 0xec4d5841041LL, 0x38178LL,
0x7ad70f846e8f1313LL, 0x7fdf5LL, 0xf5ecec69bc9LL, 0x756b6LL,
0x60de3d71574eb279LL, 0x6ea3LL, 0xe02421997a61LL, 0x18b6LL,
0x227f92ef6daab68dLL, 0x15ecLL, 0x192dda5d5ed25LL, 0xf71LL,
0x2e588bdb751a66bfLL, 0x229cLL, 0x156d025c70d97LL, 0x10bbLL,
0x224f627be76a8261LL, 0x4f4LL, 0x6ed4d3882b567LL, 0x35LL,
0x300d1ab91bd0b677LL, 0xe9cLL, 0x34a002fb76e63LL, 0x823LL,
0x2a63d80e0c52fc7dLL, 0x32LL, 0xd90970ebc4383fLL, 0x2fLL,
0x2b5dc22562dbe059LL, 0x30aLL, 0xe45055015fff5LL, 0x1c7LL,
0x4a7fd1078807d52fLL, 0x18dLL, 0x300a32f60677d4LL, 0x16bLL,
0x41a01ee8ab0849f5LL, 0x13cLL, 0x352a3971f57e9dLL, 0x29LL,
0x723bacc76bd51551LL, 0x16LL, 0x53142091089af83LL, 0xfLL,
0x11593d6b3f54de6dLL, 0x63LL, 0x2cdc6b1a7f9078LL, 0x5LL,
0x756c82d6f7069963LL, 0x5cLL, 0x146bea3ba565525LL, 0x17LL,
0x6884fa0a8f0c99e5LL, 0x12LL, 0x5ce7fab40d6088cLL, 0xdLL,
0x5052a2953c528441LL, 0x7LL, 0xb7984f0bf79809bLL, 0x4LL,
0x58dd1583185ecb57LL, 0x9LL, 0x9dfad0e90ee1697LL, 0x8LL,
0x4b21d01617167e39LL, 0x2LL, 0x2590e80b0b8b3f1cLL, 0x1LL,
0x269f4f4baaaf287LL, 0x1aed2ad9daf0LL, 0x16f3LL, 0x426550f80b7LL,
0x16917d5f9fde38bLL, 0xfb1566c7LL, 0x17029e0LL, 0x1bbe166bLL,
0x60ab67a4e5aeabbLL, 0x1bf7LL, 0x374f26f3e3edLL, 0x210LL,
0xffd2ad0e77b73dbLL, 0x146f14LL, 0xc8500600a3LL, 0xba1fLL,
0x296f8d2c76a0901LL, 0xf65628b31b01LL, 0x2b0LL, 0xf14566117651LL,
0x47811fa5f00f74dLL, 0x3d98e7d3fcd5d5c5LL, 0x0LL, 0x47811fa5f00f74dLL,

// 负数
0xc2eeb030bcff9197LL, 0x7a4e8LL, 0xFFFFF802DEB3F2A4LL, 0xFFFFFFFFFFFE9CF7LL,
0xc072e76ad59cf1afLL, 0x3a786701dLL, 0xFFFFFFFFEE9C12FDLL, 0xFFFFFFFC8A321B06LL,
};

int ldiv_test(void)
{
    int i;
    long x, y;
    ldiv_t ldivResult;

    if (sizeof(long) == 4) {
        for (i = 0; i < sizeof t1/sizeof *t1; i++) {
            x = t1[i].x;
            y = t1[i].y;
            ldivResult = ldiv(x, y);

            if (ldivResult.quot != t1[i].div)
            {
                printf("div %ld/%ld want %ld got %ld\n", x, y, t1[i].div, ldivResult.quot);
                t_status = 1;
            }

            if (ldivResult.rem != t1[i].mod)
            {
                printf("mod %ld%%%ld want %ld got %ld\n", x, y, t1[i].mod, ldivResult.rem);
                t_status = 1;
            }
        }
    } else if (sizeof(long) == 8) {
        for (i = 0; i < sizeof t2/sizeof *t2; i++) {
            x = t2[i].x;
            y = t2[i].y;
            ldivResult = ldiv(x, y);

            if (ldivResult.quot != t2[i].div)
            {
                printf("div %ld/%ld want %lld got %lld\n", x, y, t2[i].div, ldivResult.quot);
                t_status = 1;
            }
            if (ldivResult.rem != t2[i].mod)
            {
                printf("mod %ld%%%ld want %lld got %lld\n", x, y, t2[i].mod, ldivResult.rem);
                t_status = 1;
            }
        }
    } else {
        printf("sizeof(long) == %d, not implemented\n", (int)sizeof(long));
        t_status = 1;
    }

    return t_status;
}
