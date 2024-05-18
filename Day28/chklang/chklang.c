#include "apilib.h"

void HariMain(void)
{
	int langmode = api_getlang();
	static char s1[23] = {	/* 日文Shift-JIS */
		0x93, 0xfa, 0x96, 0x7b, 0x8c, 0xea, 0x83, 0x56, 0x83, 0x74, 0x83, 0x67,
		0x4a, 0x49, 0x53, 0x83, 0x82, 0x81, 0x5b, 0x83, 0x68, 0x0a, 0x00
	};
	static char s2[17] = {	/* 日文EUC模式 */
		0xc6, 0xfc, 0xcb, 0xdc, 0xb8, 0xec, 0x45, 0x55, 0x43, 0xa5, 0xe2, 0xa1,
		0xbc, 0xa5, 0xc9, 0x0a, 0x00
	};
	static char s3[20] = {
		0xce, 0xd2, 0x20, 0xca, 0xc7, 0xa1, 0xa2, 0xa1, 0xa3, 0x0a, 0x00
	};
	int i;char j;
	if (langmode == 0) {
		api_putstr0("English ASCII mode\n");
	}
	if (langmode == 1) {
		api_putstr0(s1);
	}
	if (langmode == 2) {
		api_putstr0(s2);
	}
	if (langmode == 3) {
		api_putstr0("Chinese 中国 !\n");
		api_putstr0("BY 博客园YJLAUgus \n");
		api_putstr0("中国字系统测试!");
		api_putstr0(s3);
		for(i=0xa1;i<0xcc;i++)
		{
			s3[0]=0xa1;
			j=i;
			s3[1]=j;
			s3[2]=0x00;
			api_putstr0(s3);
		}
	}
	api_end();
}
