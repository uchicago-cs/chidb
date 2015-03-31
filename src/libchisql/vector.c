#include <chisql/chisql.h>


static void vector_resize(vector_t *vec, size_t new_size)
{
    /* find the nearest power of 2 >= new_size */
    size_t max_size = 1;
    while (max_size < new_size)
        max_size *= 2;

    void **temp = (void **)calloc(max_size, sizeof(void *));
    memcpy(temp, vec->data, vec->size * sizeof(void *));
    free(vec->data);
    vec->data = temp;
    vec->max_size = max_size;
}

vector_t *vector(size_t init_size)
{
    vector_t *vec = (vector_t *)calloc(1, sizeof(vector_t));

    /* find the nearest power of 2 >= init_size */
    vec->max_size = 1;
    while (vec->max_size < init_size)
        vec->max_size *= 2;

    vec->data = (void **)calloc(init_size, sizeof(void *));
    return vec;
}

vector_t *vector_withData(size_t size, ...)
{
    vector_t *vec = vector(size);
    size_t i;
    va_list argp;
    va_start(argp, size);
    for (i=0; i<size; ++i)
    {
        vector_push(vec, va_arg(argp, void *));
    }
    return vec;
}

vector_t *vector_push(vector_t *vec, void *elem)
{
    if (vec->size == vec->max_size)
    {
        vector_resize(vec, vec->size * 2);
    }
    vec->data[vec->size++] = elem;
    return vec;
}

void vector_setAt(vector_t *vector, size_t i, void *data)
{
    if (i < vector->size)
        vector->data[i] = data;
    else
        fprintf(stderr, "vector_setAt error: index %lu out of bounds, nothing added\n", i);
}

void *vector_pop(vector_t *vec)
{
    void *ret = vec->data[--vec->size];
    if (vec->size <= (vec->max_size)/2)
        vector_resize(vec, (vec->max_size)/2);
    return ret;
}

void *vector_getAt(vector_t *vec, size_t i)
{
    if (i < vec->size)
        return vec->data[i];
    fprintf(stderr, "vector_getAt error: index %lu out of bounds, returning null\n", i);
    return NULL;
}

size_t vector_size(vector_t *vector)
{
    return vector->size;
}

void vector_print(vector_t *vector, void (*print) (void *))
{
    size_t i;
    for (i=0; i<vector->size; ++i)
    {
        print(vector->data[i]);
    }
}

void vector_printStrings(vector_t *vector)
{
    size_t i;
    printf("string_vec[");
    for (i=0; i<vector->size; ++i)
    {
        if (i != 0) printf(", ");
        printf("%s", (char *)vector_getAt(vector, i));
    }
    printf("]\n");
}

void vector_clear(vector_t *vector)
{
    size_t i;
    if (!vector) return;
    for (i = 0; i < vector->size; ++i)
    {
        if (vector->data[i] && vector->free)
        {
            vector->free(vector->data[i]);
        }
    }
    vector->size = 0;
    vector->max_size = 0;
}

void vector_setDeleteFunc(vector_t *vector, void (*free) (void *))
{
    if (vector)
        vector->free = free;
}

int word_count = 0;

char *random_str()
{
    // printf("making a random string\n"); fflush(stdout);
    size_t size = rand() % 15, i;
    char *str = (char *)malloc(size + 6);
    sprintf(str, "%4d ", word_count++);
    for (i=5; i<size+5; ++i)
        str[i] = (rand() % 26) + 'a';
    str[i] = '\0';
    // printf("result is %p, %s\n", str, str); fflush(stdout);
    return str;
}

int vec_main(int argc, char const *argv[])
{
    vector_t *vec = vector_withData(3, "hello", "hi", "how are you");
    int i;
    for (i=0; i<vec->size; ++i)
        printf("%s\n", (char *)vector_getAt(vec, i));
    for (i=0; i<1000; ++i)
    {
        char *str = random_str();
        printf("pushing %s, size is %lu, max_size %lu\n", str, vec->size, vec->max_size);
        vector_push(vec, str);
    }
    printf("printing vector %lu %lu\n", vec->size, vec->max_size);
    fflush(stdout);
    for (i=0; i<500; ++i)
    {
        char *str = (char *)vector_pop(vec);
        printf("popped %s, size is %lu, max_size %lu\n", str, vec->size, vec->max_size);
    }
    return 0;
}

void vector_free(vector_t *vector)
{
    free(vector->data);
}
