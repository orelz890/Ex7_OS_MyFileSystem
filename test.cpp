#include "my_fs.hpp"
#include "mylibc.hpp"
#include <assert.h>


int main(int argc, char const *argv[])
{
    char c_expected = '0';
    int num_expected = 0;
    int num2_expected = 1;
    char c_actual;
    int num_actual;
    int num2_actual;

    for (int i = 0; i < 5; i++)
    {
        printf("\n================================== Case %d ==================================\n", i);
        c_expected = '0';
        num_expected = 0;
        num2_expected = 1;

        mymkfs(MAX_FILES);
        assert(mymount("Disc", "Disc", NULL, 0, NULL) == 0);
        printf("Test 1: Disc opened successfully\n");
        
        myFILE *myfd = myfopen("file", "r+");
        printf("Test 2: file opened in r+ mode successfully\n");
        
        c_expected = (char)i + 'a';
        num_expected = i;
        num2_actual = i + 1;
        assert(myfprintf(myfd, "%c%d%d", c_expected, num_expected, num2_expected) == 3);
        printf("Test 3: wrote several types of elements(%c, %d, %d) into the file successfully\n",c_expected, num_expected, num2_expected);
        
        assert(myfseek(myfd, 0, SEEK_SET) >= 0);
        printf("Test 4: SEEK_SET successfuly\n");

        assert(myfscanf(myfd, "%c%d%d", &c_actual, &num_actual, &num2_actual) == 3);
        assert(c_expected == c_actual);
        assert(num_expected == num_actual);
        assert(num2_expected == num2_actual);
        printf("Test 5: retreved these 3 elements we wrote(%c, %d, %d)\n", c_actual, num_actual, num2_actual);

        assert(myfclose(myfd) == 0);
        printf("Test 6: closed the fd successfully\n");

        myfd = myfopen("file", "a");
        printf("Test 7: opened the file in apend mode successfully\n");

        num_expected = num_expected + 'a';
        assert(myfprintf(myfd, "%d", num_expected) == 1);
        printf("Test 8: wrote a num(%d) in this mode\n", num_expected);

        assert(myfclose(myfd) == 0);
        printf("Test 9: closed the fd successfully\n");
        myfd = myfopen("file", "r");
        printf("Test 10: opened the file in a reading mode\n");

        char d1;
        assert(myfscanf(myfd, "%c", &d1) == 1);
        assert(num_expected == d1);
        printf("Test 11: retreaved this num(%c)\n", (int)d1);

        myfclose(myfd);
        printf("Test 12: close the file successfully\n");

        myfd = myfopen("file", "w");
        printf("Test 13: opened the file in writing mode successfully\n");

        float f = 4.5;
        assert(myfprintf(myfd, "%f", f) == 1);
        printf("Test 14: wrote a num(%f) in this mode\n", f);

        assert(myfclose(myfd) == 0);
        printf("Test 15: closed the fd successfully\n");

        myfd = myfopen("file", "r");
        printf("Test 16: opened the file in reading mode successfully\n");

        float d2;
        assert(myfscanf(myfd, "%f", &d2) == 1);
        printf("Test 17: retreaved this num(%f)\n", f);

        myfclose(myfd);
        printf("Test 18: close the file successfully\n");
    }

    printf("\nAll tests are done!!\n");
    return 0;
}
