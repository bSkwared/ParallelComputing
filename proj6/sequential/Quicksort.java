import java.util.*;
public class Quicksort {

    private int high;
    private int low;

    private int[] arr;

    public Quicksort(int[] array) {
        this(array, 0, array.length-1);
    }

    public Quicksort(int[] array, int lo, int hi) {
        low  = lo;
        high = hi;

        arr = array;
    }

    public void sort() {
       sort(low, high); 
    }


    private void sort (int a, int b) {
        if (a >= b) {
            return;
        }

        int left = a;
        int right = b;

        int middle = (left + right)/2;
        int pivot = triMedian(arr[left], arr[middle], arr[b]); // get pivot

        while (left <= right) {
            while (left <= right && arr[left] < pivot) {
                ++left;
            }

            while (left <= right && arr[right] > pivot) {
                --right;
            }

            if (left <= right) {
                int temp = arr[left];
                arr[left] = arr[right];
                arr[right] = temp;
                ++left;
                --right;
            }
        }

        int temp = arr[left];
        arr[left] = arr[b];
        arr[b] = temp;


        sort(a, left-1);
        sort(left, b);
    }

    private static int triMedian(int a, int b, int c) {
        if (a > b) {
            if (c > a) {
                return a;
            } else if (b > c) {
                return b;
            } else {
                return c;
            }
        } else {
            if (c > b) {
                return b;
            } else if (a > c) {
                return a;
            } else {
                return c;
            }
        }
    }

    public static void main(String[] args) {


        int[] a = {5, 2, 8, 3, 6, 2, 5, 8, 3, 0};

        final int TEST_SIZE = 5;
        final int MAX = 10;
        
        final int BIG_TEST = 100000000;
        final int MAX_BIG = 1000000;

        Quicksort q = new Quicksort(a);
        
        for (int i : a) {
            System.out.print(i + " ");
        }
        System.out.println();

        q.sort();
        
        for (int i : a) {
            System.out.print(i + " ");
        }
        System.out.println();

        System.out.println("NEXT TEST ---------\n");

        Random rand = new Random();
        int[] b = new int[TEST_SIZE];
        for (int i = 0; i < TEST_SIZE; ++i) {
            int r = rand.nextInt();
            r = ((r%10)+10)%10;
            b[i] = r;
        }

        q = new Quicksort(b);

        for (int i : b) {
            System.out.print(i + " ");
        }
        System.out.println();

        q.sort();
        
        for (int i : b) {
            System.out.print(i + " ");
        }
        System.out.println();
        System.out.println();




        System.out.println("\n\nBIGGER TEST ---------\n");

        int[] c = new int[BIG_TEST];
        for (int i = 0; i < BIG_TEST; ++i) {
            int r = rand.nextInt();
            r = ((r%MAX_BIG)+MAX_BIG)%MAX_BIG;
            c[i] = r;
        }

        q = new Quicksort(c);

        q.sort();
        
        int last = 0;
        boolean foundError = false;

        for (int i : c) {
            if (i < last) foundError = true;
            last = i;
        }

        System.out.println((foundError)?"Error: numbers out of order\n\n":"Success: numbers sorted!!!\n\n(how)\n\n");


    }
}
