import java.util.Scanner;

class MyCalculator
{
    static long power(int n, int p) throws Exception
    {
        if((n < 0) || (p < 0))
        {
            throw new Exception("n or p should not be negative.");
        }
        else if((n + p) == 0)
        {
            throw new Exception("n and p should not be zero.");
        }
        else
        {
            return (long)Math.pow(n, p);
        }
    }
}

public class nesting
{
    public static void main(String args[]) throws Exception
    {
        Scanner read_stdin = new Scanner(System.in);
        System.out.print("Enter n: ");
        int n = read_stdin.nextInt();
        System.out.print("Enter p: ");
        int p = read_stdin.nextInt();
        long total = MyCalculator.power(n, p);
        System.out.println("Answer: " + total);
        read_stdin.close();
    }
}