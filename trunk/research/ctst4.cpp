#include "stdio.h"

int main(int argc, char** argv)
{
	// opening /proc/blinkbit thingy if it exist
	FILE *fbb = fopen("/proc/blinkbit", "wb");
	if (fbb) {
			fputc('0', fbb); // disable blinking
			fclose(fbb);
	}

	unsigned short int image[4000];
	unsigned char pal[48];

	FILE *fp = fopen(argv[1], "rb");
	if (!fp) {
			printf("Error: %s not found(%d)\n", argv[1], 0);
			return 0;
	}

	fread(image, sizeof(*image), 4000, fp);
	fread(pal, sizeof(*pal), 48, fp);

	fclose(fp);

	if (true) {
		for (int i = 1; i < 16; ++i) {
			char buf[21];
			int r = pal[i*3 + 0];
			int g = pal[i*3 + 1];
			int b = pal[i*3 + 2];
			snprintf(buf, 20, "\033]P%1x%02x%02x%02x", i, r, g, b);
			buf[20] = 0;
			printf("%s\n", buf);
		}
	}

	for (int y =0; y < 50; ++y)
	{
		for (int x =0; x < 80; ++x)
		{
			unsigned char ch = image[y*80 + x] & 0xFF;
			unsigned char at = image[y*80 + x] >> 8;
			printf("%c", ch);
		}
		printf("\n");
	}


	return 0;
}
