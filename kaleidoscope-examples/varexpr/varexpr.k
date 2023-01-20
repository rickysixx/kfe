def fib(x)
    var a = 1, b = 1, c in (
        for i = 3, i < x in
            c = a + b :
                a = b :
                b = c
        end
    ) : a + b
    end;