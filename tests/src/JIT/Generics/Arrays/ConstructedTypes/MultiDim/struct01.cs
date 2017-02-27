// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
//

using System;


public struct ValX1<T>
{
    public T t;
    public ValX1(T t)
    {
        this.t = t;
    }

}
public class RefX1<T>
{
    public T t;
    public RefX1(T t)
    {
        this.t = t;
    }
}


public struct Gen<T>
{
    public T Fld1;

    public Gen(T fld1)
    {
        Fld1 = fld1;
    }


}

public class Test
{
    public static int counter = 0;
    public static bool result = true;
    public static void Eval(bool exp)
    {
        counter++;
        if (!exp)
        {
            result = exp;
            Console.WriteLine("Test Failed at location: " + counter);
        }

    }

    public static int Main()
    {
        int size = 10;
        int i, j, k;
        double sum = 0;
        int cLoc = 0;

        Gen<int>[, ,] GenArray = new Gen<int>[size, size, size];
        for (i = 0; (i < size); i++)
        {
            for (j = 0; (j < size); j++)
            {
                for (k = 0; (k < size); k++)
                {
                    GenArray[i, j, k] = new Gen<int>(cLoc);
                    cLoc++;
                }
            }
        }

        for (i = 0; (i < size); i++)
        {
            for (j = 0; (j < size); j++)
            {
                for (k = 0; (k < size); k++)
                {
                    sum += GenArray[i, j, k].Fld1;
                    cLoc++;
                }
            }
        }




        Eval(sum == 499500);
        sum = 0;

        if (result)
        {
            Console.WriteLine("Test Passed");
            return 100;
        }
        else
        {
            Console.WriteLine("Test Failed");
            return 1;
        }
    }

}

