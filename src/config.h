#ifndef CONFIG_H
#define CONFIG_H


#define TRUE (1 == 1)
#define FALSE (!(TRUE))

#define MAX_TARGET_URL_LEN      512
#define MAX_CONTENT_TYPE_LEN    256

struct config
{
    int scanner_vendor_id;
    int scanner_product_id;
    int nonpriv_gid;
    int nonpriv_uid;
    int scan_timeout;
    char http_upload_verb[5];
    char http_target_url[MAX_TARGET_URL_LEN + 1];
    char http_content_type[MAX_CONTENT_TYPE_LEN + 1];
};


#endif // CONFIG_H
