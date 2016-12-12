import java.util.Random;
import java.util.HashMap;

class TestQuicksort {

    public static void main(String[] args) {


        int[] a = {5, 2, 8, 3, 6, 2, 5, 8, 3, 0};

        final int TEST_SIZE = 9;
        final int MAX = 10;
        
        final int BIG_TEST = 10000;
        final int MAX_BIG = 100000;

        Quicksort q = new Quicksort(a);
       /* 
        for (int i : a) {
            System.out.print(i + " ");
        }
        System.out.println();

        q.sort();
        
        for (int i : a) {
            System.out.print(i + " ");
        }
        System.out.println();*/

        System.out.println("NEXT TEST ---------\n");

        Random rand = new Random();
  /*      int[] b = new int[TEST_SIZE];
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

        //q.sort();
        q.parallelSort();
        
        for (int i : b) {
            System.out.print(i + " ");
        }
        System.out.println();
*/        System.out.println();




        System.out.println("\n\nBIGGER TEST ---------\n");

        HashMap<Integer, Integer> numbersBefore = new HashMap<>();

        int[] c = new int[BIG_TEST];
        for (int i = 0; i < BIG_TEST; ++i) {
            int r = rand.nextInt();
            r = ((r%MAX_BIG)+MAX_BIG)%MAX_BIG;
            c[i] = r;
            if (numbersBefore.containsKey(r)) {
                numbersBefore.put(r, numbersBefore.get(r));
            } else {
                numbersBefore.put(r, 1);
            }
        }

        q = new Quicksort(c);

        q.parallelSort();
        
        int last = 0;
        boolean foundError = false;

        HashMap<Integer, Integer> numbersAfter = new HashMap<>();
        for (int i : c) {
            if (i < last) foundError = true;
            last = i;
            if (numbersAfter.containsKey(i)) {
                numbersAfter.put(i, numbersAfter.get(i));
            } else {
                numbersAfter.put(i, 1);
            }
        }
        if (!numbersBefore.equals(numbersAfter)) {
            foundError = true;
        }

        System.out.println((foundError)?"Error: numbers out of order\n\n":"Success: numbers sorted!!!\n\n(how)\n\n");

    }
}
