#include <stdio.h>
#include <string.h>
#include <CommonCrypto/CommonCrypto.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "huffman.c"


void hashPassword(const char *password, unsigned char hash[CC_SHA256_DIGEST_LENGTH]) {
    CC_SHA256_CTX sha256;
    CC_SHA256_Init(&sha256);
    CC_SHA256_Update(&sha256, password, strlen(password));
    CC_SHA256_Final(hash, &sha256);
}

void printHash(const unsigned char hash[CC_SHA256_DIGEST_LENGTH]) {
    for (int i = 0; i < CC_SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

int compareHashes(const unsigned char hash1[CC_SHA256_DIGEST_LENGTH], const unsigned char hash2[CC_SHA256_DIGEST_LENGTH]) {
    return memcmp(hash1, hash2, CC_SHA256_DIGEST_LENGTH) == 0;
}

void hexStringToBytes(const char *hexString, unsigned char *hash) {
    for (int i = 0; i < CC_SHA256_DIGEST_LENGTH; i++) {
        sscanf(hexString + 2 * i, "%2hhx", &hash[i]);
    }
}

bool readHashFromFile(const char *filename, unsigned char hash[CC_SHA256_DIGEST_LENGTH]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return false;
    }

    char hexHash[2 * CC_SHA256_DIGEST_LENGTH + 1];
    if (fgets(hexHash, sizeof(hexHash), file) != NULL) {
        hexHash[strcspn(hexHash, "\n")] = '\0';  
        hexStringToBytes(hexHash, hash);
        fclose(file);
        return true;
    }

    fclose(file);
    return false;
}

void writeHashToFile(const char *filename, const unsigned char hash[CC_SHA256_DIGEST_LENGTH]) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Failed to open file for writing");
        return;
    }

    for (int i = 0; i < CC_SHA256_DIGEST_LENGTH; i++) {
        fprintf(file, "%02x", hash[i]);
    }

    fclose(file);
}

bool checkPass(char *userPassword) {
    unsigned char hash[CC_SHA256_DIGEST_LENGTH];
    hashPassword(userPassword, hash);
    printf("Hash of the entered password: ");
    printHash(hash);

    unsigned char hashToVerify[CC_SHA256_DIGEST_LENGTH];
    if (!readHashFromFile("pass.txt", hashToVerify)) {
        printf("Failed to read hash from file.\n");
        return false;
    }

    if (compareHashes(hash, hashToVerify)) {
        printf("Password verification successful.\n");
        return true;
    } else {
        printf("Wrong password.\n");
        printf("Enter 1 to change the password.\nEnter 2 to exit the program.\n");
        int num;
        scanf("%d", &num);
        getchar(); 

        if (num == 1) {
            printf("Enter the new password: ");
            char newPassword[200];
            fgets(newPassword, sizeof(newPassword), stdin);

            size_t len = strlen(newPassword);
            if (len > 0 && newPassword[len - 1] == '\n') {
                newPassword[len - 1] = '\0';
            }

            unsigned char newHash[CC_SHA256_DIGEST_LENGTH];
            hashPassword(newPassword, newHash);
            writeHashToFile("pass.txt", newHash);
            printf("Password changed successfully.\n");
        } else {
            printf("Access Denied.\n");
            exit(0);
        }
    }

    return false;
}

int main() {
    int n = 2;
    char **strings = (char **)malloc(n * sizeof(char *));
    bool pass = true;
    for (int i = 0; i < n; i++) {
        strings[i] = (char *)malloc(300 * sizeof(char)); 
        if (pass) {
            printf("Enter password: ");
            fgets(strings[i], 200, stdin);
            pass = !pass;
        } else {
            pass = !pass;
            printf("Enter the string you want to encode: ");
            fgets(strings[i], 300, stdin);
        }

        size_t len = strlen(strings[i]);
        if (len > 0 && strings[i][len - 1] == '\n') {
            strings[i][len - 1] = '\0';
        }
    }
    printf("\nYou entered:\n");
    for (int i = 0; i < n; i++) {
        printf("String %d: %s\n", i + 1, strings[i]);
    }

    if (checkPass(strings[0])) {
        implementHuffman(strings[1]);
    }

    for (int i = 0; i < n; i++) {
        free(strings[i]);
    }
    free(strings);

    return 0;
}

