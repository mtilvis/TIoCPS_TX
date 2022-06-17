#include <zephyr/types.h>
#include <stddef.h>

typedef struct {
   uint32_t P[16 + 2];
   uint32_t S[4][256];
} BLOWFISH_CTX;

#ifndef BLOWFISH_SELF
uint8_t blowfish_key[] = "HS_APPLICATION";
#endif

void Blowfish_Init(BLOWFISH_CTX *ctx, uint8_t *key, int32_t keyLen);
void Blowfish_Encrypt(BLOWFISH_CTX *ctx, uint32_t *xl, uint32_t *xr);
void Blowfish_Decrypt(BLOWFISH_CTX *ctx, uint32_t *xl, uint32_t *xr);
