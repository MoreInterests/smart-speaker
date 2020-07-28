#include <stdio.h>
#include <stdlib.h> //malloc使用的头文件
#include <curl/curl.h> //libcurl的头文件
#include "cJSON.h" //cJSON的头文件

int main(void)
{
    FILE* fp;

    //以只写方式打开文件
    //fp = fopen("hello.txt", "w");

    //响应消息的地址
    char* response = NULL;
    //响应消息的长度
    size_t resplen = 0;
    //创建内存文件，当通过文件句柄写入数据时，会自动分配内存
    fp = open_memstream(&response, &resplen);
    if (fp == NULL) //打开文件失败，打印错误信息并退出
    {
        perror("open_memstream() failed");
        return EXIT_FAILURE;
    }

    //初始化HTTP客户端
    CURL* curl = curl_easy_init();
    if (curl == NULL)
    {
        perror("curl_easy_init() failed");
        return EXIT_FAILURE;
    }

    //准备HTTP请求消息，设置API地址（URI）
    curl_easy_setopt(curl, CURLOPT_URL, "https://coronavirus-tracker-api.herokuapp.com/v2/latest");
    //如果不指定写入的文件，libcurl会把服务器响应消息中的内容打印到屏幕上
    //如果指定了文件句柄，libcurl会把服务器响应消息中的内容写入文件
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    //打印HTTP请求和响应消息头
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

    //发送HTTP请求消息，等待服务器的响应消息
    CURLcode error = curl_easy_perform(curl);
    if (error != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(error));
        curl_easy_cleanup(curl);
        return EXIT_FAILURE;
    }

    //释放HTTP客户端申请的资源
    curl_easy_cleanup(curl);

    //关闭内存文件
    fclose(fp);

    puts(response);

    //解析JSON字符串
    cJSON* json = cJSON_Parse(response);
    if (json == NULL)
    {
        const char* error_pos = cJSON_GetErrorPtr();
        if (error_pos != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_pos);
        }
        return EXIT_FAILURE;
    }

    cJSON* latest = cJSON_GetObjectItemCaseSensitive(json, "latest");

    cJSON* comfirmed = cJSON_GetObjectItemCaseSensitive(latest, "confirmed");

    cJSON* deaths = cJSON_GetObjectItemCaseSensitive(latest, "deaths");

    printf("确诊人数: %d\n", comfirmed->valueint);
    printf("死亡人数: %d\n", deaths->valueint);

    //释放json数据结构占用的内存
    cJSON_free(json);

    free(response);

    return EXIT_SUCCESS;
}
