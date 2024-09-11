

#include <stdio.h>
int main(void)
{
    FILE* fp = fopen("tilemap.txt", "w+");
    for (int y = 0; y < 100; y++)
    {
        for (int x = 0; x < 200; x++)
        {
            fprintf(fp, "S");
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    return 0;
}
