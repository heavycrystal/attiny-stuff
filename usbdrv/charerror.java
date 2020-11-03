import java.util.Scanner;
import java.util.regex.Pattern;

class InvalidCharException extends Exception 
{ 
    public InvalidCharException(String s) 
    { 
        // Call constructor of parent Exception 
        super(s); 
    } 
} 

class helper
{
    static void match(String input) throws InvalidCharException
    {
        if(!Pattern.matches("\\w", input))
        {
            throw new InvalidCharException("Error in input.");
        }
    }
}

public class charerror 
{
    public static void main(String args[]) throws InvalidCharException
    {
        Scanner read_stdin = new Scanner(System.in);
        int count = 1;
        while(true)
        {
            System.out.print("Enter character: ");
            helper.match(read_stdin.nextLine());
            System.out.println("Number of characters " + count++);
        }
    }    
}
