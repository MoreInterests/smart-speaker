#include <stdio.h>
#include <stdlib.h> //free
#include <string.h> //strlen
#include <curl/curl.h> //libcurl
#include "auth.h" //get_token

//*注意*：为了保持代码流程清晰方便讲解，本次实训所有代码均没有对内存不足的异常情况进行处理，实际编码时应该处理此类异常。

//将文本转换为语音
void text2speech(const char* token, const char* text)
{
    CURL* curl = curl_easy_init();

    //发送到百度云的字符串需要进行2次URL编码
    char* temp = curl_easy_escape(curl, text, strlen(text));
    char* data = curl_easy_escape(curl, temp, strlen(temp));
    curl_free(temp);

    //拼接POST请求发送的数据
    char* postdata;
    asprintf(&postdata, "tex=%s&lan=zh&cuid=hqyj&ctp=1&aue=6&tok=%s", data, token);
     //asprintf(&postdata, "tex=%s&lan=zh&cuid=hqyj&ctp=1&aue=6&tok=%s&per=106", data, token);
    curl_free(data);
    
    //启动播放软件，通过管道写入音频数据
    FILE* fp = popen("aplay -q -", "w");
    if (fp == NULL)
    {
        perror("fopen() failed");
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, "https://tsn.baidu.com/text2audio");
    //配置客户端，使用HTTP的POST方法发送请求消息
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    //配置需要通过POST请求消息发送给服务器的数据
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    //发送HTTP请求消息，等待服务器的响应消息
    CURLcode error = curl_easy_perform(curl);
    if (error != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(error));
        curl_easy_cleanup(curl);
        free(postdata);
        pclose(fp);
        return;
    }

    //释放HTTP客户端申请的资源
    curl_easy_cleanup(curl);
    free(postdata);

    //关闭管道
    pclose(fp);
}

//百度语音合成API，一次最多可以转换2048个字符
#define LINE_LEN 2048

//保存输入字符串的缓冲区
char line[LINE_LEN];

int main()
{
    //语音合成API的免费配额10QPS，大家尽量使用自己申请的APIKEY和SECRETKEY，同时使用人数太多会出错
    
    char* token = get_token("mqpytyivkpE7KXTffcu2vFaZ", "GkjQF4kNE0vTtGil9ErHGQNal2KrVV9H");
    if (NULL == token)
    {
        return EXIT_FAILURE;
    }
    //puts(token);
    //从标准输入读取一行字符
    while(fgets(line, LINE_LEN, stdin) != NULL)
    {
        //将读入的文本转换为语音
        text2speech(token, line);
    }

    free(token);

    return 0;
}
