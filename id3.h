#ifndef ID_3_H
#define ID_3_H

#include <stdbool.h>

enum FRAME_ID {
	AENC, APIC, COMM, COMR, ENCR, ETC0, GEOB, GRID, IPLS, MCDI,
	MLLT, OWNE, PRIV, PCNT, POPM, POSS, RBUF, RVAD, RVRB, SYLT,
	SYTC, TALB, TBPM, TCOM, TCON, TCOP, TDAT, TDLY, TENC, TEXT,
	TFLT, TIME, TIT1, TIT2, TIT3, TKEY, TLAN, TLEN, TMED, TOAL,
	TOFN, TOLY, TOPE, TORY, TOWM, TPE1, TPE2, TPE3, TPE4, TPOS,
	TPUB, TRCK, TRDA, TRSN, TRSO, TSIZ, TSRC, TSSE, TYER, TXXX,
	UFID, USER, USLT, WCOM, WCOP, WOAF, WOAR, WOAS, WORS, WPAY,
	WPUB, WXXX,
};

extern char *frame_id_keys[];
extern char *frame_id_values[];

#define FRAME_ID_KEY(idx) frame_id_keys[(idx)]
#define FRAME_ID_VALUE(idx) frame_id_values[(idx)]

#define HEAD_LENGTH 10 //

typedef struct id3v2header {
	char tag[3];
	char ver;
	char ver1;
	char flag;
	char size[4];
} id3v2header;

typedef struct id3v2frame {
	char id[4];
	char size[4];
	char flags[2];
	char *content;
} id3v2frame;

typedef struct cover {
	char *mime;					// 字符串 (utf8 / utf16)
	char *content;				// 字节串
	unsigned long content_len;	// 字节串长度
} cover;

#define ALL_FRAME_ID_COUNT 72

typedef struct id3v2 {
	id3v2header *head;
	id3v2frame frames[ALL_FRAME_ID_COUNT];
	unsigned int frames_count;
} id3v2;

id3v2 *id3v2_create(void);

bool parse_id3v2(id3v2 *, const int fd);

id3v2frame *get_frame(id3v2 *, enum FRAME_ID);

cover *get_cover(id3v2frame *);

void id3v2_destory(id3v2 *);

void cover_destory(cover *);

#endif
