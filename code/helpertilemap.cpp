

#include <stdio.h>
int main(void)
{
    FILE* fp = fopen("map.txt", "w");
    for (int y = 0; y < 200; y++)
    {
        for (int x = 0; x < 400; x++)
        {
            fprintf(fp, "#");
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    return 0;
}
