def simpleif(x)
    var z = x in (
        if x < 3 then
            z = 1
        end
    ) : z
    end;

def fib(n)
    var a, b = 1, next in (
        for i = 2, i < n in (
            next = a + b :
                a = b :
                b = next
        )
        end
    ) : next
    end;