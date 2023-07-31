class Program2 {
    static void a_function_call(int a) {
        return;
    }

    static int a_function_call() {
        int lol;
        lol = 1;
        return lol;
    }

    public static void main(String[] args) {
        int j = a_function_call();
        int i = 5+5;
        int a = i + j;
    }
}
