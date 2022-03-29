#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#define MAXP 2000029 //Need to caculate this
#define SIZE 250004

int iteration = 0;
typedef struct htp_l_node {
	long a;
	struct htp_l_node *next;
}htp_l_node;

typedef struct hash_function_param_t{
	long b;
	int size;
	struct htp_l_node *a_list;
}hash_function_param_t;

typedef struct bloomfilter {
	//needs to maintain 8 250,000 char arrays 
	char *arrays[8];

    hash_function_param_t *hfp[8];
    
	//also needs to maintain 8 universal hash functions
}bf_t;

long generateRand(){
    

    long num;

    /* add code to seed random number generator */
    num = rand();
    num = (num << 8) | rand();

    num = (num % MAXP);
	return num;
}

//returns a node for the hash function parameter list
htp_l_node *get_node(){
    //Figure out how to randomize a here
    struct htp_l_node *temp = (htp_l_node * ) malloc(sizeof(htp_l_node));
    temp->a = generateRand();
    temp->next = NULL;
    return (temp);
}

//Returns a hash function parameter
hash_function_param_t *get_param(){
    //Need to figure out how to randomize b here
    hash_function_param_t *temp = (hash_function_param_t *) malloc(sizeof(hash_function_param_t));
    temp->b = generateRand();
    temp->size = 1;
    temp->a_list = get_node();
    return temp;
}

//Universal hash function
long universalHashFunction(char *S, hash_function_param_t hfp){
	long sum;
	htp_l_node *al;
	sum = hfp.b;
	al= hfp.a_list;

	while(*S != '\0'){
		if(al->next == NULL){
			al->next = (htp_l_node *) get_node();
			al->next->next = NULL;
			al->a = generateRand() % MAXP;
		}
		sum += ((al->a) * abs(((int) *S))) % MAXP;
		S += 1;
		al = al->next;
	}
	
    iteration++;
    long output = sum % MAXP;
	return(output);
}

//function definition
bf_t* create_bf();
void insert_bf(bf_t* B, char *S);
int is_element(bf_t* B, char *S);
void set(char bloom[], long pos);
int find(char bloom[], long pos);

//function implementation
bf_t* create_bf(){
	bf_t *temp = (bf_t *) malloc(sizeof(bf_t));
    //Malloc the different bit arrays
    //Malloc the different bit arrays
    for(int i = 0; i < 8; i++){
        temp->arrays[i] = (char *)malloc(sizeof(char *) * SIZE);
    }
    
    for(int i = 0; i < 8; i++){
        temp->hfp[i] = get_param();
    }
	return temp;
}

void set(char *bloom, long pos) {
	long temp = pos % 8;
	long new_pos = pos / 8; 
	
	unsigned char shift = 1;
	shift = shift << temp;
	bloom[new_pos] = bloom[new_pos] | shift;
}

int find(char *bloom, long pos) {
	int flag = 0; 
	long new_pos = pos / 8;
	long temp = pos % 8;
	unsigned char shift = 1; 
	shift = shift << temp;

	//check to see if the bit in the new position is on or off
	if (bloom[new_pos] & shift)
		return 1;
	else
		return 0;
}

void insert_bf(bf_t* B, char* S) {

    for(int i = 0; i < 8; i++){
        long hash =  universalHashFunction(S, *B->hfp[i]);
        //printf("%ld \n", hash);
        set(B->arrays[i], hash);
    }
	return;
}
int is_element(bf_t* B, char* S) {
    //call universal hash fuctions here
	
	int found = 1;
	int i = 0;
	for (i = 0; i < 8; i++) {
		long hash =  universalHashFunction(S, *B->hfp[i]);
		int check = find(B->arrays[i], hash); //find function will return 1 or 0 depending on if bit is turned on
		found = found & check;
	}
	return found; //temporary
}

void sample_string_A(char *s, long i)
{  s[0] = (char)(1 + (i%254));
   s[1] = (char)(1 + ((i/254)%254));
   s[2] = (char)(1 + (((i/254)/254)%254));
   s[3] = 'a';
   s[4] = 'b';
   s[5] = (char)(1 + ((i*(i-3)) %217));
   s[6] = (char)(1 + ((17*i+129)%233 ));
   s[7] = '\0';
}
void sample_string_B(char *s, long i)
{  s[0] = (char)(1 + (i%254));
   s[1] = (char)(1 + ((i/254)%254));
   s[2] = (char)(1 + (((i/254)/254)%254));
   s[3] = 'a';
   s[4] = (char)(1 + ((i*(i-3)) %217));
   s[5] = (char)(1 + ((17*i+129)%233 ));
   s[6] = '\0';
}
void sample_string_C(char *s, long i)
{  s[0] = (char)(1 + (i%254));
   s[1] = (char)(1 + ((i/254)%254));
   s[2] = 'a';
   s[3] = (char)(1 + ((i*(i-3)) %217));
   s[4] = (char)(1 + ((17*i+129)%233 ));
   s[5] = '\0';
}
void sample_string_D(char *s, long i)
{  s[0] = (char)(1 + (i%254));
   s[1] = (char)(1 + ((i/254)%254));
   s[2] = (char)(1 + (((i/254)/254)%254));
   s[3] = 'b';
   s[4] = 'b';
   s[5] = (char)(1 + ((i*(i-3)) %217));
   s[6] = (char)(1 + ((17*i+129)%233 ));
   s[7] = '\0';
}
void sample_string_E(char *s, long i)
{  s[0] = (char)(1 + (i%254));
   s[1] = (char)(1 + ((i/254)%254));
   s[2] = (char)(1 + (((i/254)/254)%254));
   s[3] = 'a';
   s[4] = (char)(2 + ((i*(i-3)) %217));
   s[5] = (char)(1 + ((17*i+129)%233 ));
   s[6] = '\0';
}



int main()
{  long i,j; 
   bf_t * bloom;
   bloom = create_bf();
   printf("Created Filter\n");
   for( i= 0; i< 1450000; i++ )
   {  char s[8];
      sample_string_A(s,i);
      insert_bf( bloom, s );
   }
   for( i= 0; i< 500000; i++ )
   {  char s[7];
      sample_string_B(s,i);
      insert_bf( bloom, s );
   }
   for( i= 0; i< 50000; i++ )
   {  char s[6];
      sample_string_C(s,i);
      insert_bf( bloom, s );
   }
   printf("inserted 2,000,000 strings of length 8,7,6.\n");
   
   for( i= 0; i< 1450000; i++ )
   {  char s[8];
      sample_string_A(s,i);
      if( is_element( bloom, s ) != 1 )
	{  printf("found negative error (1)\n"); exit(0); }
   }
   for( i= 0; i< 500000; i++ )
   {  char s[7];
      sample_string_B(s,i);
      if( is_element( bloom, s ) != 1 )
	{  printf("found negative error (2)\n"); exit(0); }
   }
   for( i= 0; i< 50000; i++ )
   {  char s[6];
      sample_string_C(s,i);
      if( is_element( bloom, s ) != 1 )
	{  printf("found negative error (3)\n"); exit(0); }
   }
   j = 0;
   for( i= 0; i< 500000; i++ )
   {  char s[8];
      sample_string_D(s,i);
      if( is_element( bloom, s ) != 0 )
	j+=1;
   }
   for( i= 0; i< 500000; i++ )
   {  char s[7];
      sample_string_E(s,i);
      if( is_element( bloom, s ) != 0 )
	j+=1;
   }
   printf("Found %l positive errors out of 1,000,000 tests.\n",j);
   printf("Positive error rate %f\%.\n", (float)j/10000.0);

} 
 
