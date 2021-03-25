#pragma once

#include <windows.h>
#include <math.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#pragma warning(disable : 4996)

namespace screen_sync {
	typedef struct _NODE {
		// int64_t pts, dts;
		int size;
		uint8_t* buf;
		_NODE* next;
	} NODE, * PNODE;

	class Encoder {
		private:
			AVCodec* encoder = NULL;
			AVCodecContext* enc_context = NULL;
			AVFrame* enc_frame = NULL;
			AVPacket* pkt = NULL;
			SwsContext* sws_ctx = NULL;
			PNODE cnode = NULL;
		public:
			PNODE head = NULL;
			Encoder();
			Encoder(int width, int height, int fps);
			~Encoder();
			void encodeRgb24(uint8_t* rgb24);
			void flush();
			void encode(AVFrame* frame);
	};

	class Decoder {
		private:
			AVCodec* decoder = NULL;
			AVCodecContext* dec_context = NULL;
			AVFrame* dec_frame = NULL;
			AVPacket* dec_pkt = NULL;
			SwsContext* sws_ctx = NULL;
			PNODE cnode = NULL;
		public:
			PNODE head = NULL;
			Decoder();
			Decoder(int width, int height);
			~Decoder();
			void decodeRgb24(PNODE node);
			void flush();
			void decode(AVPacket* pkt);
	};

	class ScreenCap {
		private:
			HDC hdcScreen, hdc;
			HWND hwnd;
			HBITMAP bitmap;
			HGDIOBJ gdiObj;
			BITMAPINFOHEADER bmi = { 0 };
			BYTE* tmpScreenData = NULL;
		public:
			int sw, sh;
			ScreenCap();
			~ScreenCap();
			void desktopshot(BYTE* screenData);
	};
}