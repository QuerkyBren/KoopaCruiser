#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <3ds.h>

PrintConsole topScreen, bottomScreen;
char *url;
int dlCounter;
httpcContext context;
Result ret = 0;

Result http_download(httpcContext *context)//This error handling needs updated with proper text printing once ctrulib itself supports that.
{
	ret=0;
	//u8* framebuf_top;
	u32 statuscode=0;
	//u32 size=0;
	u32 contentsize=0;
	u8 *buf;

	ret = httpcBeginRequest(context);
	if(ret!=0)return ret;

	ret = httpcGetResponseStatusCode(context, &statuscode, 0);
	if(ret!=0)return ret;

	//printf("http status code: %i\n", statuscode);

	if(statuscode!=200){
		printf("status code not 200, it was %i", statuscode);
		gfxFlushBuffers();
		return -2;
	}

	ret=httpcGetDownloadSizeState(context, NULL, &contentsize);
	if(ret!=0)return ret;
	unsigned char *buffer = (unsigned char*)malloc(contentsize+1);

	consoleSelect(&topScreen);

	printf("HTTP status code: %i\n", statuscode);
	printf("%i bytes\n", contentsize);
	gfxFlushBuffers();

	buf = (u8*)malloc(contentsize);
	if(buf==NULL)return -1;
	memset(buf, 0, contentsize);


	ret = httpcDownloadData(context, buffer, contentsize, NULL);
	if(ret!=0)
	{
		free(buf);
		return ret;
	}

	consoleSelect(&bottomScreen);
	printf("%s", buffer);

	free(buf);

	return 0;
}

Result http_downloadsave(httpcContext *context)//This error handling needs updated with proper text printing once ctrulib itself supports that.
{
    ret = 0;
    //u8* framebuf_top;
    u32 statuscode=0;
    //u32 size=0;
    u32 contentsize=0;
    u8 *buf;

    ret = httpcBeginRequest(context);
    if(ret!=0)return ret;

    ret = httpcGetResponseStatusCode(context, &statuscode, 0);
    if(ret!=0)return ret;

    //printf("http status code: %i\n", statuscode);

    if(statuscode!=200){
        printf("status code not 200, it was %i\n", statuscode);
        gfxFlushBuffers();
        return -2;
    }

    ret=httpcGetDownloadSizeState(context, NULL, &contentsize);
    if(ret!=0)return ret;
    unsigned char *buffer = (unsigned char*)malloc(contentsize+1);

    consoleSelect(&topScreen);

    printf("HTTP status code: %i\n", statuscode);
    printf("%i bytes\n", contentsize);
    gfxFlushBuffers();

    buf = (u8*)malloc(contentsize);
    if(buf==NULL)return -1;
    memset(buf, 0, contentsize);


    ret = httpcDownloadData(context, buffer, contentsize, NULL);
    if(ret!=0)
    {
        free(buf);
        return ret;
    }

    /*size = contentsize;
    if(size>(240*400*3*2))size = 240*400*3*2;
    framebuf_top = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    memcpy(framebuf_top, buf, size);
    gfxFlushBuffers();
    gfxSwapBuffers();
    framebuf_top = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    memcpy(framebuf_top, buf, size);
    gfxFlushBuffers();
    gfxSwapBuffers();
    gspWaitForVBlank();*/

		printf("Got file\n");

		char filename[32];
		FILE *dlfile;

		char *fnameformat;

		snprintf(filename, sizeof(char) * 32, "koopadl%i.txt", ++dlCounter);
		printf("Saving to %s\n", filename);

    dlfile = fopen(filename, "w");
    fwrite(buffer, 1, contentsize, dlfile);
    fclose(dlfile);

    consoleSelect(&topScreen);
		printf("Saved to %s\n", filename);

    //printf("%s", buffer);

    free(buf);

    return 0;
}

void downloadfile()
{
    consoleSelect(&topScreen);

    printf("Downloading %s\n",url);
    gfxFlushBuffers();

    ret = httpcOpenContext(&context, url , 0);
    gfxFlushBuffers();

    if(ret==0)
    {
        ret=http_downloadsave(&context);
        gfxFlushBuffers();
        httpcCloseContext(&context);
    }
}

int main()
{
	const char build_time[] = __DATE__ " " __TIME__;

	dlCounter = 0;

	ret = 0;

	gfxInitDefault();

	consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM, &bottomScreen);
	consoleSelect(&topScreen);
	printf("Koopa Cruiser\n");
	printf("Version: %s\n", VERSION);
	printf("main.c modified: %s\n", __TIMESTAMP__);
	printf("Built: %s\n\n", build_time);
	printf("Press X to save the page\n");
	printf("Press START to exit.\n\n");
	gfxFlushBuffers();

	httpcInit();

	//Change this to your own URL.
	url = "http://jsa.paperplane.io/index.html";

	printf("Loading %s\n\n",url);
	gfxFlushBuffers();

	ret = httpcOpenContext(&context, url , 0);
	//printf("return from httpcOpenContext: %"PRId32"\n",ret);
	gfxFlushBuffers();

	if(ret==0)
	{
		ret=http_download(&context);
		//printf("return from http_download: %"PRId32"\n",ret);
		gfxFlushBuffers();
		httpcCloseContext(&context);
	}

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		// Your code goes here

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		if (kDown & KEY_X)
	    {
	        downloadfile();
	    }

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	// Exit services
	httpcExit();
	gfxExit();
	return 0;
}
