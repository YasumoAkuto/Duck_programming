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

// �`�����N�f�[�^�̊�{�\�� 
struct Chunk
{
	char	id[4]; // �`�����N����ID
	//int32_t	size;  // �`�����N�T�C�Y
	int	size;  // �`�����N�T�C�Y
};

// RIFF�w�b�_�[
struct RiffHeader
{
	Chunk	chunk;   // "RIFF"
	char	type[4]; // "WAVE"
};

// FMT�`�����N
struct FormatChunk
{
	Chunk		chunk; // "fmt "
	WAVEFORMAT	fmt;   // �g�`�t�H�[�}�b�g
};

#endif