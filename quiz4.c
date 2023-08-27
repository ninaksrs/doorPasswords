#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

int check(char s[], char a[], char ch);
void rep_(char s[], char a[]);

int main()
{
    char ref[5][100] = {
        "apple",
        "banana",
        "cat",
        "dog",
        "bitcoin"};

    char solution[100] = {0};
    char answer[100] = {0};
    char ch;
    int tries = 20;

    srand((unsigned)time(NULL));

    // strcpy copy array ref to array solution
    strcpy(solution, ref[rand() % 5]);
    rep_(solution, answer);

    while (1)
    {
        if (tries == 0)
        {
            printf("\nYou have used all the limited number of times.\n");
            break;
        }

        printf("Please enter a string : %s\n", answer);
        printf("Guess a letter: ");
        scanf(" %c", &ch);

        if (check(solution, answer, ch) == 1)
        {
            printf("\n%s\n", answer);
            printf("Correct answer. quit the game\n");
            break;
        }
        tries--;
    }
    return 0;
}

void rep_(char s[], char a[])
{
    int i;
    for (i = 0; s[i] != '\0'; i++)
    {
        if (isalpha(s[i]))
        {
            a[i] = '-';
        }
        else
        {
            a[i] = ' ';
        }
    }
}

int check(char s[], char a[], char ch)
{
    int i;
    for (i = 0; s[i] != '\0'; i++)
    {
        if (s[i] == ch)
        {
            a[i] = ch;
        }
        else if (s[i] == ' ')
        {
            a[i] = ' ';
        }
    }
    // strcmp = 0 = equal.
    if (strcmp(s, a) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
