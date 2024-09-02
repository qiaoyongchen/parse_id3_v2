#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "id3.h"
#include <string.h>
#include <unistd.h>
#include <string.h>

char *frame_id_keys[] = {
	"AENC", "APIC", "COMM", "COMR", "ENCR", "ETC0", "GEOB", "GRID", "IPLS", "MCDI",
	"MLLT", "OWNE", "PRIV", "PCNT", "POPM", "POSS", "RBUF", "RVAD", "RVRB", "SYLT",
	"SYTC", "TALB", "TBPM", "TCOM", "TCON", "TCOP", "TDAT", "TDLY", "TENC", "TEXT",
	"TFLT", "TIME", "TIT1", "TIT2", "TIT3", "TKEY", "TLAN", "TLEN", "TMED", "TOAL",
	"TOFN", "TOLY", "TOPE", "TORY", "TOWM", "TPE1", "TPE2", "TPE3", "TPE4", "TPOS",
	"TPUB", "TRCK", "TRDA", "TRSN", "TRSO", "TSIZ", "TSRC", "TSSE", "TYER", "TXXX",
	"UFID", "USER", "USLT", "WCOM", "WCOP", "WOAF", "WOAR", "WOAS", "WORS", "WPAY",
	"WPUB", "WXXX"
};

char *frame_id_values[] = {
	"音频加密技术", "附加描述", "注释", "广告", "加密方法注册", "事件时间编码", "常规压缩对象", "组识别注册", "复杂类别列表", "音乐CD标识符",
	"MPEG位置查找表格", "所有权", "私有", "播放计数", "普通仪表", "位置同步", "推荐缓冲区大小", "音量调节器", "混响", "同步歌词或文本",
	"同步节拍编码", "专辑", "每分钟节拍数", "作曲家", "流派", "版权", "日期", "播放列表返录", "编码", "歌词作者",
	"文件类型", "时间", "内容组描述", "标题", "副标题", "最初关键字", "语言", "长度", "媒体类型", "原唱片集",
	"原文件名", "原歌词作者", "原艺术家", "最初发行年份", "文件所有者", "艺术家", "乐队", "指挥者", "翻译", "作品集部分",
	"发行人", "音轨(曲号)", "录制日期", "Intenet电台名称", "Intenet电台所有者", "大小", "ISRC(国际的标准记录代码)", "编码使用的软件(硬件设置)", "年代", "年度",
	"唯一的文件标识符", "使用条款", "歌词", "广告信息", "版权信息", "官方音频文件网页", "官方艺术家网页", "官方音频原始资料网页", "官方互联网无线配置首页", "付款",
	"出版商官方网页", "用户定义的URL链接",
};

id3v2 *id3v2_create(void) {
	id3v2 *id3 = malloc(sizeof(id3v2));
	if (id3 == NULL) {
		return NULL;
	}

	id3->frames_count = 0;
	return id3;
}

void id3v2_destory(id3v2 * const id3) {
	free(id3->head);

	int i;
	for (i = 0; i < id3->frames_count; i++) {
		free(id3->frames[i].content);
	}

	free(id3);
	return;
}

static unsigned long get_frame_content_size(const char *, int);

id3v2frame *get_frame(id3v2 *id3, const enum FRAME_ID id) {
	int i;
	for (i = 0; i < id3->frames_count; i++) {
		if (strcmp(FRAME_ID_KEY(id), id3->frames[i].id) == 0) {
			return id3->frames + i;
		}
	}
	return NULL;
}

static bool parse_id3v2_head(id3v2header *head, const int fd) {
	if (head == NULL) {
		printf("head is null\n");
		return false;
	}

	char head_bts[HEAD_LENGTH];
	char *head_bts_ptr = head_bts;
	if (read(fd, head_bts, HEAD_LENGTH) < HEAD_LENGTH) {
		printf("read head failed\n");
		return false;
	}

	memcpy(head->tag, head_bts, 3); // [0,2]
	head_bts_ptr += 3;

	head->ver = *head_bts_ptr;  head_bts_ptr++;
	head->ver1 = *head_bts_ptr; head_bts_ptr++;
	head->flag = *head_bts_ptr; head_bts_ptr++;
	memcpy(head->tag, head_bts_ptr, 4);

	return true;
}

static unsigned long get_frame_content_size(const char *bytes, int length) {
	return ((unsigned long)bytes[0])*0x1000000ul +
		   ((unsigned long)bytes[1])*0x10000ul   +
		   ((unsigned long)bytes[2])*0x100ul     +
		   ((unsigned long)bytes[3]);
}

static void parse_id3v2_frames(
	id3v2frame *frames,
	const int frames_num,
	const int fd,
	unsigned int *num) {

	if (frames == NULL) {
		printf("frame is null\n");
		return;
	}

	int i = 0;
	int len = 10;
	char f_head[len]; // id(4) + size(4) + flags[2]
	unsigned long content_size = 0;

	while (true) {
		if (read(fd, f_head, len) < len) {
			printf("read frame failed 1\n");
			break;
		}

		char *head_ptr = f_head;
		memcpy(frames[i].id, head_ptr, 4);   head_ptr += 4; // id(4)
		memcpy(frames[i].size, head_ptr, 4); head_ptr += 4; // size(4)
		memcpy(frames[i].flags, head_ptr, 2);               // flags(2)

		bool id_exist = false;
		for (int id = AENC; id <= WXXX; id++) {
			if (memcmp(FRAME_ID_KEY(id), frames[i].id, 4) == 0) {
				id_exist = true;
				printf("find id: %s \n", FRAME_ID_KEY(id));
				break;
			}
		}

		if (!id_exist) {
			break;
		}

		content_size = get_frame_content_size(frames[i].size, 4);
		frames[i].content = (char *)malloc(content_size);
		if (frames[i].content == NULL) {
			printf("read frame[%d] content failed\n", i);
			break;
		}

		if (read(fd, frames[i].content, content_size) < len) {
			break;
		}

		/*
		char str[content_size+1];
		str[content_size] = '\n';
		memcpy(str, frames[i].content, content_size);
		printf("----------------- %s \n", str+1);
		*/

		i++;
	}

	*num = i;
	return;
}

bool parse_id3v2(id3v2 *id3, const int fd) {
	id3->head = (id3v2header *)malloc(sizeof(id3v2header));
	if (!parse_id3v2_head(id3->head, fd)) {
		printf("parse head failed\n");
		return false;
	}

	unsigned int count = 0;
	parse_id3v2_frames(id3->frames, sizeof(id3->frames) / sizeof(id3v2frame), fd, &count);

	if (count == 0) {
		printf("parse frames failed\n");
		return false;
	}
	id3->frames_count = count;
	printf("parse frames ok\n");
	return true;
}

void cover_destory(cover *co) {
	free(co->mime);
	free(co->content);

	free(co);
}

cover *get_cover(id3v2frame *frame) {
	if (frame == NULL) {
		printf("frame is null\n");
		return NULL;
	}

	if (memcmp(FRAME_ID_KEY(APIC), frame->id, 4) != 0) {
		printf("frame id is not found\n");
		return NULL;
	}

	char *all_bytes = frame->content;
	unsigned long all_bytes_len = get_frame_content_size(frame->size, 4);

	if (all_bytes_len <= 0) {
		printf("frame content size is zero");
		return NULL;
	}

	// 0x00 -> utf8, 0x01 -> utf16
	char first_byte_utf8or16 = all_bytes[0];
	char mime_bytes[10];
	int mime_bytes_idx = 0;
	int i;

	for (i = 0; i < all_bytes_len; i++) {
		if (i == sizeof(mime_bytes)) {
			printf("parse cover failed 1\n");
			return NULL;
		}

		if (first_byte_utf8or16 == 0x00 || first_byte_utf8or16 == 0x01) {
			continue;
		}

		mime_bytes[mime_bytes_idx++] = all_bytes[i];

		// end
		if (0x06 == all_bytes[i] && 0x00 == all_bytes[i+1]) {
			break;
		}
	}
	mime_bytes[mime_bytes_idx++] = '\0';

	cover *co = (cover *)malloc(sizeof(cover));
	co->mime = (char *)malloc(strlen(mime_bytes));
	memcpy(co->mime, mime_bytes, 4);

	//
	char tmp_contents[all_bytes_len];	// 申请一个 足够大的缓冲
	unsigned long tmp_content_idx = 0;	// 足够大的缓冲 的位置
	bool in_binary_flag = false;		// 是否正在解析二进制
	char img_type[10];					// 图片类型

	for (i = 0; i < all_bytes_len; i++) {
		if (!in_binary_flag) {
			if (0xff == all_bytes[i+0] && 0xd8 == all_bytes[i+1]) {
				tmp_contents[tmp_content_idx++] = all_bytes[i];
				in_binary_flag = true;
				strcpy(img_type, "JPEG");
				continue;
			}

			if (0x89 == all_bytes[i+0] &&
				0x50 == all_bytes[i+1] &&
				0x4e == all_bytes[i+2] &&
				0x47 == all_bytes[i+3]) {

				tmp_contents[tmp_content_idx++] = all_bytes[i];
				in_binary_flag = true;
				strcpy(img_type, "PNG");
				continue;
			}
		} else {
			tmp_contents[tmp_content_idx++] = all_bytes[i];
			if (strcmp(img_type, "JPEG") == 0 &&
				0xff == all_bytes[i+0] &&
				0xd9 == all_bytes[i+1] ) {

				tmp_contents[tmp_content_idx++] = all_bytes[i+1];
				break;
			}

			if (strcmp(img_type, "PNG") == 0 &&
				0x4e == all_bytes[i+0] &&
				0x44 == all_bytes[i+1] &&
				0xae == all_bytes[i+2] &&
				0x42 == all_bytes[i+3] &&
				0x60 == all_bytes[i+4] &&
				0x82 == all_bytes[i+5]) {

				tmp_contents[tmp_content_idx++] = all_bytes[i+1];
				tmp_contents[tmp_content_idx++] = all_bytes[i+2];
				tmp_contents[tmp_content_idx++] = all_bytes[i+3];
				tmp_contents[tmp_content_idx++] = all_bytes[i+4];
				tmp_contents[tmp_content_idx++] = all_bytes[i+5];
				break;
			}
		}
	}

	co->content = (char *)malloc(tmp_content_idx);
	memcpy(co->content, tmp_contents, tmp_content_idx);
	return co;
}

