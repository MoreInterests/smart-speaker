#include <stdio.h> //open_memstream
#include <stdlib.h> //free
#include <string.h> //strdup
#include <curl/curl.h> //libcurl
#include "cJSON.h"
#include "auth.h"

//使用APIKEY和SECRETKEY获取ACCESS_TOKEN
char* get_token(const char* api_key, const char* secret_key)
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

    char* uri = NULL;
    //拼接URI，asprintf会自动分配内存，并将要打印的字符串保存到内存中。
    asprintf(&uri, 
             "https://openapi.baidu.com/oauth/2.0/token?grant_type=client_credentials&client_id=%s&client_secret=%s",
             api_key,
             secret_key);
    //准备HTTP请求消息，设置API地址（URI）
    curl_easy_setopt(curl, CURLOPT_URL, uri);
    //如果不指定写入的文件，libcurl会把服务器响应消息中的内容打印到屏幕上
    //如果指定了文件句柄，libcurl会把服务器响应消息中的内容写入文件
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    //发送HTTP请求消息，等待服务器的响应消息
    CURLcode error = curl_easy_perform(curl);
    if (error != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(error));
        curl_easy_cleanup(curl);
        free(uri);
        fclose(fp);
        free(response);
        return NULL;
    }

    //释放HTTP客户端申请的资源
    curl_easy_cleanup(curl);
    //释放asprintf分配的内存
    free(uri);
    //关闭内存文件
    fclose(fp);

    //puts(response);
    /*
    正确响应报文
    {
        "refresh_token": "25.2b97c3f44c2a255306098e589f2b70ee.315360000.1909101772.282335-17496766",
        "expires_in": 2592000,
        "session_key": "9mzdXvCDci68hjNx8j\/SO1e97mi9CXFhcJy\/mjgvmctijGB53gWfoz\/TR1mqIK\/M2\/Vd9hr7svLYvqKKE+Xc4KNBucwimg==",
        "access_token": "24.e5129685f90f2fdcfc70965c094602c2.2592000.1596333772.282335-17496766",
        "scope": "brain_speech_realtime brain_asr_async audio_voice_assistant_get brain_enhanced_asr audio_tts_post public brain_all_scope picchain_test_picchain_api_scope wise_adapt lebo_resource_base lightservice_public hetu_basic lightcms_map_poi kaidian_kaidian ApsMisTest_Test\u6743\u9650 vis-classify_flower lpq_\u5f00\u653e cop_helloScope ApsMis_fangdi_permission smartapp_snsapi_base iop_autocar oauth_tp_app smartapp_smart_game_openapi oauth_sessionkey smartapp_swanid_verify smartapp_opensource_openapi smartapp_opensource_recapi fake_face_detect_\u5f00\u653eScope vis-ocr_\u865a\u62df\u4eba\u7269\u52a9\u7406 idl-video_\u865a\u62df\u4eba\u7269\u52a9\u7406",
        "session_secret": "086951772c0e0ae69fd932df7e0045b6"
    }

    错误响应报文
    {
        "error": "invalid_client",
        "error_description": "unknown client id"
    }
     */
    cJSON* json = cJSON_Parse(response);
    if (json == NULL)
    {
        const char* error_pos = cJSON_GetErrorPtr();
        if (error_pos != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_pos);
        }
        free(response);
        return NULL;
    }

    //获取JSON报文中的access_token字段
    cJSON* access_token = cJSON_GetObjectItemCaseSensitive(json, "access_token");
    if (!cJSON_IsString(access_token)) //如果不存在access_token字段，进行错误处理
    {
        cJSON* error_description = cJSON_GetObjectItemCaseSensitive(json, "error_description");
        fprintf(stderr, "%s\n", error_description->valuestring);
        return NULL;
    }

    //复制token字符串，使用完之后需要使用free函数释放此内存
    char* token = strdup(access_token->valuestring);

    free(response);
    //释放cjson数据结构占用的内存
    cJSON_Delete(json);

    return token;
}
