#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

unsigned int ngram_size = 3;

unsigned char n_in_arr(char *str, char *arr, unsigned int arr_len, unsigned int c)
{
    unsigned int i;
    for (i = 0; i < arr_len; i++)
        if (str[0] == arr[i * ngram_size])
            if (memcmp(&str[ngram_size - c], &arr[ngram_size - c + i * ngram_size], c) == 0)
                return 0;
    return 1;
}

unsigned int index_of(char *str, char *arr, unsigned int arr_len, unsigned int c)
{
    unsigned int i;
    for (i = 0; i < arr_len; i++)
    {
        if (str[0] == arr[i * ngram_size])
            if (memcmp(&str[ngram_size - c], &arr[ngram_size - c + i * ngram_size], c) == 0)
                return i;
    }
    return arr_len;
}

unsigned int weighted_random(float *arr, unsigned int len)
{
    unsigned int i;
    float sum = 0;
    float r = (float)rand() / (float)RAND_MAX;
    for (i = 0; i < len; i++)
    {
        sum += arr[i];
        if (r <= sum && sum > 0)
            return i;
    }
    return len - 1;
}

int main(int argc, char **argv)
{
    FILE *fp, *processated;
    char c;
    unsigned int len;
    char *data, *arr, *output;
    unsigned int *table;
    unsigned int i;
    unsigned int pre_arr_len, arr_len, table_len;
    unsigned int output_len;
    unsigned int sum;
    unsigned int pos;
    float *chances;
    
    srand(time(0));

    fp = fopen("data.txt", "r");

    if (fp == NULL)
    {
        perror("Error opening the file");
        return 1;
    }

    if (argc > 1)
    {
        ngram_size = atoi(argv[1]);
    }

    printf("Counting file length...\n");
    len = 0;
    while (1)
    {
        c = fgetc(fp);
        if (c == EOF)
            break;
        len++;
    }
    printf("Data length: %u\n", len);

    fseek(fp, 0, SEEK_SET);


    printf("Getting data...\n");
    data = malloc(len + 1);
    for (i = 0; i < len; i++)
    {
        c = fgetc(fp);
        data[i] = c;
    }
    data[len] = '\0';
    fclose(fp);

    pre_arr_len = len - ngram_size + 1;

    processated = fopen("processated.txt", "r");

    if (processated == NULL)
    {
        arr_len = 0;
        arr = malloc(pre_arr_len * ngram_size);

        printf("Removing repeated ngrams...\n");
        for (i = 0; i < pre_arr_len; i++)
        {
            if (i % 1000 == 0)
            {
                printf(
                    "%f%%\n",
                    (float)i * 100.0f / (float)pre_arr_len
                );
            }
            if (n_in_arr(&data[i], arr, i, ngram_size))
            {
                arr_len++;
                memcpy(&arr[i * ngram_size], &data[i], ngram_size);
            }
        }

        arr = realloc(arr, arr_len * ngram_size);

        processated = fopen("processated.txt", "w");

        fwrite(&arr_len, sizeof(unsigned int), 1, processated);
        fwrite(arr, sizeof(char), arr_len * ngram_size, processated);

        fclose(processated);
    }
    else
    {
        fread(&arr_len, sizeof(unsigned int), 1, processated);
        arr = malloc(arr_len * ngram_size);
        fread(arr, sizeof(char), arr_len * ngram_size, processated);

        fclose(processated);
    }

    table_len = arr_len * 128;
    table = malloc(table_len * sizeof(unsigned int));
    memset(table, 0, table_len * sizeof(unsigned int));

    printf("\nMaking the probabilities table...\n");
    for (i = 0; i < pre_arr_len; i++)
    {
        unsigned int at;

        at = index_of(&data[i], arr, arr_len, ngram_size);

        if (at < arr_len)
            table[at * 128 + data[i + ngram_size]]++;
    }

    output = malloc(1000 + 1);
    sprintf(output, "GNU ");
    output_len = strlen(output);

    printf("Making text...\n\n");
    for (; output_len < 1000; output_len++)
    {
        unsigned int sz = ngram_size - 1;
        pos = index_of(&output[output_len - ngram_size], arr, arr_len, ngram_size);
        while (pos == arr_len)
        {
            pos = index_of(&output[output_len - ngram_size], arr, arr_len, sz--);
            if (sz == 0)
            {
                unsigned int r;
                r = (unsigned int)((float)rand() * (float)(arr_len - 1) / (float)RAND_MAX);
                for (i = 0; i < ngram_size && output_len < 1000; i++)
                {
                    output[output_len++] = arr[r * ngram_size + i];
                }
                output_len--;
                break;
            }
        }
        
        if (sz == 0)
            continue;
        

        sum = 0;
        for (i = 0; i < 128; i++)
            sum += table[pos * 128 + i];

        chances = malloc(128 * sizeof(float));

        for (i = 0; i < 128; i++)
            chances[i] = (float)table[pos * 128 + i] / (float)sum;

        output[output_len] = weighted_random(chances, 128);

        free(chances);
    }
    output[output_len] = '\0';

    printf("%u\n", output_len);
    for (i = 0; i < output_len; i++)
    {
        printf("%c", output[i]);
    }
    printf("\n");

    free(output);
    free(table);
    free(arr);
    free(data);
    return 0;
}
