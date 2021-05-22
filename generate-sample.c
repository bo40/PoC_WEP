#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void print_bytes(unsigned char* bytes, int len, char* name) {
  printf("%s[%d] =", name, len);
  for (int i = 0; i < len; i++) { printf(" %02x", bytes[i]); }
  printf("\n");
}

int rc4_key_setup(unsigned char* state, unsigned char* key) {
  unsigned char i = 0;
  while(1) {
    state[i] = i;
    if (i == 255) { break; }
    i++;
  }
  unsigned char tmp = 0;
  i = 0;
  unsigned char j = 0;
  while(1) {
    j = j + state[i] + key[i % 16];
    tmp = state[i];
    state[i] = state[j];
    state[j] = tmp;
    if (i == 255) { break; }
    i++;
  }
  return 0;
}

int is_strong_keybyte(unsigned char* rootkey, unsigned int idx) {
  for (int l = 1; l <= idx; l++) {
    unsigned char sum = 0;
    for (int k = l; k <= idx; k++) {
      sum = sum + rootkey[k] + 3 + k;
    }
    if (sum == 0) {
      return 1;
    }
  }
  return 0;
}

int main(int argc, char* argv[]) {
  unsigned char state[256];
  unsigned char terminator = 0xff;

  if (argc < 2) {
    printf("Error: specify the count of samples.\n");
    return 0;
  }
  srand((unsigned int)time(NULL));

  unsigned char rootkeybytes[13] = {0x33, 0x31, 0x34, 0x31, 0x35, 0x39, 0x32, 0x36, 0x35, 0x33, 0x35, 0x38, 0x39}; // sample of normal key
  // unsigned char rootkeybytes[13] = {0x31, 0x31, 0x34, 0x35, 0x31, 0x34, 0x38, 0x31, 0x30, 0x38, 0x39, 0x33, 0x39}; // sample of strong key
  print_bytes(rootkeybytes, 13, "rootkeybytes");
  for(int i = 1; i < 13; i++) {
    if (is_strong_keybyte(rootkeybytes, i) == 1) {
      printf("Rk[%d] is a strong key byte", i);
    }
  }

  unsigned char arpbytes[16] ={0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x02};
  print_bytes(arpbytes, 16, "arpbytes");

  unsigned int max = atoi(argv[1]);

  FILE* fp;
  fp = fopen("data.out", "wb+");
  if (fp != NULL) {

    for (int i = 0; i < max; i++) {
      unsigned char keybytes[16];
      keybytes[0] = rand() % 256;
      keybytes[1] = rand() % 256;
      keybytes[2] = rand() % 256;

      for (int k = 0; k < 13; k++) {
        keybytes[3 + k] = rootkeybytes[k];
      }
      // print_bytes(keybytes, 16, "keybytes"); // for debug

      rc4_key_setup(state, keybytes);

      fwrite(&keybytes[0], sizeof(unsigned char), 1, fp);
      fwrite(&keybytes[1], sizeof(unsigned char), 1, fp);
      fwrite(&keybytes[2], sizeof(unsigned char), 1, fp);

      unsigned char r_i = 0;
      unsigned char r_j = 0;
      unsigned char tmp;
      for (int k = 0; k < 16; k++) {
        r_i = r_i + 1;
        r_j = r_j + state[r_i];
        tmp = state[r_i];
        state[r_i] = state[r_j];
        state[r_j] = tmp;
        unsigned char idx = state[r_i] + state[r_j];
        unsigned char ret = arpbytes[k] ^ state[idx];
        fwrite(&ret, sizeof(unsigned char), 1, fp);
      }
      fwrite(&terminator, sizeof(unsigned char), 1, fp);
    }
  }
  fclose(fp);

  printf("data generation finished.\n");

  return 0;
}

