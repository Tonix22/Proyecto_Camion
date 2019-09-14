/**
 * Definition for an interval.
 * struct Interval {
 *     int start;
 *     int end;
 *     Interval() : start(0), end(0) {}
 *     Interval(int s, int e) : start(s), end(e) {}
 * };
 */
vector<Interval> Solution::merge(vector<Interval> &A) 
{
    // Do not write main() function.
    // Do not read input, instead use the arguments to the function.
    // Do not print the output, instead return values as specified
    // Still have a doubt. Checkout www.interviewbit.com/pages/sample_codes/ for more details
// A function to implement bubble sort 
   int i, j,temp;
   int n = A.size();
   if(n==1)
   {
       return A;
   }
   
   for (i = 0; i < n-1; i++)
   {
      if(A[i].start>=A[i].end)
       {
           temp       = A[i].start;
           A[i].start = A[i].end;
           A[i].end   = temp;
       }
        // Last i elements are already in place    
       for (j = 0; j < n-i-1; j++)
       {
               if (A[j].start > A[j+1].start)
               {
                   temp         = A[j].start;
                   A[j].start   = A[j+1].start;
                   A[j+1].start = temp;
               }
                if (A[j].end > A[j+1].end)
                {
                   temp         = A[j].end;
                   A[j].end     = A[j+1].end;
                   A[j+1].end   = temp;
                }               
       }
   }

   i=0;
    do
    {
       if(A[i].end>=A[i+1].start)
       {
           if(A[i].end<=A[i+1].end)
           {
              A[i].end = A[i+1].end;
           }
            A.erase(A.begin() + i+1);
       }
       else
       {
         i++;  
       }
      
    }while(A.size()-1 >i);
  return A;
}