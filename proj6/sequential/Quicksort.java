import java.util.Comparator;

public class Quicksort<B extends Comparable<B>> {

    private int hi;
    private int lo;

    private B[] arr;

    Comparator<B> comp;

    public Quicksort(B[] array) {
        this(array, 0, array.length-1, Comparator.<B>naturalOrder());
    }

    public Quicksort(B[] array, Comparator<B> comparator) {
        this(array, 0, array.length-1, comparator);
    }

    public Quicksort(B[] array, int high, int low) {
        this(array, high, low, Comparator.<B>naturalOrder());

    }

    public Quicksort(B[] array, int high, int low, Comparator<B> comparator) {
        hi = high;
        lo = low;

        arr = array;

        comp = comparator;
    }


    public void sort() {
        
    }

}
