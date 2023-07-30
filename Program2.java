class Program {
    public static void main(String[] args) {
        // compute the first 100 fibbonacci numbers
        int a = 0;
        int b = 1;
        int c = 0;
        for (int i = 0; i < 15; i++) {
            c = a + b;
            a = b;
            b = c;
            System.out.println(c);
        }
    }
}
