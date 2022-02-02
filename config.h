#ifndef CONFIG_H
#define CONFIG_H


#define TRUE (1 == 1)
#define FALSE (!(TRUE))

struct config
{
    int scanner_vendor_id;
    int scanner_product_id;
    int nonpriv_gid;
    int nonpriv_uid;
    int scan_timeout;
    char http_target_url[512];
};


#endif // CONFIG_H
