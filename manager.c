#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "protect.h"
#include "dictionary.h"

#define dict_size1 100


int getFileIndex(FILE *fp,char *w){
    int cur = ftell(fp);
    fseek(fp,34,SEEK_SET);
    int len = strlen(w);
    char c = fgetc(fp);
    int i = 0;

    while (c != EOF){
        if(c == w[i]){
            i++;
        }else{
            i = 0;
        }
        if(i >= len){
            return ftell(fp) - len;
        }
        c = fgetc(fp);
    }
    fseek(fp,cur,SEEK_SET);
    return -1;   
}


void mreplace(FILE *fp,char *org,char *usr,char *str){
    // USE THIS IF strlen(STR1) = strlen(STR2)
    char temp1[43];
    strcpy(temp1,org);
    strcat(temp1,":");
    strcat(temp1,usr);
    
    int index = getFileIndex(fp,temp1)+strlen(temp1)+1;
    fseek(fp,index,SEEK_SET);
    for (int i = 0; i < strlen(str); i++){
        fputc(str[i],fp);
    }
}

int fdelete(FILE* fp, int bytes) {
    
    char byte;
    long readPos = ftell(fp) + bytes, writePos = ftell(fp), startingPos = writePos;
    fseek(fp, readPos, SEEK_SET);
    while (fread(&byte, sizeof(byte), 1, fp)) {
        // Modify readPos
        readPos = ftell(fp);
        fseek(fp, writePos, SEEK_SET);
        
        // Check and write in file 
        if (fwrite(&byte, sizeof(byte), 1, fp) == 0) 
            return errno;

        // Modify writePos
        writePos = ftell(fp);
        fseek(fp, readPos, SEEK_SET);
    }

    // Truncate file size
    ftruncate(fileno(fp), writePos);
    fseek(fp, startingPos, SEEK_SET); 
    return 0;
}


int main (void)
{   
    // VARIABLES
        int new;
        FILE *pfile;
        char opt1;
        char uhpass[35];
        char fpass[33];
        char org[21],usr[21],psk[16];
        unsigned char enc[33],enc2[33],dec[16];
        unsigned char password[16];
        unsigned char key[33];
        unsigned char iv[17];        
        unsigned char plaintext[16]; // one for '\0' so,total 15 
        unsigned char ciphertext[128];
        unsigned char decryptedtext[128];
        int decryptedtext_len, ciphertext_len;
        char temp1[80];
    // VARIABLES

    
    start:
    memset(password,0,16);
    printf("\n\n\t\tWELCOME TO PASS-MANAGER\n");

    pfile = fopen(".pass","r");
    if (pfile!=NULL){
        new = 0; // NOT A NEW USER
        memset(fpass,0,33);
        fgets(fpass,33,pfile);
        fclose(pfile);
    }else{
        ask:
            printf("\nCAN'T FIND YOUR PASS FILE\n\tARE YOU NEW HERE [y/n]\n");
            opt1 = fgetc(stdin);
            if(opt1 == 'n'){
                printf("FILE ERROR : CAN'T DETERMINE YOUR FILE \n");
                exit(0);
            }else if (opt1 == 'y'){
                new = 1; // NEW USER
                fflush(stdin);
            }else{
                printf("Can't understand your input please try again ...\n");
                fflush(stdin);
                goto ask;
            }
        
    }

    // get password
        printf("\nEnter Your Password : ");
        fgets(password,16,stdin);
        password[strcspn(password, "\n")] = '\0';
        KeyIvGen(password,key,iv);
    // HASH THE PASSWORD AND HEX IT (WITCHCRAFT)
        ciphertext_len = encrypt (password, strlen ((char *)password), key, iv,ciphertext);
        memset(uhpass,0,35);
        textToHex(ciphertext,uhpass);
        // BIO_dump_fp (stdout, (const char *)ciphertext, ciphertext_len);

    if(!new){ // NOT A NEW USER
        do{
            if (strcmp(uhpass,fpass) == 0){
                printf("\n\n\t\t\tWELCOME USER\n");
                break;
            }else{
                // get password
                    printf("\nINCOREECT PASSWORD\nEnter Your Password : ");
                    fgets(password,16,stdin);
                    password[strcspn(password, "\n")] = '\0';
                    KeyIvGen(password,key,iv);

                // HASH THE PASSWORD AND HEX IT (WITCHCRAFT)
                    ciphertext_len = encrypt (password, strlen ((char *)password), key, iv,ciphertext);
                    memset(uhpass,0,35);
                    textToHex(ciphertext,uhpass);
            }
        }while(1);
    }else{
        pfile = fopen(".pass","w");
        fprintf(pfile,"%s\n",uhpass);
        fclose(pfile);
        memset(ciphertext,0,sizeof(ciphertext));
        printf("DONE ...\nRestarting...\n");
        goto start;
    }

    // IMPORTANT
    // GET THE PASSWORDS FROM THE FILE AND STORE IT IN A DICTIONARY
        pfile = fopen(".pass","r");
        dict_t *dictionary = intDict(dict_size1);
        fseek(pfile,34,SEEK_SET);
        char c = fgetc(pfile);
        int i;
        while( c != EOF ){
            if(c == '\n'){
                c = fgetc(pfile);
                continue;
            }

            // ORG
            memset(org,0,21);
            i=0;
            while (c != 58){
                org[i] = c;
                c = fgetc(pfile);
                i++;
            }
            c = fgetc(pfile);

            // USR
            memset(usr,0,21);
            i=0;
            while (c != 58){
                usr[i] = c;
                c = fgetc(pfile);
                i++;
            }
            
            // PSK
            memset(enc,0,33);
            memset(dec,0,16);
            fgets(enc,33,pfile);
            mdecrypt(enc,key,dec);
            dictAdd(dictionary,org,usr,dec,dict_size1);

            // CONTINUE
            c = fgetc(pfile);
        }
        fclose(pfile);
    // IMPORTANT

    while(1){
        printf("\n\nWhat Do You Want To DO ...\n\n[1] Add Credentials\n[2] Update Credentials\n[3] Delete Credentials\n[4] List All \n[5] Search By Org.\n[6] Change Main Password\n[7] Exit\n\n");
        opt1=fgetc(stdin);
        fflush(stdin);
        printf("\n");
        switch(opt1){
            case '1':
                opt1 = 1;
                printf("Organisation Name [len -> 20] : ");
                    fgets(org,21,stdin);
                    fflush(stdin);
                    org[strcspn(org,"\n")] = '\0';
                printf("Username [len -> 20] : ");
                    fgets(usr,21,stdin);
                    fflush(stdin);
                    usr[strcspn(usr, "\n")] = '\0';
                printf("Password [len -> 15] : ");
                    fgets(psk,16,stdin);
                    fflush(stdin);
                    psk[strcspn(psk, "\n")] = '\0';

                mencrypt(psk,key,strlen(psk),enc);
                pfile = fopen(".pass","a");
                fprintf(pfile,"%s:%s:%s\n",org,usr,enc);
                fclose(pfile);
                dictAdd(dictionary,org,usr,psk,dict_size1);
                break;
            case '2':
                while (1){
                    printf("[1] Update password\n[2] Update Username\n\n");
                    opt1=fgetc(stdin);
                    fflush(stdin);
                    printf("\n");

                    if(opt1 == '1'){
                        printf("Organisation Name [len -> 20] : ");
                            fgets(org,21,stdin);
                            fflush(stdin);
                            org[strcspn(org,"\n")] = '\0';
                        
                        printf("Username [len -> 20] : ");
                            fgets(usr,21,stdin);
                            fflush(stdin);
                            usr[strcspn(usr, "\n")] = '\0'; 
                        
                        // Check If Entry Exists
                        if(chkUser(dictionary,org,usr,dict_size1) == 0){
                            printf("No org/usr Found\n");
                            break;
                        }
                        
                        // Asking For New password
                        printf("NEW Password [len -> 15] : ");
                            fgets(psk,16,stdin);
                            fflush(stdin);
                            psk[strcspn(psk, "\n")] = '\0';


                        // Encrpyting new password 
                        memset(enc,0,sizeof(enc));
                        mencrypt(psk,key,strlen(psk),enc);
                        
                        // Replacing enc with enc2 in the file
                        pfile = fopen(".pass","r+");
                        mreplace(pfile,org,usr,enc);
                        fclose(pfile);

                        // UPDATE the dictionary
                        dictAdd(dictionary,org,usr,psk,dict_size1);
                        printf("Password Has Been Updated ...\n");
                        break;
                    }

                    if(opt1 == '2'){
                        printf("Organisation Name [len -> 20] : ");
                            fgets(org,21,stdin);
                            fflush(stdin);
                            org[strcspn(org,"\n")] = '\0';
                        
                        printf("OlD Username [len -> 20] : ");
                            fgets(usr,21,stdin);
                            fflush(stdin);
                            usr[strcspn(usr, "\n")] = '\0';
                        

                        if(chkUser(dictionary,org,usr,dict_size1) == 0){
                            printf("No org/usr Found\n");
                            break;
                        }

                        // USERNAME Exists 
                        // Prepare Temp1 variable
                        memset(temp1,0,80);
                        strcpy(temp1,org);
                        strcat(temp1,":");
                        strcat(temp1,usr);

                        // GET PSK AND ENC and Delete the entry
                        memset(psk,0,16);
                        memset(enc,0,sizeof(enc));
                        getPsk(dictionary,org,usr,dict_size1,psk);
                        mencrypt(psk,key,strlen(psk),enc);
                        dictDelete(dictionary,org,usr,dict_size1);

                        // Ask For New Username
                        printf("New Username [len -> 20] : ");
                            fgets(usr,21,stdin);
                            fflush(stdin);
                            usr[strcspn(usr, "\n")] = '\0';
                        
                        
                        // REMOVE the Entry line from the file
                        FILE *pfile = fopen(".pass","r+");
                        fseek(pfile,getFileIndex(pfile,temp1),SEEK_SET);
                        fdelete(pfile,strlen(temp1)+35);
                        fclose(pfile);
                        

                        // ADD the New Entry in the file
                        pfile = fopen(".pass","a");
                        fprintf(pfile,"%s:%s:%s\n",org,usr,enc);
                        fclose(pfile);

                        // UPDATE Dictionary
                        dictAdd(dictionary,org,usr,psk,dict_size1);
                        printf("Username Has Been Updated ...\n");
                        break;
                    }
                }
                opt1 = 1;
                break;
                
            case '3':
                opt1 = 1;
                printf("Organisation Name [len -> 20] : ");
                    fgets(org,21,stdin);
                    fflush(stdin);
                    org[strcspn(org,"\n")] = '\0';
                        
                printf("Username [len -> 20] : ");
                    fgets(usr,21,stdin);
                    fflush(stdin);
                    usr[strcspn(usr, "\n")] = '\0';
                
                // getPsk(dictionary,org,usr,dict_size1,psk);
                
                // Check If Entry Exists
                // memset(psk,0,16);
                // getPsk(dictionary,org,usr,dict_size1,psk);
                if(chkUser(dictionary,org,usr,dict_size1) == 0){
                    printf("No org/usr Found\n");
                    break;
                }

                // Entry Exists
                // Prepare Temp1 variable
                memset(psk,0,16);
                getPsk(dictionary,org,usr,dict_size1,psk);
                memset(enc,0,sizeof(enc));
                mencrypt(psk,key,strlen(psk),enc);
                memset(temp1,0,80);
                strcpy(temp1,org);
                strcat(temp1,":");
                strcat(temp1,usr);

                // REMOVE the Entry line from the file
                FILE *pfile = fopen(".pass","r+");
                fseek(pfile,getFileIndex(pfile,temp1),SEEK_SET);
                fdelete(pfile,strlen(temp1)+35);
                fclose(pfile);

                // UPDATE dictionary
                dictDelete(dictionary,org,usr,dict_size1);
                printf("Entry Has Been Deleted\n");
                break;

            case '4':
                opt1 = 1;
                dump_all(dictionary,dict_size1);
                break;
            case '5':
                opt1 = 1;
                printf("Organisation Name [len -> 20] : ");
                    fgets(org,21,stdin);
                    fflush(stdin);
                    org[strcspn(org,"\n")] = '\0';
                multi2entry * m2e = getByOrg(dictionary,org,dict_size1);

                printf("ORG : %s\n",org);
                while (m2e != NULL){
                    printf("%10s : %s\n%10s : %s\n\n","USER",m2e->e1,"PASSWORD",m2e->e2);
                    m2e = m2e->prev;
                }
                free(m2e);
                break;
            case '6':
                opt1 = 1;
                // get password
                    printf("\nEnter Your New Password : ");
                    fgets(password,16,stdin);
                    password[strcspn(password, "\n")] = '\0';
                    KeyIvGen(password,key,iv);
                // HASH THE PASSWORD AND HEX IT (WITCHCRAFT)
                    ciphertext_len = encrypt (password, strlen ((char *)password), key, iv,ciphertext);
                    memset(uhpass,0,35);
                    textToHex(ciphertext,uhpass);
                    // BIO_dump_fp (stdout, (const char *)ciphertext, ciphertext_len);

                // CHANGE IN FILE
                    pfile = fopen(".pass","r+");
                    for(int i = 0; i < strlen(uhpass); i++){
                        fputc(uhpass[i],pfile);
                    }
                    printf("Main Password Has been changed\n");
                    break;
            case '7':
                printf("Bye Bye ...\n");
                exit(0);  
            default:
                break;
        }
        if(opt1 == 1){
            printf("\nEnter Any key to continue ...");
            getchar();
        }
    }
    /* Decrypt the ciphertext */
    // decryptedtext_len = decrypt(ciphertext, ciphertext_len, key, iv,decryptedtext);

    return 0;
}