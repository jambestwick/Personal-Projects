#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <curl/curl.h>

#define FROM_ADDR "<zzApotheosis@gmail.com>"
#define TO_ADDR "<steven.c.jennings@lmco.com>"
#define FROM_MAIL "<zzApotheosis@gmail.com>"
#define TO_MAIL "<steven.c.jennings@lmco.com>"

#define USER "zzApotheosis@gmail.com"
#define PASS "LIGMA"
#define PASS_FILE "/root/zzApotheosis_gmail_app_password.txt"

char* get_password(char *f) {
    /* Check for NULL file */
    if (f == NULL) {
        return(NULL);
    }
    
    if (access(f, F_OK) == 0) {
        fprintf(stdout, "%s: FILE EXISTS!\n", f);
    } else {
        fprintf(stdout, "%s: FILE NO EXIST!\n", f);
    }
}

static const char *payload_text =
  /* "Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n" */
  "To: " TO_MAIL "\r\n"
  "From: " FROM_MAIL "\r\n"
  /* "Cc: \r\n" */
  /* "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@" */
  /* "rfcpedant.example.org>\r\n" */
  "Subject: One really cool message\r\n"
  "\r\n" /* empty line to divide headers from body, see RFC5322 */
  "The body of the message starts here.\r\n"
  "\r\n"
  "It could be a lot of lines, could be MIME encoded, whatever.\r\n"
  "Check RFC5322.\r\n";

struct upload_status {
    size_t bytes_read;
};

static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp) {
    struct upload_status *upload_ctx = (struct upload_status *) userp;
    const char *data;
    size_t room = size * nmemb;

    if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1)) {
        return(0);
    }

    data = &payload_text[upload_ctx->bytes_read];

    if (data) {
        size_t len = strlen(data);
        if (room < len) {
            len = room;
        }
        memcpy(ptr, data, len);
        upload_ctx->bytes_read += len;
        return(len);
    }

    return(0);
}

int main(int argc, char **argv) {
    int exit_code = 0;
    
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx = { 0 };
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, USER);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, PASS);
        
        /* This is the URL for your mailserver */
        curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");
        
        /* Note that this option is not strictly required, omitting it will result
         * in libcurl sending the MAIL FROM command with empty sender data. All
         * autoresponses should have an empty reverse-path, and should be directed
         * to the address in the reverse-path which triggered them. Otherwise,
         * they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
         * details.
         */
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM_ADDR);
        
        /* Add two recipients, in this particular case they correspond to the
         * To: and Cc: addressees in the header, but they could be any kind of
         * recipient. */
        recipients = curl_slist_append(recipients, TO_ADDR);
        /* recipients = curl_slist_append(recipients, CC_ADDR); */
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        
        /* We are using a callback function to specify the payload (the headers and
         * body of the message). You could just use the CURLOPT_READDATA option to
         * specify a FILE pointer to read from. */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        /* Send the message */
        res = curl_easy_perform(curl);
        
        /* Check for errors */
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                  curl_easy_strerror(res));
        }
        
        /* Free the list of recipients */
        curl_slist_free_all(recipients);
        
        /* curl will not send the QUIT command until you call cleanup, so you
         * should be able to re-use this connection for additional messages
         * (setting CURLOPT_MAIL_FROM and CURLOPT_MAIL_RCPT as required, and
         * calling curl_easy_perform() again. It may not be a good idea to keep
         * the connection open for a very long time though (more than a few
         * minutes may result in the server timing out the connection), and you do
         * want to clean up in the end.
         */
        curl_easy_cleanup(curl);
    }
    
    return(exit_code);
}
