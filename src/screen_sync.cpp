#include "screen_sync.h"

namespace screen_sync {
	Encoder::Encoder() {}
	Encoder::Encoder(int width, int height, int fps) {
		encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
		if (!encoder) {
			printf("encoder avcodec_find_encoder_by_name fail\n");
			exit(0);
		}

		enc_context = avcodec_alloc_context3(encoder);
		if (!enc_context) {
			printf("encoder avcodec_alloc_context3 fail\n");
			exit(0);
		}
		//enc_context->bit_rate = 5 * 1024 * 1024;
		enc_context->bit_rate = floor((width / 1024.0) * (height / 1024.0) * 3.0) * 1024 * 1024;
		// printf("%d\n", enc_context->bit_rate);
		enc_context->width = width;
		enc_context->height = height;
		enc_context->time_base = { 1,fps };
		enc_context->framerate = { fps, 1 };
		enc_context->gop_size = 5;
		enc_context->max_b_frames = 0;
		enc_context->pix_fmt = AV_PIX_FMT_YUV420P;
		//enc_context->thread_count = 4;
		av_opt_set(enc_context->priv_data, "preset", "ultrafast", 0);
		av_opt_set(enc_context->priv_data, "tune", "zerolatency", 0);
			
		int ret = avcodec_open2(enc_context, encoder, NULL);
		if (ret < 0) {
			printf("encoder avcodec_open2 fail\n");
			exit(0);
		}

		enc_frame = av_frame_alloc();
		if (!enc_frame) {
			printf("encoder av_frame_alloc fail\n");
			exit(0);
		}
		enc_frame->format = enc_context->pix_fmt;
		enc_frame->width = enc_context->width;
		enc_frame->height = enc_context->height;
		ret = av_frame_get_buffer(enc_frame, 0);
		if (ret < 0) {
			printf("av_frame_get_buffer\n");
			exit(0);
		}

		pkt = av_packet_alloc();
		if (!pkt) {
			printf("encoder av_packet_alloc fail\n");
			exit(0);
		}
	};
	Encoder::~Encoder() {
		sws_freeContext(sws_ctx);
		avcodec_free_context(&enc_context);
		av_frame_free(&enc_frame);
		av_packet_free(&pkt);

		PNODE tmp_node;
		cnode = head;
		while (cnode != NULL) {
			free(cnode->buf);
			tmp_node = cnode;
			cnode = cnode->next;
			free(tmp_node);
		}
	}
	void Encoder::encodeRgb24(uint8_t* rgb24) {
		sws_ctx = sws_getCachedContext(sws_ctx, 
			enc_context->width, enc_context->height, AV_PIX_FMT_RGB24, 
			enc_context->width, enc_context->height, enc_context->pix_fmt, SWS_BILINEAR, NULL, NULL, NULL);
			
		uint8_t* indata[AV_NUM_DATA_POINTERS] = { 0 };
		indata[0] = rgb24;
		int inlinesize[AV_NUM_DATA_POINTERS] = { 0 };
		inlinesize[0] = enc_context->width * 3;
		sws_scale(sws_ctx, indata, inlinesize, 0, enc_context->height, enc_frame->data, enc_frame->linesize);

		encode(enc_frame);
	}
	void Encoder::flush() {
		encode(NULL);
	}
	void Encoder::encode(AVFrame* frame) {
		int ret = avcodec_send_frame(enc_context, frame);
		if (ret < 0) {
			printf("encode avcodec_send_frame fail\n");
			exit(0);
		}
		while (ret >= 0) {
			ret = avcodec_receive_packet(enc_context, pkt);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
				return;
			}
			else if (ret < 0) {
				printf("encoding error\n");
				exit(0);
			}
			if (frame) {
				printf("pict_type=%2d", frame->pict_type);
			}
			//printf("Write packet pts %3lld dts %3lld (size=%5d)\n", pkt->pts, pkt->dts, pkt->size);

			PNODE pnode = (PNODE)malloc(sizeof(NODE));
			if (pnode == NULL) {
				printf("malloc fail\n");
				return;
			}
			memset(pnode, 0, sizeof(NODE));
			if (head == NULL) {
				head = pnode;
				cnode = pnode;
			}
			else {
				cnode->next = pnode;
				cnode = pnode;
			}
			pnode->buf = (uint8_t*)malloc(pkt->size);
			if (pnode->buf == NULL) {
				exit(0);
			}
			memcpy(pnode->buf, pkt->data, pkt->size);
			pnode->size = pkt->size;
			// pnode->pts = pkt->pts;
			// pnode->dts = pkt->dts;
			
			av_packet_unref(pkt);
		}
		//av_packet_free(&pkt);
	}
	
	Decoder::Decoder() {}
	Decoder::Decoder(int width, int height) {
		decoder = avcodec_find_decoder(AV_CODEC_ID_H264);
		if (!decoder) {
			printf("decoder avcodec_find_encoder_by_name fail\n");
			exit(0);
		}
		dec_context = avcodec_alloc_context3(decoder);
		if (!dec_context) {
			printf("decoder avcodec_alloc_context3 fail\n");
			exit(0);
		}
		dec_context->width = width;
		dec_context->height = height;
		dec_context->pix_fmt = AV_PIX_FMT_RGB24;
		//dec_context->thread_count = 8;
		
		int ret = avcodec_open2(dec_context, decoder, NULL);
		if (ret < 0) {
			printf("decoder avcodec_open2 fail\n");
			exit(0);
		}
		
		dec_frame = av_frame_alloc();
		if (!dec_frame) {
			printf("decoder av_frame_alloc fail\n");
			exit(0);
		}
		dec_frame->width = width;
		dec_frame->height = height;
		dec_frame->format = dec_context->pix_fmt;

		dec_pkt = av_packet_alloc();
		if (!dec_pkt) {
			printf("decoder av_packet_alloc fail\n");
			exit(0);
		}
	}
	Decoder::~Decoder() {
		sws_freeContext(sws_ctx);
		avcodec_free_context(&dec_context);
		av_frame_free(&dec_frame);
		av_packet_free(&dec_pkt);

		PNODE tmp_node;
		cnode = head;
		while (cnode != NULL) {
			free(cnode->buf);
			tmp_node = cnode;
			cnode = cnode->next;
			free(tmp_node);
		}
	}
	void Decoder::decodeRgb24(PNODE cnode) {
		while (cnode != NULL) {
			dec_pkt->data = cnode->buf;
			dec_pkt->size = cnode->size;
			// dec_pkt->pts = cnode->pts;
			// dec_pkt->dts = cnode->dts;
			decode(dec_pkt);
			cnode = cnode->next;
		}
	}
	void Decoder::flush() {
		decode(NULL);
	}
	void Decoder::decode(AVPacket* pkt) {
		int ret = avcodec_send_packet(dec_context, pkt);
		if (ret < 0) {
			printf("avcodec_send_packet fail\n");
			exit(0);
		}
		while (ret >= 0) {
			ret = avcodec_receive_frame(dec_context, dec_frame);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
				return;
			}
			else if (ret < 0) {
				printf("decoding error\n");
				exit(0);
			}
			sws_ctx = sws_getCachedContext(sws_ctx,
				dec_context->width, dec_context->height, AV_PIX_FMT_YUV420P,
				dec_context->width, dec_context->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

			AVFrame* rgbFrame = NULL;
			rgbFrame = av_frame_alloc();
			if (!rgbFrame) {
				return;
			}
			int numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, dec_frame->width, dec_frame->height);
			uint8_t* m_rgbBuffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
			avpicture_fill((AVPicture*)rgbFrame, m_rgbBuffer, AV_PIX_FMT_RGB24, dec_frame->width, dec_frame->height);

			PNODE pnode = (PNODE)malloc(sizeof(NODE));
			if (pnode == NULL) {
				printf("decoder malloc node error\n");
				return;
			}
			memset(pnode, 0, sizeof(NODE));
			if (head == NULL) {
				head = pnode;
				cnode = pnode;
			}
			else {
				cnode->next = pnode;
				cnode = pnode;
			}
			pnode->buf = (uint8_t*)malloc(numBytes);
			if (pnode->buf == NULL) {
				exit(0);
			}
			
			//uint8_t* outdata[AV_NUM_DATA_POINTERS] = { 0 };
			//int outlinesize[AV_NUM_DATA_POINTERS] = { 0 };
			sws_scale(sws_ctx, dec_frame->data, dec_frame->linesize, 0, dec_context->height, rgbFrame->data, rgbFrame->linesize);
			memcpy(pnode->buf, rgbFrame->data[0], numBytes);
			pnode->size = numBytes;
			av_frame_free(&rgbFrame);
			av_free(m_rgbBuffer);

			//printf("decode frame %3d\n", dec_frame->pkt_size);
		}
	}

	ScreenCap::ScreenCap() {
		BOOL ret = FALSE;
		hdcScreen = GetDC(NULL); // 使用主屏幕

		CURSORINFO cursorInfo = { 0 };
		cursorInfo.cbSize = sizeof(cursorInfo);
		ret = GetCursorInfo(&cursorInfo);
		if (ret) {
			if (cursorInfo.flags == CURSOR_SHOWING) {
				POINT pos = cursorInfo.ptScreenPos;
				DrawIconEx(hdcScreen, pos.x, pos.y, cursorInfo.hCursor, 0, 0, 0, 0, DI_NORMAL | DI_DEFAULTSIZE);
			}
		}
		hdc = CreateCompatibleDC(hdcScreen);

		sw = GetDeviceCaps(hdcScreen, HORZRES);
		sh = GetDeviceCaps(hdcScreen, VERTRES);

		bitmap = CreateCompatibleBitmap(hdcScreen, sw, sh);
		gdiObj = SelectObject(hdc, bitmap);
		tmpScreenData = new BYTE[sizeof(BYTE) * sw * sh * 3];
		ret = BitBlt(hdc, 0, 0, sw, sh, hdcScreen, 0, 0, SRCCOPY);

		if (ret) {
			bmi = { 0 };
			bmi.biSize = sizeof(BITMAPINFOHEADER);
			bmi.biPlanes = 1;
			bmi.biBitCount = 24;
			bmi.biWidth = sw;
			bmi.biHeight = sh;
			bmi.biCompression = BI_RGB;
			bmi.biSizeImage = 0;
			bmi.biXPelsPerMeter = 0;
			bmi.biYPelsPerMeter = 0;
			bmi.biClrUsed = 0;
			bmi.biClrImportant = 0;
		}
	}
	ScreenCap::~ScreenCap() {
		delete tmpScreenData;
		SelectObject(hdc, gdiObj);
		DeleteDC(hdc);
		ReleaseDC(NULL, hdcScreen);
		DeleteObject(bitmap);
	}
	void ScreenCap::desktopshot(BYTE* screenData) {
		GetDIBits(hdc, bitmap, 0, sh, tmpScreenData, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS);
		for (int i = 0; i < sh; i++) {
			memcpy(&(screenData[(i * sw) * 3]), &(tmpScreenData[((sh - 1 - i) * sw) * 3]), (size_t)(sw * (size_t)3));
		}
	}
}