#ifndef protect
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>


void hexToText(char *src,unsigned char * dest);
void textToHex(unsigned char *src,char * dest);


void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}


int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len=0;

    // INIT
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    ciphertext_len = len;

    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        handleErrors();
    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);
    ciphertext[ciphertext_len]='\0';
    return ciphertext_len;
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,unsigned char *iv, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len=0;

    // INIT
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = len;

    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        handleErrors();
    plaintext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);
    plaintext[plaintext_len]='\0';
    return plaintext_len;
}


int isIn(int n, int * lst,int size){ // For checking for a int in int array
    for(;size >= 0; size--){
        if(lst[size] == n){
            return 1;
        }
    }
    return 0;
}

int isIn2(unsigned char n){ // For checking for a char in char array
    unsigned char pad[] = {200,201,202,203,204,205,206,207,208,209,210};
    for(int i = 0; i < 10; i++){
        if(pad[i] == n){
            return 1;
        }
    }
    return 0;
}

unsigned char padChar(){
    unsigned char pad[] = {200,201,202,203,204,205,206,207,208,209,210};
    int r = rand()%10;
    return pad[r]; 
}

void padding(char *txt,const int txt_len,unsigned char *dest){
    int diff = 16 - txt_len;
    int r;
    srand(time(0));
    int *rand_list = (int *)malloc(sizeof(int)*diff);
    for(int i = 0; i < diff; i++){
        do{
            r = rand()%16;
        }while(isIn(r,rand_list,i-1));
        rand_list[i] = r;
    }
    
    for (int i = 0,j = 0; i <= 16; i++){
        if(isIn(i,rand_list,diff-1)){
            dest[i] = padChar();
        }else{
            dest[i] = txt[j];
            j++;
        }
    }
}

void mencrypt(char *txt,unsigned char *hash,const int txt_len,unsigned char *dest){
    unsigned char padTxt[16];
    unsigned char psk;
    padding(txt,txt_len,padTxt);
    for (int i = 0; i < 16; i++){
        psk = padTxt[i] ^ hash[i];
        sprintf(dest+(i*2),"%02x",psk);
    }
    printf("\n");
    dest[33] = '\0';
}

void mdecrypt(unsigned char *enc,unsigned char *hash,unsigned char *dest){
    unsigned char psk[16];
    hexToText(enc,psk);
    unsigned char c1;

    int i = 0,j = 0;
    for (; i < 16; i++){
        c1 = psk[i] ^ hash[i];
        // printf("%c",c1);
        if (! isIn2(c1)){
            dest[j] =  c1;
            ++j;
        }
    }
    dest[j] = '\0';
}



void KeyIvGen(const char *pass, char *m_key,char *m_iv){
    /* A 256 bit key */
    /* A 128 bit IV */
    
    // KEY
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, pass, strlen(pass));
    SHA256_Final(m_key, &sha256);
    m_key[SHA256_DIGEST_LENGTH]='\0';

    // IV
    MD5_CTX IVmd5;
    MD5_Init(&IVmd5);
    MD5_Update(&IVmd5,pass,strlen(pass));
    MD5_Final(m_iv,&IVmd5);
    m_iv[MD5_DIGEST_LENGTH]='\0';
}

void hexToText(char *src,unsigned char * dest){
    char temp[3];
    for(int i=0,j=0; i < strlen(src); i+=2,j++){
        sprintf(temp,"%c%c",src[i],src[i+1]);
        dest[j]=(char)(int)strtol(temp,NULL,16);
    }
}

void textToHex(unsigned char *src,char * dest){
    for(int i=0; i < strlen(src); i++){
        sprintf(dest+(i*2),"%02x",src[i]);
    }
    dest[strlen(src)*2] = '\0';
    printf("\n");
}

void mgetpass(char *psk){
    char c;
    int index = 0;
    while((c = getch()) != 13){
        if(c == 8){
            index--;
            if(index < 0){
                index = 0;
            }else{
                printf("\b \b");
            }  
            continue;
        }
        if(index < 15){
            if(c == 3){
                exit(0);
            }
            psk[index] = c;
            putch('*');
            index++;
        }
    }
    psk[index] = '\0';
}

#define protect
#endif