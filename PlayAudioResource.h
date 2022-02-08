#ifndef __PLAYAUDIORESOURCE_H__
#define __PLAYAUDIORESOURCE_H__

#include <xaudio2.h>
#include <wchar.h>

bool InitXAudio2(void);
IXAudio2SourceVoice* loadSound(const char* filename);
int playSound(IXAudio2SourceVoice* p);
int stopSound(IXAudio2SourceVoice* p);
int destroySound(IXAudio2SourceVoice* p);
void FinalizeXAudio2(void);
IXAudio2SourceVoice* LoadSound(wchar_t* filename, HRESULT ret);

// チャンクデータの基本構造 
struct Chunk
{
	char	id[4]; // チャンク毎のID
	//int32_t	size;  // チャンクサイズ
	int	size;  // チャンクサイズ
};

// RIFFヘッダー
struct RiffHeader
{
	Chunk	chunk;   // "RIFF"
	char	type[4]; // "WAVE"
};

// FMTチャンク
struct FormatChunk
{
	Chunk		chunk; // "fmt "
	WAVEFORMAT	fmt;   // 波形フォーマット
};

#endif