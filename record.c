//录音程序，演示alsa library的使用
#include <stdio.h>          //fprintf
#include <alsa/asoundlib.h> //alsa
#include <signal.h> //signal

//是否停止录音
int interrupted = 0;

//处理ctrl+c信号
void handle_sigint(int signo)
{
    interrupted = 1;
}

int main()
{
    signal(SIGINT, handle_sigint);
    //声音设备句柄
    snd_pcm_t *mic = NULL;
    //打开音频设备
    int error = snd_pcm_open(&mic, "sysdefault", SND_PCM_STREAM_CAPTURE, 0);
    if (error != 0)
    {
        fprintf(stderr, "snd_pcm_open() failed: %s\n", snd_strerror(error));
        return EXIT_FAILURE;
    }

    //设置硬件参数
    snd_pcm_hw_params_t *hw_params;
    //分配硬件参数内存
    snd_pcm_hw_params_alloca(&hw_params);
    //初始化硬件参数，使用默认值
    snd_pcm_hw_params_any(mic, hw_params);

    //多声道交错存储，读写方式
    snd_pcm_hw_params_set_access(mic, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);

    //设置采样格式：有符号16位整数，小端字节序
    snd_pcm_hw_params_set_format(mic, hw_params, SND_PCM_FORMAT_S16_LE);

    //设置采样率：16000Hz
    snd_pcm_hw_params_set_rate(mic, hw_params, 16000, 0);

    //设置声道数：单声道
    snd_pcm_hw_params_set_channels(mic, hw_params, 1);

    //根据硬件参数配置声卡设备
    error = snd_pcm_hw_params(mic, hw_params);
    if (error != 0)
    {
        fprintf(stderr, "snd_pcm_hw_params() failed: %s\n", snd_strerror(error));
        return EXIT_FAILURE;
    }

    FILE *fp = fopen("test.pcm", "w");
    if (fp == NULL)
    {
        perror("fopen() failed");
        return EXIT_FAILURE;
    }

    //获取声卡一次传输数据的大小
    unsigned long period = 0;
    int dir = 0;
    snd_pcm_hw_params_get_period_size(hw_params, &period, &dir);

    printf("period: %d\n", period);

    //buf大小是period整数倍
    char buf[682];

    //开始录音
    snd_pcm_prepare(mic);

    while (interrupted != 1)
    {   //从声卡读数据，并写入音频文件
        snd_pcm_sframes_t frames = snd_pcm_readi(mic, buf, period);
        fwrite(buf, 1, snd_pcm_frames_to_bytes(mic, frames), fp);
    }

    //停止录音
    snd_pcm_drain(mic);
    snd_pcm_close(mic);
    fclose(fp);

    return 0;
}
