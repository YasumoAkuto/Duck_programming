#include <windows.h>
#include    <stdio.h>
#pragma comment (lib, "xinput.lib")
#include <xinput.h>
#include <cstdlib>      // srand,rand

#pragma comment(lib, "xaudio2.lib")
#include <xaudio2.h>

#include <mmsystem.h>
#include <wchar.h>

#include "PlayAudioResource.h"

IXAudio2* pXAudio;
IXAudio2MasteringVoice* pMasteringVoice;
IXAudio2SourceVoice* pSourceVoice = NULL;
XAUDIO2_BUFFER buf{};
char* g_wavBuffer;


// --------------------------------------------------------------------------------------------------------------------
// 初期化
bool InitXAudio2() {
    if (FAILED(CoInitializeEx(0, COINIT_MULTITHREADED)))
    {
        return false;
    }
    if (FAILED(XAudio2Create(&pXAudio)))
    {
        return false;
    }
    if (FAILED(pXAudio->CreateMasteringVoice(&pMasteringVoice)))
    {
        return false;
    }

    return true;
}

// 音ファイルの読み込み
IXAudio2SourceVoice* loadSound(const char* filename)
{
    FILE* file = NULL;

    if (fopen_s(&file, filename, "rb") != 0) {
        return NULL;        // ファイルが見つからなかった
    }
    // RIFFヘッダーの読み込み
    RiffHeader riff;
    fread(&riff, sizeof(riff), 1, file);

    // Formatチャンクの読み込み
    FormatChunk format;
    fread(&format, sizeof(format), 1, file);

    // Dataチャンクの読み込み
    Chunk data;
    fread(&data, sizeof(data), 1, file);

    // Dataチャンクのデータ部（波形データ）の読み込み
    g_wavBuffer = (char*)malloc(data.size); // 波形データのサイズ分のメモリを確保する
    if (g_wavBuffer != NULL)
        fread(g_wavBuffer, data.size, 1, file);
    fclose(file);   // ファイルのストリームを閉める

    WAVEFORMATEX wfex{};
    // 波形フォーマットの設定
    memcpy(&wfex, &format.fmt, sizeof(format.fmt));
    wfex.wBitsPerSample = format.fmt.nBlockAlign * 8 / format.fmt.nChannels;

    // 波形フォーマットを元にSourceVoiceの生成
    IXAudio2SourceVoice* pSourceVoice = NULL;
    if (FAILED(pXAudio->CreateSourceVoice(&pSourceVoice, &wfex))) {
        free(g_wavBuffer);
        return NULL;
    }

    // 再生する波形データの設定
    buf.pAudioData = (BYTE*)g_wavBuffer;
    buf.Flags = XAUDIO2_END_OF_STREAM;
    buf.AudioBytes = data.size;
    pSourceVoice->SubmitSourceBuffer(&buf);

    return pSourceVoice;
}

int playSound(IXAudio2SourceVoice* p) {
    if (p == NULL)  return -1;      // ポインタの値をチェックする

    XAUDIO2_VOICE_STATE xa2state;
    p->GetState(&xa2state);
    if (xa2state.BuffersQueued > 0) {   // 再生中か？
        p->Stop();                      // 再生中なら、一応止める 
    }
    p->FlushSourceBuffers();        // バッファをクリアして..
    p->SubmitSourceBuffer(&buf);    // 再度、データをセットする
    p->Start();                     // 再生開始
    return 0;
}

int stopSound(IXAudio2SourceVoice* p) {
    if (p == NULL)  return -1;      // ポインタの値をチェックする
    p->Stop();                      // 再生停止
    return 0;
}

int destroySound(IXAudio2SourceVoice* p) {
    if (p == NULL)  return -1;      // ポインタの値をチェックする
    p->DestroyVoice();              // データの破棄
    return 0;
}

void FinalizeXAudio2() {
    if (pMasteringVoice) {
        pMasteringVoice->DestroyVoice();
        pMasteringVoice = NULL;
    }
    if (pXAudio) {
        pXAudio->Release();         // XAudio2の解放
        pXAudio = NULL;
    }
    if (g_wavBuffer != NULL)        // 波形データ領域の解放
        free(g_wavBuffer);
    CoUninitialize();
}


IXAudio2SourceVoice* LoadSound(wchar_t* filename, HRESULT ret) {
    HMMIO mmio = NULL;
    MMIOINFO info = { 0 };
    mmio = mmioOpen(filename, &info, MMIO_READ);
    if (!mmio) {
        printf("error mmioOpen\n");
        return NULL;
    }

    MMRESULT mret;
    MMCKINFO riff_chunk;
    riff_chunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    mret = mmioDescend(mmio, &riff_chunk, NULL, MMIO_FINDRIFF);
    if (mret != MMSYSERR_NOERROR) {
        printf("error mmioDescend(wave) ret=%d\n", mret);
        return NULL;
    }

    MMCKINFO chunk;
    /* fmt chunk */
    chunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    mret = mmioDescend(mmio, &chunk, &riff_chunk, MMIO_FINDCHUNK);
    if (mret != MMSYSERR_NOERROR) {
        printf("error mmioDescend(fmt) ret=%d\n", mret);
        return NULL;
    }

    WAVEFORMATEX format = { 0 };
    {
        DWORD size = mmioRead(mmio, (HPSTR)&format, chunk.cksize);
        if (size != chunk.cksize) {
            printf("error mmioRead(fmt) ret=%d\n", mret);
            return NULL;
        }

        printf("foramt    =%d\n", format.wFormatTag);
        printf("channel   =%d\n", format.nChannels);
        printf("sampling  =%dHz\n", format.nSamplesPerSec);
        printf("bit/sample=%d\n", format.wBitsPerSample);
        printf("byte/sec  =%d\n", format.nAvgBytesPerSec);
    }

    IXAudio2SourceVoice* voice = NULL;
    ret = pXAudio->CreateSourceVoice(
        &voice,
        &format,
        0,                          // UINT32 Flags = 0,
        XAUDIO2_DEFAULT_FREQ_RATIO, // float MaxFrequencyRatio = XAUDIO2_DEFAULT_FREQ_RATIO,
        0                   // IXAudio2VoiceCallback *pCallback = NULL,
        // const XAUDIO2_VOICE_SENDS *pSendList = NULL,
        // const XAUDIO2_EFFECT_CHAIN *pEffectChain = NULL
    );
    if (FAILED(ret)) {
        printf("error CreateSourceVoice ret=%d\n", ret);
        return NULL;
    }

    chunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    mret = mmioDescend(mmio, &chunk, &riff_chunk, MMIO_FINDCHUNK);
    if (mret != MMSYSERR_NOERROR) {
        printf("error mmioDescend(data) ret=%d\n", mret);
        return NULL;
    }

    char* sound_buffer = new char[chunk.cksize];
    int size;
    size = mmioRead(mmio, (HPSTR)sound_buffer, chunk.cksize);
    buf.AudioBytes = size;
    buf.pAudioData = (BYTE*)sound_buffer;
    buf.Flags = XAUDIO2_END_OF_STREAM;
    if (0 < size) {
        ret = voice->SubmitSourceBuffer(&buf);
        if (FAILED(ret)) {
            printf("error SubmitSourceBuffer ret=%d\n", mret);
            return NULL;
        }
    }

    mmioClose(mmio, MMIO_FHOPEN);   //ファイルを閉じる

    return voice;

}