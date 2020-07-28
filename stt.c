#include <stdio.h>
#include <stdlib.h> //malloc/free
#include <curl/curl.h> //libcurl
#include "auth.h" //get_token
#include "cJSON.h"
#include "stt.h"

//读取音频数据到内存，返回数据大小
size_t stt_load_file(const char* file, char** pbuf)
{
    FILE* fp = fopen(file, "r");
    if (fp == NULL)
    {
        perror("fopen failed");
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    *pbuf = malloc(size);
    fseek(fp, 0, SEEK_SET);

    fread(*pbuf, 1, size, fp);
    fclose(fp);

    return size;
}

char* stt_send_request(const char* token, const char* audio, size_t size)
{
    FILE* fp;
    //响应消息的地址
    char* response = NULL;
    //响应消息的长度
    size_t resplen = 0;
    //创建内存文件，当通过文件句柄写入数据时，会自动分配内存
    fp = open_memstream(&response, &resplen);
    if (fp == NULL) //打开文件失败，打印错误信息并退出
    {
        perror("open_memstream() failed");
        return NULL;
    }

    //初始化HTTP客户端
    CURL* curl = curl_easy_init();
    if (curl == NULL)
    {
        perror("curl_easy_init() failed");
        return NULL;
    }

    //拼接API地址和参数
    char* url = NULL;
    asprintf(&url, "http://vop.baidu.com/server_api?cuid=liuyu&token=%s&dev_pid=1537", token);

    //准备HTTP请求消息，设置API地址（URI）
    curl_easy_setopt(curl, CURLOPT_URL, url);
    //创建请求头链表
    struct curl_slist* headers = NULL;
    //向链表中增加头部字段
    headers = curl_slist_append(headers, "Content-Type: audio/pcm;rate=16000");
    //设置HTTP请求头部字段
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    //配置客户端，使用HTTP的POST方法发送请求消息
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    //配置需要通过POST请求消息发送给服务器的数据
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, audio);
    //指定发送数据的长度
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, size);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    //打印HTTP请求和响应消息头
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

    //发送HTTP请求消息，等待服务器的响应消息
    CURLcode error = curl_easy_perform(curl);
    if (error != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(error));
        curl_easy_cleanup(curl);
        fclose(fp);
        free(response);
        free(url);
        return NULL;
    }

    //释放HTTP客户端申请的资源
    curl_easy_cleanup(curl);

    //关闭内存文件
    fclose(fp);

    //释放asprintf申请的内存
    free(url);

    return response;
}

/*
解析服务器响应报文
{
    "corpus_no": "6846258921441177293",
    "err_msg": "success.",
    "err_no": 0,
    "result": [
        "西安工程大学。"
    ],
    "sn": "396129478411594018870"
}
 */
void stt_process_response(const char* response)
{
    cJSON* json = cJSON_Parse(response);
    if (json == NULL)
    {
        const char* error_pos = cJSON_GetErrorPtr();
        if (error_pos != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_pos);
        }
        return;
    }

    cJSON* err_no = cJSON_GetObjectItemCaseSensitive(json, "err_no");
    if (err_no->valueint != 0)
    {
        cJSON* err_msg = cJSON_GetObjectItemCaseSensitive(json, "err_msg");
        fprintf(stderr, "%s\n", err_msg->valuestring);
        return;
    }

    cJSON* result = cJSON_GetObjectItemCaseSensitive(json, "result");
    //获取数组中的第一个元素
    cJSON* text = cJSON_GetArrayItem(result, 0);
    puts(text->valuestring);
}

//将语音转换为文字打印到屏幕上
void speech2text(const char* buf, size_t size)
{
    
    char* token = get_token("mqpytyivkpE7KXTffcu2vFaZ", "GkjQF4kNE0vTtGil9ErHGQNal2KrVV9H");
    if (NULL == token)
    {
        return;
    }
    //将语音数据发送给云服务器，等待响应报文
    char* response = stt_send_request(token, buf, size);
    free(token);
    if (response == NULL)
    {
        return;
    }
    //处理响应报文，将文本信息打印到屏幕上
    stt_process_response(response);
    free(response);
}

//作业：录制一句话音频，通过语音识别模块生成文本，再通过对话管理模块生成应答文本，最后通过语音合成模块将应答文本读出来。
//录音：arecord -t raw -r 16000 -f S16_LE -c 1 test.pcm
//播放：aplay -t raw -r 16000 -f S16_LE -c 1 test.pcm
#if 0
int main()
{
    //指向音频数据的指针
    char* audio = NULL;
    //音频数据大小
    size_t size;
    //读取PCM文件内容
    size = stt_load_file("test.pcm", &audio);
    if (size == 0)
    {
        return EXIT_FAILURE;
    }

    char* token = get_token("gG1oXkYhqC72KbsfKNawHZNv", "ijAXKgNwUaQZbbVXryZmef6DnouQ6EnS");
    if (NULL == token)
    {
        free(audio);
        return EXIT_FAILURE;
    }
    //将语音数据发送给云服务器，等待响应报文
    char* response = stt_send_request(token, audio, size);
    free(token);
    free(audio);
    if (response == NULL)
    {
        return EXIT_FAILURE;
    }
    //puts(response);
    //处理响应报文，将文本信息打印到屏幕上
    stt_process_response(response);
    free(response);
    return 0;
}
#endif