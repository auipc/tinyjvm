class Program {
    public static void main(String[] args) {
        // compute the first 100 fibbonacci numbers
        long a = 0;
        long b = 1;
        long c = 0;
        for (int i = 0; i < 1000; i++) {
            c = a + b;
            a = b;
            b = c;
            //System.out.println(c);
        }
    }
}
