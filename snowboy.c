#include "snowboy-detect-c-wrapper.h" //snowboy
#include <stdio.h>          //fprintf
#include <alsa/asoundlib.h> //alsa
#include <signal.h> //signal
#include "stt.h" //speech2text

//是否停止录音
int interrupted = 0;

//处理ctrl+c信号
void handle_sigint(int signo)
{
    interrupted = 1;
}

//打开录音设备
snd_pcm_t* open_recorder(const char* name)
{
    //声音设备句柄
    snd_pcm_t *mic = NULL;
    //打开音频设备
    int error = snd_pcm_open(&mic, "sysdefault", SND_PCM_STREAM_CAPTURE, 0);
    if (error != 0)
    {
        fprintf(stderr, "snd_pcm_open() failed: %s\n", snd_strerror(error));
        return NULL;
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
        return NULL;
    }

    return mic;
}

//关闭声音设备
void close_recorder(snd_pcm_t* mic)
{
    snd_pcm_drain(mic);
    snd_pcm_close(mic);
}

//开始录音
int start_recording(snd_pcm_t* mic)
{
    return snd_pcm_prepare(mic);
}

//停止录音
int stop_recording(snd_pcm_t* mic)
{
    return snd_pcm_drop(mic);
}

//作业：由于百度语音识别有60秒的限制，因此需要限制录音时长在30秒以内，修改代码。
int main()
{
    //将标准输出设置为行缓冲，将输出的内容作为下一个程序的输入
    setlinebuf(stdout);

    SnowboyDetect* detector = SnowboyDetectConstructor("common.res", "tiedan.pmdl");
    SnowboyDetectSetSensitivity(detector, "0.5");
    SnowboyDetectSetAudioGain(detector, 1);

    //设置信号处理函数
    signal(SIGINT, handle_sigint);

    snd_pcm_t* mic = open_recorder("sysdefault");
    if (mic == NULL)
    {
        return EXIT_FAILURE;
    }

    #define NUM_FRAMES 1600
    char* buf = malloc(snd_pcm_frames_to_bytes(mic, NUM_FRAMES));

    char* record = NULL;
    size_t reclen = 0;
    FILE* fp = NULL;

    start_recording(mic);
    //标识是否正在录音
    int recording = 0;
    //记录语音停顿时间
    int count = 0;
    while (interrupted != 1)
    {
        //每次读取0.1秒的音频数据，1600个采样点，一个采样点2字节，1600 x 2 = 3200字节
        snd_pcm_sframes_t frames = snd_pcm_readi(mic, buf, NUM_FRAMES);
        if (frames < 0)
        {
            fprintf(stderr, "snd_pcm_readi() failed: %s\n", snd_strerror(frames));
            continue;
        }
        if (recording == 1)
        {
            //将音频数据写入内存文件
            fwrite(buf, 1, snd_pcm_frames_to_bytes(mic, frames), fp);
        }
        //返回值：
        // >0 检测到关键词
        // =0 有声音但不是关键词
        // =-1 出错
        // =-2 没有声音
        int result = SnowboyDetectRunDetection(detector, (int16_t*)buf, frames, 0);
        if ((result > 0) && (recording == 0)) //检测到关键词，并且没有开始录音
        {
            //创建内存文件，将后续的音频数据保存到此文件中
            fp = open_memstream(&record, &reclen);
            recording = 1; //开始录音
            //puts("start recording");
            stop_recording(mic);
            system("aplay -q ding.wav"); //提示录音开始
            start_recording(mic); //重新开始录音
        }

        if ((result == -2) && (recording == 1)) //命令结束，并且正在录音，停止录音
        {
            count++;//停顿0.1秒
            if (count > 20) //*连续*停顿超过2秒
            {
                recording = 0;
                fclose(fp);
                count = 0;
                //puts("stop recording");
                //由于语音识别时间较长，需要暂停录音
                stop_recording(mic);
                system("aplay -q dong.wav"); //提示录音结束
                //将录制语音数据，发送到百度云进行识别
                speech2text(record, reclen);
                free(record);
                start_recording(mic); //重新开始录音
            }
        }

        if (result == 0) //当有声音时count值复位
        {
            count = 0;
        }
    }

    close_recorder(mic);
    SnowboyDetectDestructor(detector);
    free(buf);
    return 0;
}
