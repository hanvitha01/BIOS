1. **Reverse an Array Using Pointers**
**Algorithm**
Start
Declare an array arr[5] and initialize values.
Declare two pointers:
    p pointing to first element.
    q pointing to last element.
Repeat while p < q:
    Swap values of *p and *q using a temporary variable.
    Increment p.
    Decrement q.
Print the reversed array.
Stop.

**Code**
#include<stdio.h>

void main(){

    int arr[5] = {1,2,3,4,5};
    int *p = arr;
    int *q = arr + 4;
    int temp;

    while(p < q){
        temp = *p;
        *p = *q;
        *q = temp;

        p++;
        q--;
    }

    printf("Reversed Array:\n");

    for(int i = 0; i < 5; i++){
        printf("%d ",arr[i]);
    }

}

----------------------------------------------------------------------------------
**2. Count Vowels in a String**
**Algorithm**

Start
Declare a string.
Initialize count = 0.
Read the string.
Traverse each character of the string.
If the character is a vowel (a, e, i, o, u):
    Increment count.
Print the number of vowels.
Stop.

**Code**
#include<stdio.h>

void main(){

    char str[100];
    int count = 0;

    printf("Enter a string: ");
    scanf("%s",str);

    for(int i = 0; str[i] != '\0'; i++){
        if(str[i]=='a'||str[i]=='e'||str[i]=='i'||str[i]=='o'||str[i]=='u'){
            count++;
        }
    }

    printf("Number of vowels = %d",count);

}

----------------------------------------------------------------------------------
**3. Factorial of a Number Using Function**
**Algorithm**

Start
Input a number n.
Call factorial(n).
Inside function:
    Initialize fact = 1.
    Repeat from i = 1 to n:
        fact = fact × i.
Return fact.
Print factorial value.
Stop.

**Code**
#include<stdio.h>

int factorial(int n){

    int fact = 1;

    for(int i = 1; i <= n; i++){
        fact = fact * i;
    }

    return fact;
}

void main(){

    int num;

    printf("Enter a number: ");
    scanf("%d",&num);

    int result = factorial(num);

    printf("Factorial = %d",result);

}

----------------------------------------------------------------------------------

**4. Linear Search in Array**
**Algorithm**

Start
Declare an array and initialize elements.
Input the number to search.
Repeat for i = 0 to size-1:
    If arr[i] equals search value:
        Print position.
        Stop search.
If element not found:
    Print "Element not found".
Stop.

**Code**
#include<stdio.h>

void main(){

    int arr[5] = {2,4,6,8,10};
    int key;
    int found = 0;

    printf("Enter number to search: ");
    scanf("%d",&key);

    for(int i = 0; i < 5; i++){

        if(arr[i] == key){
            printf("Element found at position %d",i+1);
            found = 1;
            break;
        }

    }

    if(found == 0){
        printf("Element not found");
    }

}

----------------------------------------------------------------------------------

**5. Count Digits in a Number**
**Algorithm**

Start
Input a number.
Initialize count = 0.
Repeat while number ≠ 0:
    Divide number by 10.
    Increment count.
Print count of digits.
Stop.

**Code**
#include<stdio.h>

void main(){

    int number;
    int count = 0;

    printf("Enter a number: ");
    scanf("%d",&number);

    while(number != 0){

        number = number / 10;
        count++;

    }

    printf("Number of digits = %d",count);

}