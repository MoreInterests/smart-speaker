#include <stdio.h>  //fgets
#include <stdlib.h> //free
#include <string.h> //strlen
#include <curl/curl.h> //libcurl
#include "cJSON.h"

/*
 构造JSON请求报文
 {
    "perception": {
        "inputText": {
            "text": "你好"
        }
    },
    "userInfo": {
        "apiKey": "xxx",
        "userId": "liuyu"
    }
 }
 */
char* robot_make_request(const char* apikey, const char* text)
{
    //判断输入的字符串长度不为0
    if (strlen(text) == 0)
    {
        return NULL;
    }

    cJSON* request = cJSON_CreateObject();

    cJSON* perception = cJSON_CreateObject();
    cJSON* inputText = cJSON_CreateObject();

    cJSON_AddStringToObject(inputText, "text", text);
    cJSON_AddItemToObject(perception, "inputText", inputText);
    cJSON_AddItemToObject(request, "perception", perception);

    cJSON* userInfo = cJSON_CreateObject();
    cJSON_AddStringToObject(userInfo, "apiKey", apikey);
    cJSON_AddStringToObject(userInfo, "userId", "liuyu");
    cJSON_AddItemToObject(request, "userInfo", userInfo);

    //将JSON数据结构转为字符串
    return cJSON_Print(request);
}

//作业：完成此函数，发送请求报文给图灵机器人服务器，等待服务器的响应报文
//     收到响应报文后，不需要解析，直接通过函数返回值返回JSON字符串
char* robot_send_request(const char* request)
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

    //准备HTTP请求消息，设置API地址（URI）
    curl_easy_setopt(curl, CURLOPT_URL, "http://openapi.tuling123.com/openapi/api/v2");
    //配置客户端，使用HTTP的POST方法发送请求消息
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    //配置需要通过POST请求消息发送给服务器的数据
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request);
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
        return NULL;
    }

    //释放HTTP客户端申请的资源
    curl_easy_cleanup(curl);

    //关闭内存文件
    fclose(fp);

    return response;
}

/*
处理机器人响应报文
{
    "intent": {
        "actionName": "",
        "code": 10004,
        "intentName": ""
    },
    "results": [
        {
            "groupType": 1,
            "resultType": "text",
            "values": {
                "text": "真有礼貌，人家喜欢你呢"
            }
        }
    ]
}
 */
void robot_process_response(const char* response)
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

    cJSON* intent = cJSON_GetObjectItemCaseSensitive(json, "intent");
    if (intent == NULL)
    {
        fprintf(stderr, "Get intent object failed\n");
        cJSON_Delete(json);
        return;
    }
    cJSON* code = cJSON_GetObjectItemCaseSensitive(intent, "code");
    if (code->valueint < 10000) //code字段的值如果小于10000，认为机器人出错
    {
        fprintf(stderr, "机器人出现%d号错误\n", code->valueint);
    }

    cJSON* results = cJSON_GetObjectItemCaseSensitive(json, "results");
    cJSON* result;
    cJSON_ArrayForEach(result, results) //遍历数组
    {
        cJSON* resultType = cJSON_GetObjectItemCaseSensitive(result, "resultType");
        if (strcmp(resultType->valuestring, "text") == 0) //只打印文本消息
        {
            cJSON* values = cJSON_GetObjectItemCaseSensitive(result, "values");
            cJSON* text = cJSON_GetObjectItemCaseSensitive(values, "text");
            puts(text->valuestring);
        }
    }

    cJSON_Delete(json);
}


//图灵机器人API，一次最多可以处理128个字符
#define LINE_LEN 128

//保存输入字符串的缓冲区
char line[LINE_LEN];

int main()
{
    //将标准输出设置为行缓冲，将输出的内容作为下一个程序的输入
    setlinebuf(stdout);
    char* apikey = "a38a80a5401648c1964e09aa962516b5";
    
    //char* apikey = "d6de5a57291d476ab770c11551b937b0";
    //从标准输入读取一行字符
    while(fgets(line, LINE_LEN, stdin) != NULL)
    {
        //构造请求报文
        char* request = robot_make_request(apikey, line);
        if (request == NULL)
        {
            continue;
        }
        //将请求报文发送给图灵机器人
        char* response = robot_send_request(request);
        free(request);
        if (response == NULL)
        {
            continue;
        }
        robot_process_response(response);
        free(response);
    }

    return 0;
}
