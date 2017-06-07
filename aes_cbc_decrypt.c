#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

static void handleErrors(void)
{
  ERR_print_errors_fp(stderr);
  abort();
}

static int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
  unsigned char *iv, unsigned char *plaintext)
{
  EVP_CIPHER_CTX *ctx;

  int len;

  int plaintext_len;

  /* Create and initialise the context */
  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

  /* Initialise the decryption operation. IMPORTANT - ensure you use a key
   * and IV size appropriate for your cipher
   * In this example we are using 256 bit AES (i.e. a 256 bit key). The
   * IV size for *most* modes is the same as the block size. For AES this
   * is 128 bits */
  if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    handleErrors();

  /* Provide the message to be decrypted, and obtain the plaintext output.
   * EVP_DecryptUpdate can be called multiple times if necessary
   */
  if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    handleErrors();
  plaintext_len = len;

  /* Finalise the decryption. Further plaintext bytes may be written at
   * this stage.
   */
  if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleErrors();
  plaintext_len += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return plaintext_len;
}

int decrypt_aes(unsigned char *ciphertext, const int ciphertext_len, const unsigned char *pass, const unsigned char *salt, unsigned char *decryptedtext, int *decryptedtext_len)
{
  /* A 256 bit key */
  unsigned char key[32]; 


  /* A 128 bit IV */
  unsigned char iv[16];
/*  openssl_random_pseudo_bytes(openssl_cipher_iv_length(AES_256_CBC)); */

//  /* Message to be encrypted */
//  unsigned char *plaintext =
//                (unsigned char *)"The quick brown fox jumps over the lazy dog";

  /* Buffer for ciphertext. Ensure the buffer is long enough for the
   * ciphertext which may be longer than the plaintext, dependant on the
   * algorithm and mode
   */
//  unsigned char ciphertext[128];

  /* Buffer for the decrypted text */
//  unsigned char decryptedtext[128];

//  int decryptedtext_len, ciphertext_len;

  /* Initialise the library */
  ERR_load_crypto_strings();
  OpenSSL_add_all_algorithms();
  OPENSSL_config(NULL);

  /* http://security.stackexchange.com/questions/29106/openssl-recover-key-and-iv-by-passphrase/29139#29139 */
  /* https://www.openssl.org/docs/manmaster/crypto/EVP_BytesToKey.html */
  /* https://www.openssl.org/docs/man1.0.1/crypto/EVP_BytesToKey.html */

  const EVP_CIPHER *cipher;
  const EVP_MD *dgst = NULL;

  cipher = EVP_get_cipherbyname("aes-256-cbc");
  if (!cipher) { fprintf(stderr, "no such cipher\n"); return(-1); }

  dgst=EVP_get_digestbyname("md5");
  if (!dgst) { fprintf(stderr, "no such digest\n"); return(-1); }

  if (!EVP_BytesToKey(cipher, dgst, salt, (unsigned char *)pass, strlen((const char *)pass), 1, key, iv)) {
    return(-1);
  }

//  /* Encrypt the plaintext */
//  ciphertext_len = encrypt (plaintext, strlen ((char *)plaintext), key, iv,
//                            ciphertext);

//  /* Do something useful with the ciphertext here */
//  printf("Ciphertext is:\n");
//  BIO_dump_fp (stdout, (const char *)ciphertext, ciphertext_len);

  /* Decrypt the ciphertext */
  *decryptedtext_len = decrypt(ciphertext, ciphertext_len, key, iv,
    decryptedtext);

  /* Add a NULL terminator. We are expecting printable text */
  decryptedtext[*decryptedtext_len] = '\0';

  /* Clean up */
  EVP_cleanup();
  ERR_free_strings();

  return 0;
}

#if 0
int main(int argc, char **argv)
{
  int ret = 0;

  int f = -1;
  const unsigned char *pass = argv[1];

  /* Buffer for ciphertext. Ensure the buffer is long enough for the
   * ciphertext which may be longer than the plaintext, dependant on the
   * algorithm and mode
   */
  unsigned char ciphertext[128];

  /* Buffer for the decrypted text */
  unsigned char decryptedtext[128];

  int decryptedtext_len, ciphertext_len;

  unsigned char salt[8];

//  f = open(argv[2], O_RDONLY);
//  if (f < 0 )
//    perror("open");

//  read(f, salt, 8);
  if (strncmp(salt, "Salted__", 8)!=0)
    return(-1);
//  read(f, salt, 8);
//  ciphertext_len = read(f, ciphertext, 128);

//  close(f);

  ret = decrypt_aes(ciphertext, ciphertext_len, pass, salt, decryptedtext, &decryptedtext_len);

  /* Show the decrypted text */
  printf("Decrypted text is:\n");
  printf("%s\n", decryptedtext);

  return ret;
}
#endif

