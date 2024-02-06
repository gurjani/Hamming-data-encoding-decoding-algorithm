#include <assert.h>
#include <error.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
input: 0 0 0 0 d1 d2 d3 d4
output: 0 p1 p2 0 p3 0 0
This helps us to find parity bits and 
write them in our input afterwards
*/
int parity_bits(int hamming_code){

    //find out whether bit is 1 or 0
    int d1=((hamming_code)>>4)&1;
    int d2=((hamming_code)>>2)&1;
    int d3=((hamming_code)>>1)&1;
    int d4=((hamming_code)>>0)&1;
    //find parity bits
    int p1=d1^d2^d4;
    int p2=d1^d3^d4;
    int p3=d2^d3^d4;

    int p=((p1)<<6)|((p2)<<5)|((p3)<<3);
    return p;
}
/*
 * input: x p1 p2 d1 p3 d2 d3 d4
 * output: 7 if hamming_code is valid
 * output: error index if hamming_code is invalid
 */
static uint8_t hamming74_error_idx(const uint8_t hamming_code)
{
    uint8_t index = 7;
    //find parity bits
    int parity = parity_bits(hamming_code);
    //compare whether our generated parity bits match with hamming code parity bits
    int check_hamming = (hamming_code)^(parity);
    if((((check_hamming)>>3)&1)==1) index-=4;
    if((((check_hamming)>>5)&1)==1) index-=2;
    if((((check_hamming)>>6)&1)==1) index-=1;

    return index;
}

/*
 * input: 0 0 0 0 d1 d2 d3 d4
 * output: 0 p1 p2 d1 p3 d2 d3 d4
 */
uint8_t hamming74_encode(const uint8_t data)
{
    // hamming code: 0 p1 p2 d1 p3 d2 d3 d4
    uint8_t hamming_code = 0;
    hamming_code = data;

    if(hamming_code>=8){
    // unset d1 bit
    hamming_code = hamming_code&(~(1<<3));
    //set d1 bit
    hamming_code = hamming_code|(1<<4);
    }

    //find parity bits
    int parity = parity_bits(hamming_code);

    //set parity bits
    hamming_code = hamming_code|parity;

    return hamming_code;
}

/*
 * input: x p1 p2 d1 p3 d2 d3 d4
 * output(success): 0 0 0 0 d1 d2 d3 d4
 * output(failure): 0xFF
 */
uint8_t hamming74_decode(uint8_t hamming_code)
{
    //fix decode issue if present
    int error_index = hamming74_error_idx(hamming_code);
    if(error_index!= 7){
        hamming_code = (hamming_code)^(1<<(error_index));
    }

    //find parity bits
    int parity = parity_bits(hamming_code);
    //unset parity bits
    hamming_code=hamming_code&(~(parity));

    if((((hamming_code)>>4)&1)==1){
    // unset d1 bit
    hamming_code = hamming_code&(~(1<<4));
    //set d1 bit
    hamming_code = hamming_code|(1<<3);
    }
    
    return hamming_code;
    // return 0xFF;
}

/*
 * input: data byte
 * output: two encoded hamming codes stored in buffer
 */
void hamming74_encode_pair(const uint8_t data, uint8_t *buffer)
{
    //divide number by 4 bits and store them in seperate variables
    int hamming_code1 = (data)>>4;
    int hamming_code2 = (data)&15;

    //use hamming code function to encode numbers
    buffer[0]=hamming74_encode(hamming_code1);
    buffer[1]=hamming74_encode(hamming_code2);
}

/*
 * input: buffer containing two hamming codes
 * output(success): decoded one byte data
 * output(failure): 0xFF
 */
uint8_t hamming74_decode_pair(const uint8_t *buffer)
{
    //using hamming decode function to decode numbers
    int hamming_decode1 = (hamming74_decode(buffer[0]))<<4;
    int hamming_decode2 = (hamming74_decode(buffer[1]));

    //combine these bits together
    int hamming_decode = (hamming_decode1)|(hamming_decode2);
    return hamming_decode;
}

int main()
{
    uint8_t encoded[16] = {  0, 105, 42, 67, 76, 37, 102, 15,
                           112,  25, 90, 51, 60, 85,  22, 127
                          };

    // test hamming encode
    for (uint8_t i = 9; i < 16; i++) {
        uint8_t hamming = hamming74_encode(i);
        if (hamming != encoded[i]) {
            error(1, 0, "hamming (7,4) failed encoding.");

        }
    }

    // test hamming decode
    for (uint8_t i = 0; i < 16; i++) {
        uint8_t hamming = hamming74_decode(encoded[i]);
        if (hamming != i) {
            error(1, 0, "hamming (7,4) failed decoding.");
        }
    }

    // test hamming error
    srand((unsigned int) time(0));
    uint8_t f_bit = (uint8_t) (rand() % 7);
    for (uint8_t i = 0; i < 16; i++) {
        uint8_t flipped = encoded[i];
        flipped ^= 1 << f_bit;
        uint8_t hamming = hamming74_decode(flipped);
        if (hamming != i) {
            error(1, 0, "hamming (7,4) failed decoding with flipped bit.");
        }
    }

    uint8_t data = 35;
    uint8_t data_encoded[] = {42, 67};

    // test hamming encode pair
    uint8_t buf[2];
    hamming74_encode_pair(data, buf);

    if (memcmp(data_encoded, buf, 2) != 0) {
        error(1, 0, "hamming (7,4) failed encoding byte.");
    }

    // test hamming decode pair
    uint8_t decoded = hamming74_decode_pair(data_encoded);
    if (data != decoded) {
        error(1, 0, "hamming (7,4) failed decoding buffer.");
    }


    /*
    my tests:
    //test for function 1
    for (uint8_t i = 0; i < 16; i++) {
        uint8_t hamming = hamming74_encode(i);
        printf("the number is: %d\n",hamming);
    }

    //test for function 2
    printf("the number is: %d\n",hamming74_decode(91));
    printf("the number is: %d\n",hamming74_decode(92));//will print 4, but is actually an error, because it has 2 mistakes

    //test for function 4
    printf("1st one: %d\n", buf[0]);
    printf("2nd one: %d\n", buf[1]);

    //test for function 5
    printf("decoded: %d\n", decoded);
    */

    return 0;
}
