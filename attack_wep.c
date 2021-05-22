#include <stdio.h>
#include <stdlib.h>

unsigned char arpbytes[16] ={0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x02};

void print_bytes(unsigned char* bytes, int len, char* name) {
  printf("%s[%d] =", name, len);
  for (int i = 0; i < len; i++) { printf(" %02x", bytes[i]); }
  printf("\n");
}

void initialize_prob(unsigned int prob[][256], int len) {
  for (int i = 0; i < len; i++) {
    for(int k = 0; k < 256; k++) {
      prob[i][k] = 0;
    }
  }
}

int rc4_setup_S3(unsigned char* state, unsigned char* ivbytes) {
  unsigned char i = 0;
  while(1) {
    state[i] = i;
    if (i == 255) { break; }
    i++;
  }
  unsigned char tmp = 0;
  i = 0;
  unsigned char j = 0;
  j = j + state[i] + ivbytes[0];
  tmp = state[i];
  state[i] = state[j];
  state[j] = tmp;
  i++;
  // At this point, state is S1
  j = j + state[i] + ivbytes[1];
  tmp = state[i];
  state[i] = state[j];
  state[j] = tmp;
  i++;
  // At this point, state is S2
  j = j + state[i] + ivbytes[2];
  tmp = state[i];
  state[i] = state[j];
  state[j] = tmp;
  // At this point, state is S3
  return j;
}

void setup_xbytes(unsigned char* xbytes, unsigned char* cbytes, unsigned char* arpbytes) {
  for (unsigned char i = 0; i < 16; i++) {
    xbytes[i] = cbytes[i] ^ arpbytes[i];
  }
  return;
}

unsigned char state_inverse(unsigned char* state, unsigned char idx) {
  unsigned char i = 0;
  while(1) {
    if (state[i] == idx) {
      return i;
    }
    if (i == 255) {
      break;
    }
    i++;
  }
  printf("Error(state_inverse): given state is invalid.");
  return 0;
}

unsigned char sum_of_state(unsigned char* state, unsigned char start, unsigned char end) {
  if (start > end) {
    printf("Error(sum_of_state): given start and end is invalid");
    return 0;
  }
  unsigned char ret = 0x00;
  for (unsigned char i = start; i <= end; i++) {
    ret = ret + state[i];
  }
  return ret;
}

void search_first_to_third(unsigned int* data, unsigned char* first, unsigned char* second, unsigned char* third, unsigned int* f_c, unsigned int* s_c, unsigned int* t_c) {
  unsigned char i = 0;
  *f_c = 0;
  while(1) {
    if (*f_c < data[i]) {
      *f_c = data[i];
      *first = i;
    }
    if (i == 255) { break; }
    i++;
  }
  i = 0;
  *s_c = 0;
  while(1) {
    if (*s_c < data[i]) {
      if (i != *first) {
        *s_c = data[i];
        *second = i;
      }
    }
    if (i == 255) { break; }
    i++;
  }
  i = 0;
  *t_c = 0;
  while(1) {
    if (*t_c < data[i]) {
      if (i != *first && i != *second) {
        *t_c = data[i];
        *third = i;
      }
    }
    if (i == 255) { break; }
    i++;
  }
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    printf("Error: specify the number of data to read and filename of data");
    return 0;
  }

  // the number of byte x for sigma_i is prob[i][x]
  unsigned int prob[13][256];
  unsigned char state[256];
  unsigned int max = atoi(argv[1]);

  initialize_prob(prob, 13);

  FILE* fp;
  fp = fopen(argv[2], "rb");
  if (fp != NULL) {
    for(int data_count = 0; data_count < max; data_count++) {
      unsigned char ivbytes[3] = {0x00, 0x00, 0x00};
      unsigned char cbytes[16];
      unsigned char xbytes[16];
      unsigned char terminator = 0x00;
      fread(ivbytes, sizeof(unsigned char), 3, fp);
      // print_bytes(ivbytes, 3, "ivbytes"); // for debug
      fread(cbytes, sizeof(unsigned char), 16, fp);
      // print_bytes(cbytes, 16, "cbytes"); // for debug
      fread(&terminator, sizeof(unsigned char), 1, fp);
      if (terminator != 0xff) {
        printf("Error: input data is invalid!\n");
        return 1;
      }

      // K[0] to K[2] is ivbytes[]
      unsigned char j3 = rc4_setup_S3(state, ivbytes); // state is set to S3 using ivbytes[3]
      setup_xbytes(xbytes, cbytes, arpbytes); // setup keystream xbytes

      for (int i = 0; i < 13; i++) {
        // compute approximation a_i for sigma_i
        unsigned char idx = (3 + i) - xbytes[2 + i];
        unsigned char a_i = state_inverse(state, idx) - (j3 + sum_of_state(state, 3, i + 3));

        // count a_i
        prob[i][a_i] = prob[i][a_i] + 1;
      }
    }
  }
  fclose(fp);

  unsigned char candidates[13];

  // analyze results
  for(int i = 0; i < 13; i++) {
    unsigned char first = 0x00;
    unsigned char second = 0x00;
    unsigned char third = 0x00;
    unsigned int f_c = 0;
    unsigned int s_c = 0;
    unsigned int t_c = 0;

    search_first_to_third(prob[i], &first, &second, &third, &f_c, &s_c, &t_c);
    printf("sigma_%02d: first: %02x (%d), second: %02x (%d), third: %02x (%d)\n", i, first, f_c, second, s_c, third, t_c);
    candidates[i] = first;
  }
  // show result
  printf("Predicted key: %02x", candidates[0]);
  for (int i = 1; i < 13; i++) {
    unsigned char byte = candidates[i] - candidates[i - 1];
    printf(" %02x", byte);
  }
  printf("\n");

  return 0;
}