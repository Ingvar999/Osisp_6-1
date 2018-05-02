#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#define maxThreadCount 10
#define Pi 3.1416

struct args {
	int n, i, j, N;
};

FILE *f = 0;
int count = 0;
pthread_mutex_t mutex;

void *ThreadFunc(void *args)
{
	//sleep(1);
	struct args *args1 = (struct args *)args;
	int n = args1->n, i = args1->i, j = args1->j, N = args1->N;
	free(args1);
	int k = j*2 + 1;
	int sign = (j & 1) * (-2) + 1;
	float x = (float)(2*Pi*i)/N;
	while (x>Pi)
		x -= 2*Pi;
	if (x > Pi/2)
		x = Pi - x;
	if (x < -Pi/2)
		x = -Pi - x;
	long fact = 1;
	float pow = 1;
	for (int l = 1; l <= k; ++l)
	{
		fact *= l;
		pow *= x;
	}
	
	float result = sign * pow / fact;

	printf("%d / %d / %d / %.4f\n", pthread_self(), i, j, result);  

	fseek(f, (i*n + j)*sizeof(float), SEEK_SET);
	fwrite(&result, sizeof(float), 1, f);
	pthread_mutex_lock(&mutex);
	--count;
	pthread_mutex_unlock(&mutex);
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != 4)
		printf("Неверное число параметров");
	else
	{
		int K = atoi(argv[1]), N = atoi(argv[2]), n = atoi(argv[3]);
		f = fopen("temp~", "r+");
		pthread_t *id;
		pthread_mutex_init(&mutex, 0);
		for (int i = 0; i<K; ++i)
			for (int j = 0; j<n; ++j){
				struct args *args = (struct args *)malloc(sizeof(struct args));
				args->n = n;
				args->i = i;
				args->j = j;
				args->N = N;
				pthread_create(id , 0, ThreadFunc, (void *)args);
				pthread_detach(*id);
				pthread_mutex_lock(&mutex);
				++count;
				pthread_mutex_unlock(&mutex);
				while (count >= maxThreadCount)
					sleep(0.01);
			}
		while (count > 0)
			sleep(0.01);
		pthread_mutex_destroy(&mutex);
		FILE *out = fopen("result.txt", "w");
		float temp, result;
		for (int i = 0; i<K; ++i)
		{
			result = 0;
			for (int j = 0; j<n; ++j)
			{
				fseek(f, (i*n + j)*sizeof(float), SEEK_SET);
				fread(&temp, sizeof(float), 1, f);
				result += temp;
			}
			fprintf(out, "%.4f\n", result);
		}
		fclose(f);
		fclose(out);
		printf("done\n");
	}
	return 0;
}
