**1.Swap Two Numbers Using Pointers**

**Algorithm**



* Start



* Declare two integers x and y.



* Print the values before swapping.



* Call the swap() function and pass the addresses of x and y.



* Inside swap()



&nbsp;	Create a temporary variable temp.



&nbsp;	Store value of \*a in temp.



&nbsp;	Assign value of \*b to \*a.



&nbsp;	Assign temp to \*b.



* Return to main().



* Print the values after swapping.



* Stop.



**Code:**

\#include<stdio.h>



void swap(int \*a, int \*b){

&nbsp;   int temp;

&nbsp;   temp = \*a;

&nbsp;   \*a = \*b;

&nbsp;   \*b = temp;

}



void main(){

&nbsp;   int x = 5;

&nbsp;   int y = 10;



&nbsp;   printf("Before Swap:\\n%d\\n%d\\n",x,y);



&nbsp;   swap(\&x,\&y);



&nbsp;   printf("After Swap:\\n%d\\n%d\\n",x,y);

}





---------------------------------------------------------------------



**2.Sum of Array Elements Using Pointer**

**Algorithm**



* Start



* Declare an array a\[5] and initialize values.



* Declare pointer p and assign it to array a.



* Initialize sum = 0.



* Repeat for i = 0 to 4:



&nbsp;	Add \*p to sum.



&nbsp;	Increment pointer p.



* Print the value of sum.



* Stop.



**code:**

\#include<stdio.h>

void main(){

&nbsp;   int a\[5] = {1,2,3,4,5};

&nbsp;   int \*p = a;

&nbsp;   int sum = 0;

&nbsp;   for (int i = 0; i < 5; i++)

&nbsp;   {

&nbsp;       sum = sum + \*p;

&nbsp;       p++;

&nbsp;   }

&nbsp;   

&nbsp;   printf("%d",sum);

}



---------------------------------------------------------------------



**3.Find Greatest Number in Array**

**Algorithm**



* Start



* Declare an array arr\[] with elements.



* Call function findMax(arr, size).



* Inside function:



&nbsp;	Set max = arr\[0].



&nbsp;	Assign pointer p = arr.



* Repeat for i = 0 to size-1:



&nbsp;	If \*p > max, update max = \*p.



&nbsp;	Increment pointer p.



* Return max.



* Print the maximum value.



* Stop.



**Code:**

int findMax(int \*arr, int size){

&nbsp;   int max = \*arr;

&nbsp;   int \*p = arr;



&nbsp;   for (int i = 0; i < size; i++)

&nbsp;   {

&nbsp;       if (\*p>max)

&nbsp;       {

&nbsp;           max = \*p;

&nbsp;       }

&nbsp;       p++;

&nbsp;   }

&nbsp;   return max;

}



void main(){

&nbsp;   int arr\[5] = {3,5,1,6,0};

&nbsp;   int max  = findMax(arr,5);



&nbsp;   printf("%d",max);

}



---------------------------------------------------------------------



**4.Check Palindrome Number**

**Algorithm**



* Start



* Input a number.



* Store the number in original.



* Initialize reverse = 0.



* Repeat while number ≠ 0:



&nbsp;	Find remainder number % 10.



&nbsp;	Update reverse = reverse × 10 + remainder.



&nbsp;	Divide number by 10 (number = number / 10).



* Compare original and reverse.



* If equal → print Palindrome.



* Otherwise → print Not Palindrome.



* Stop.



**Code:**

\#include <stdio.h>

int main() {

&nbsp;   int number, reverse = 0, original, remainder;

&nbsp;   printf("Enter a number:");

&nbsp;   scanf("%d", \&number);

&nbsp;   original = number;

&nbsp;

&nbsp;   while(number!=0){

&nbsp;       remainder = number%10;

&nbsp;       reverse = reverse\*10 + remainder;

&nbsp;       number = number/10;

&nbsp;   }



if (original == reverse) {

&nbsp;   printf("%d is a palindrome number.\\n", original); 

}   

else {

&nbsp;   printf("%d is not a palindrome number.\\n", original);

}

return 0;

}



---------------------------------------------------------------------



**6.Pangram Program**

**Algorithm**



* Start



* Declare a string str.



* Create an array letters\[26] and initialize to 0.



* Read the input string.



* Traverse each character of the string.



* If character is between 'a' and 'z':



&nbsp;	Mark corresponding index in letters as 1.



* Check all 26 elements of letters.



* If any element is 0 → print Not Pangram.



* Otherwise → print Pangram.



* Stop.



**Code:**

\#include <stdio.h>



int main() {

&nbsp;   char str\[100];

&nbsp;   int letters\[26] = {0};

&nbsp;   int i;



&nbsp;   printf("Enter a word: ");

&nbsp;   scanf("%s", str);



&nbsp;   for(i = 0; str\[i] != '\\0'; i++) {

&nbsp;       if(str\[i] >= 'a' \&\& str\[i] <= 'z') {

&nbsp;           letters\[str\[i] - 'a'] = 1;

&nbsp;       }

&nbsp;   }



&nbsp;   for(i = 0; i < 26; i++) {

&nbsp;       if(letters\[i] == 0) {

&nbsp;           printf("Not a Pangram\\n");

&nbsp;           return 0;

&nbsp;       }

&nbsp;   }



&nbsp;   printf("Pangram\\n");

&nbsp;   return 0;

}



---------------------------------------------------------------------